open BsSquel;
open BsSquel.Params.Infix;
open Bluebird;
open BatPervasives;
module BluebirdEx = PromiseEx.Make(Bluebird);

let map = BluebirdEx.map;

let testAffectedRows = DbHelper.testAffectedRows;
let pool = DbPool.pool;

let doQuery = (query) => Mysql.Queryable.query(pool, query) |> fromPromise;

[@autoserialize] type artist = {
    id: int,
    name: string,
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

[@autoserialize] type artistsResult = array(artist);

let _convertDbArtist = ({ id, name, napsterId, metadataSource }) =>
    { Models.Artist.id, name, napsterId, metadataSource };

let createFromNapster = (name: string, napsterId: string) => {
    open Insert;
    let napsterId = Some(napsterId);

    Insert.make()
        |> into("artists")
        |> setString("name", name)
        |> setString("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> setString("metadataSource", Js.Json.stringify(Models.metadataSource__to_json(Models.Napster)))
        |> setString("createdUtc", Std.getCurrentUtc())
        |> toString
        |> doQuery
        |> map(DbHelper.getInsertId)
        |> map(id => {
            Models.Artist.id, name, napsterId,
            metadataSource: Models.Napster
        });
};

let _selectArtistFields = Select.(
    Select.make
        %> from("artists")
        %> field("*")
);

let _parseSingleArtistResult = ((result, _)) => {
    switch (artistsResult__from_json(result)) {
        | Ok([| artist |]) => Some(_convertDbArtist(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(Some(s)) => Js.Exn.raiseError("Error converting artistsResult: " ++ s)
        | Error(_) => Js.log2("hmm",result); Js.Exn.raiseError("Error converting artistsResult")
    };
};

let findByName = (name) => {
    open Select;

    _selectArtistFields()
        |> where("name = ?" |?. name)
        |> toString
        |> doQuery
        |> map(_parseSingleArtistResult);
};

let findByNapsterId = (napsterId) => {
    open Select;

    let json = Some(napsterId)
        |> option__to_json(string__to_json)
        |> Js.Json.stringify;

    _selectArtistFields()
        |> where("napsterId = ?" |?. json)
        |> toString
        |> doQuery
        |> map(_parseSingleArtistResult);
};

let setNapsterId = (napsterId, { Models.Artist.id }) => {
    open Update;

    Update.make()
        |> table("artists")
        |> setString("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> where("id = ?" |?. id)
        |> toString
        |> doQuery
        |> map(testAffectedRows)
};
