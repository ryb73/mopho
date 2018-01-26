open BsSquel;
open BsSquel.Params.Infix;
open BatPervasives;
open Bluebird;
module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

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

    Insert.make()
        |> into("albums")
        |> setString("name", name)
        |> setString("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> setString("metadataSource", Js.Json.stringify(Models.metadataSource__to_json(Models.Napster)))
        |> setString("createdUtc", Std.getCurrentUtc())
        |> setInt("primaryArtistId", primaryArtist.id)
        |> setInt("albumId", album.Models.Album.id)
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
        |> where("name = ?" |?. name)
        |> where("primaryArtistId = ?" |?. primaryArtistId)
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
        |> where("napsterId = ?" |?. json)
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleTrackResult)
        |> tapMaybe(({ Models.Track.id, name }) => resolve(Js.log({j|Matched track $name [$id] by Napster ID|j})));
};

let setNapsterId = (napsterId, { Models.Track.id }) => {
    open Update;

    Js.log2("Setting napster ID for track ", id);

    Update.make()
        |> table("tracks")
        |> setString("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> where("id = ?" |?. id)
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
