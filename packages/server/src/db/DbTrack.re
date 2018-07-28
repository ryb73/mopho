open BsKnex;
open BsKnex.Params.Infix;
open BatPervasives;
open Bluebird;
module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let map = BluebirdEx.map;

[@decco] type track = {
    id: int,
    name: string,
    primaryArtistId: int,
    albumId: int,
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

[@decco] type tracksResult = array(track);

let _convertDbTrack = ({ id, name, napsterId, metadataSource, primaryArtistId, albumId }) =>
    { Models.Track.id, name, napsterId, metadataSource, primaryArtistId, albumId };

let createFromNapster = (~name, ~napsterId, ~primaryArtist, ~album) => {
    open Insert;

    Js.log4("Inserting track from Napster:", primaryArtist.Models.Artist.name, {j|–|j}, name);

    let napsterId = Some(napsterId);

    Insert.make(DbHelper.knex)
        |> into("tracks")
        |> set("name", name)
        |> set("napsterId", napsterId)
        |> set("metadataSource", Js.Json.stringify(Models.metadataSource_encode(Models.Napster)))
        |> set("createdUtc", Std.getCurrentUtc())
        |> set("primaryArtistId", primaryArtist.id)
        |> set("albumId", album.Models.Album.id)
        |> toString
        |> Js.String.replaceByRe([%bs.re "/^insert/i"], "insert ignore")
        |> DbHelper.doQuery
        |> map(DbHelper.getInsertId)
        |> map(id => {
            Models.Track.id, name, napsterId,
            primaryArtistId: primaryArtist.id,
            albumId: album.id,
            metadataSource: Models.Napster
        });
};

let _parseSingleTrackResult = ((result, _)) => {
    switch (tracksResult_decode(result)) {
        | Ok([| artist |]) => Some(_convertDbTrack(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(e) => Js.log(e); Js.Exn.raiseError("Error converting albumsResult")
    };
};

let findByNameAndArtist = (name, primaryArtist) => {
    open Select;

    let { Models.Artist.id: primaryArtistId, name: primaryArtistName } = primaryArtist;

    DbHelper.selectAll("tracks")
        |> whereParam("name = ?", ?? name)
        |> whereParam("primaryArtistId = ?", ?? primaryArtistId)
        |> where("napsterId IS NULL") /* TODO: this'll have to be generalized */
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleTrackResult)
        |> tapMaybe(({ Models.Track.id }) => resolve(Js.log({j|Matched track $name [$id] by artist ($primaryArtistName [$primaryArtistId])|j})));
};

let findByNapsterId = (napsterId) => {
    open Select;

    DbHelper.selectAll("tracks")
        |> whereParam("napsterId = ?", ?? napsterId)
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleTrackResult)
        |> tapMaybe(({ Models.Track.id, name }) => resolve(Js.log({j|Matched track $name [$id] by Napster ID|j})));
};

let setNapsterId = (napsterId, { Models.Track.id }) => {
    open Update;

    Js.log2("Setting napster ID for track", id);

    Update.make("tracks", DbHelper.knex)
        |> set("napsterId", napsterId)
        |> whereParam("id = ?", ?? id)
        |> toString
        |> DbHelper.doQuery
        |> map(DbHelper.testAffectedRows)
};

let matchNapster = (track) => {
    let { BsNapsterApi.Types.Track.id: napsterId, name } = track;

    findByNapsterId(napsterId)
        |> then_(fun
            | Some(a) => resolve(a)
            | None =>
                DbArtist.matchNapster(track.artistName, track.artistId)
                    |> then_(primaryArtist =>
                        findByNameAndArtist(name, primaryArtist)
                            |> tapMaybe(setNapsterId(Some(napsterId)))
                            |> then_(fun
                                | Some(a) => resolve(a)
                                | None => DbAlbum.matchNapster(track.albumName, track.albumId, track.artistName, track.artistId)
                                    |> then_((album) => createFromNapster(
                                        ~name, ~napsterId, ~primaryArtist, ~album
                                    ))
                            )
                    )
        );
};
