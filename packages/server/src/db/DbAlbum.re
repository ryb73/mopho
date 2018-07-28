open BsKnex;
open BsKnex.Params.Infix;
open BatPervasives;
open Bluebird;
module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let map = BluebirdEx.map;

[@decco]
type album = {
    id: int,
    name: string,
    primaryArtistId: int,
    napsterId: option(string),
    metadataSource: [@decco.codec (
        Models.metadataSource_encode
            %> Js.Json.stringify
            %> Js.Json.string,
        (j) => Js.Json.decodeString(j)
            |> ResultEx.fromOpt({ Decco.path: "", message: "Not a string", value: j })
            |> Belt.Result.map(_, Js.Json.parseExn)
            |> Belt.Result.flatMap(_, Models.metadataSource_decode)
    )] Models.metadataSource
};

[@decco] type albumsResult = array(album);

let _convertDbAlbum = ({ id, name, napsterId, metadataSource, primaryArtistId }) =>
    { Models.Album.id, name, napsterId, metadataSource, primaryArtistId };

let createFromNapster = (~name, ~napsterId, ~primaryArtist) => {
    open Insert;

    Js.log3("Inserting album from Napster:", primaryArtist.Models.Artist.name, {j|– $name ($napsterId)|j});

    let napsterId = Some(napsterId);

    Insert.make(DbHelper.knex)
        |> into("albums")
        |> set("name", name)
        |> set("napsterId", napsterId)
        |> set("metadataSource", Js.Json.stringify(Models.metadataSource_encode(Models.Napster)))
        |> set("createdUtc", Std.getCurrentUtc())
        |> set("primaryArtistId", primaryArtist.id)
        |> toString
        |> DbHelper.doQuery
        |> map(DbHelper.getInsertId)
        |> map(id => {
            Models.Album.id, name, napsterId,
            primaryArtistId: primaryArtist.id,
            metadataSource: Models.Napster
        });
};

let _parseSingleAlbumResult = ((result, _)) => {
    switch (albumsResult_decode(result)) {
        | Ok([| artist |]) => Some(_convertDbAlbum(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(e) => Js.log(e); Js.Exn.raiseError("Error converting albumsResult")
    };
};

let findByNameAndArtist = (name, primaryArtist) => {
    open Select;

    DbHelper.selectAll("albums")
        |> whereParam("name = ?", ?? name)
        |> whereParam("primaryArtistId = ?", ?? primaryArtist.Models.Artist.id)
        |> where("napsterId IS NULL") /* TODO: this'll have to be generalized */
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleAlbumResult)
        |> tapMaybe(({ Models.Album.id }) => resolve(Js.log({j|Matched album $name [$id] by artist|j})));
};

let findByNapsterId = (napsterId) => {
    open Select;

    DbHelper.selectAll("albums")
        |> whereParam("napsterId = ?", ?? napsterId)
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleAlbumResult)
        |> tapMaybe(({ Models.Album.id, name }) => resolve(Js.log({j|Matched album $name [$id] by Napster ID|j})));
};

let setNapsterId = (napsterId, { Models.Album.id, name }) => {
    open Update;

    Js.log3("Setting napster ID for album", id, name);

    Update.make("albums", DbHelper.knex)
        |> set("napsterId", napsterId)
        |> whereParam("id = ?", ?? id)
        |> toString
        |> DbHelper.doQuery
        |> map(DbHelper.testAffectedRows)
};

let matchNapster = (name, napsterId, artistName, primaryArtistNapsterId) => {
    findByNapsterId(napsterId)
        |> then_(fun
            | Some(a) => resolve(a)
            | None =>
                DbArtist.matchNapster(artistName, primaryArtistNapsterId)
                    |> then_((primaryArtist) =>
                        findByNameAndArtist(name, primaryArtist)
                            |> tapMaybe(setNapsterId(Some(napsterId)))
                            |> then_(fun
                                | Some(a) => resolve(a)
                                | None => createFromNapster(~name, ~napsterId, ~primaryArtist)
                            )
                    )
        );
};
