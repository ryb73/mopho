open BsSquel;
open BsSquel.Params.Infix;
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

let createFromNapster = (album) => {
    let { BsNapsterApi.Types.Album.name, id: napsterId } = album;

    Js.log4("Inserting album from Napster:", album.artistName, {j|–|j}, name);

    let napsterId = Some(napsterId);

    DbArtist.matchNapster(album.artistName, album.contributingArtists.primaryArtist)
        |> then_(({ Models.Artist.id: primaryArtistId }) => Insert.(
            Insert.make()
                |> into("albums")
                |> setString("name", name)
                |> setString("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
                |> setString("metadataSource", Js.Json.stringify(Models.metadataSource__to_json(Models.Napster)))
                |> setString("createdUtc", Std.getCurrentUtc())
                |> setInt("primaryArtistId", primaryArtistId)
                |> toString
                |> DbHelper.doQuery
                |> map(DbHelper.getInsertId)
                |> map(id => {
                    Models.Album.id, name, napsterId, primaryArtistId,
                    metadataSource: Models.Napster
                })
        ));
};

let _selectAlbumFields = Select.(
    Select.make
        %> from("albums")
        %> field("*")
);

let _parseSingleAlbumResult = ((result, _)) => {
    switch (albumsResult__from_json(result)) {
        | Ok([| artist |]) => Some(_convertDbAlbum(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(Some(s)) => Js.Exn.raiseError("Error converting albumsResult: " ++ s)
        | Error(_) => Js.Exn.raiseError("Error converting albumsResult")
    };
};

let findByNameAndArtist = (name, artistId) => {
    open Select;

    _selectAlbumFields()
        |> where("name = ?" |?. name)
        |> where("primaryArtistId = ?" |?. artistId)
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

    _selectAlbumFields()
        |> where("napsterId = ?" |?. json)
        |> toString
        |> DbHelper.doQuery
        |> map(_parseSingleAlbumResult)
        |> tapMaybe(({ Models.Album.id, name }) => resolve(Js.log({j|Matched album $name [$id] by Napster ID|j})));
};

let setNapsterId = (napsterId, { Models.Album.id }) => {
    open Update;

    Js.log2("Setting napster ID for album ", id);

    Update.make()
        |> table("albums")
        |> setString("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> where("id = ?" |?. id)
        |> toString
        |> DbHelper.doQuery
        |> map(DbHelper.testAffectedRows)
};

let matchNapster = (album) => {
    let { BsNapsterApi.Types.Album.id: napsterId, name } = album;

    findByNapsterId(napsterId)
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None =>
                DbArtist.matchNapster(album.artistName, album.contributingArtists.primaryArtist)
                    |> then_(({ Models.Artist.id: primaryArtistId }) =>
                        findByNameAndArtist(primaryArtistId, name)
                    )
                    |> tapMaybe(setNapsterId(Some(napsterId)))
        )
        |> then_(fun
            | Some(a) => resolve(a)
            | None => createFromNapster(album)
        );
};
