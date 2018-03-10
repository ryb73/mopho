open BsKnex;
open BsKnex.Params.Infix;
open BatPervasives;
open Bluebird;
module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let map = BluebirdEx.map;

[@autoserialize] type track = {
    id: int,
    name: string,
    primaryArtistId: int,
    albumId: int,
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

[@autoserialize] type tracksResult = array(track);

let _convertDbTrack = ({ id, name, napsterId, metadataSource, primaryArtistId, albumId }) =>
    { Models.Track.id, name, napsterId, metadataSource, primaryArtistId, albumId };

let createFromNapster = (~name, ~napsterId, ~primaryArtist, ~album) => {
    open Insert;

    Js.log4("Inserting track from Napster:", primaryArtist.Models.Artist.name, {j|–|j}, name);

    let napsterId = Some(napsterId);

    Insert.make(DbHelper.knex)
        |> into("tracks")
        |> set("name", name)
        |> set("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> set("metadataSource", Js.Json.stringify(Models.metadataSource__to_json(Models.Napster)))
        |> set("createdUtc", Std.getCurrentUtc())
        |> set("primaryArtistId", primaryArtist.id)
        |> set("albumId", album.Models.Album.id)
        |> toString
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
    switch (tracksResult__from_json(result)) {
        | Ok([| artist |]) => Some(_convertDbTrack(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(Some(s)) => Js.Exn.raiseError("Error converting albumsResult: " ++ s)
        | Error(_) => Js.Exn.raiseError("Error converting albumsResult")
    };
};

let findByNameAndArtist = (name, primaryArtist) => {
    open Select;

    let { Models.Artist.id: primaryArtistId, name: primaryArtistName } = primaryArtist;

    DbHelper.selectAll("tracks")
        |> whereParam("name = ?", ?? name)
        |> whereParam("primaryArtistId = ?", ?? primaryArtistId)
        |> whereParam("napsterId = ?", ?? option__to_json((_) => Js.Json.null, None)) /* TODO: this'll have to be generalized */
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleTrackResult)
        |> tapMaybe(({ Models.Track.id }) => resolve(Js.log({j|Matched track $name [$id] by artist ($primaryArtistName [$primaryArtistId])|j})));
};

let findByNapsterId = (napsterId) => {
    open Select;

    let json = Some(napsterId)
        |> option__to_json(string__to_json)
        |> Js.Json.stringify;

    DbHelper.selectAll("tracks")
        |> whereParam("napsterId = ?", ?? json)
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleTrackResult)
        |> tapMaybe(({ Models.Track.id, name }) => resolve(Js.log({j|Matched track $name [$id] by Napster ID|j})));
};

let setNapsterId = (napsterId, { Models.Track.id }) => {
    open Update;

    Js.log2("Setting napster ID for track", id);

    Update.make("tracks", DbHelper.knex)
        |> set("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
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
