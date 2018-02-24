open BsKnex;
open BsKnex.Params.Infix;
open BatPervasives;
open Bluebird;
module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let map = BluebirdEx.map;

[@autoserialize] type album = {
    id: int,
    name: string,
    primaryArtistId: int,
    napsterId: [@autoserialize.custom (
        option__to_json(string__to_json)
            %> Js.Json.stringify
            %> Js.Json.string,
        Js.Json.decodeString
            %> Option.get_js
            %> Js.Json.parseExn
            %> option__from_json(string__from_json),
        string__to_devtools
    )] option(string),
    metadataSource: [@autoserialize.custom (
        Models.metadataSource__to_json
            %> Js.Json.stringify
            %> Js.Json.string,
        Js.Json.decodeString
            %> ResultEx.fromOpt(None)
            %> ResultEx.map(Js.Json.parseExn)
            %> ResultEx.bind(Models.metadataSource__from_json),
        string__to_devtools
    )] Models.metadataSource
};

[@autoserialize] type albumsResult = array(album);

let _convertDbAlbum = ({ id, name, napsterId, metadataSource, primaryArtistId }) =>
    { Models.Album.id, name, napsterId, metadataSource, primaryArtistId };

let createFromNapster = (~name, ~napsterId, ~primaryArtist) => {
    open Insert;

    Js.log4("Inserting album from Napster:", primaryArtist.Models.Artist.name, {j|–|j}, name);

    let napsterId = Some(napsterId);

    Insert.make(DbHelper.knex)
        |> into("albums")
        |> set("name", name)
        |> set("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> set("metadataSource", Js.Json.stringify(Models.metadataSource__to_json(Models.Napster)))
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
    switch (albumsResult__from_json(result)) {
        | Ok([| artist |]) => Some(_convertDbAlbum(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(Some(s)) => Js.Exn.raiseError("Error converting albumsResult: " ++ s)
        | Error(_) => Js.Exn.raiseError("Error converting albumsResult")
    };
};

let findByNameAndArtist = (name, primaryArtist) => {
    open Select;

    DbHelper.selectAll("albums")
        |> whereParam("name = ?", ?? name)
        |> whereParam("primaryArtistId = ?", ?? primaryArtist.Models.Artist.id)
        |> whereParam("napsterId = ?", ?? option__to_json((_) => Js.Json.null, None)) /* TODO: this'll have to be generalized */
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleAlbumResult)
        |> tapMaybe(({ Models.Album.id }) => resolve(Js.log({j|Matched album $name [$id] by artist|j})));
};

let findByNapsterId = (napsterId) => {
    open Select;

    let json = Some(napsterId)
        |> option__to_json(string__to_json)
        |> Js.Json.stringify;

    DbHelper.selectAll("albums")
        |> whereParam("napsterId = ?", ?? json)
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleAlbumResult)
        |> tapMaybe(({ Models.Album.id, name }) => resolve(Js.log({j|Matched album $name [$id] by Napster ID|j})));
};

let setNapsterId = (napsterId, { Models.Album.id, name }) => {
    open Update;

    Js.log3("Setting napster ID for album", id, name);

    Update.make("albums", DbHelper.knex)
        |> set("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
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
