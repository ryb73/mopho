open BsKnex;
open BsKnex.Params.Infix;
open Bluebird;
open BatPervasives;
module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

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

let _selectArtistFields = () => Select.(
    Select.make(DbHelper.knex)
        |> from("artists")
        |> column("*")
);

let _parseSingleArtistResult = ((result, _)) => {
    switch (artistsResult__from_json(result)) {
        | Ok([| artist |]) => Some(_convertDbArtist(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(Some(s)) => Js.log(result); Js.Exn.raiseError("Error converting artistsResult: " ++ s)
        | Error(_) => Js.Exn.raiseError("Error converting artistsResult")
    };
};

let findByName = (name) => {
    open Select;

    Js.log2("Trying to match artist", name);

    _selectArtistFields()
        |> whereParam("name = ?", ?? name)
        |> whereParam("napsterId = ?", ?? option__to_json((_) => Js.Json.null, None)) /* TODO: this'll have to be generalized */
        |> toString
        |> doQuery
        |> map(_parseSingleArtistResult)
        |> tapMaybe(({ Models.Artist.id }) => resolve(Js.log({j|Matched artist $name [$id] by name|j})))
        /* |> map((v) => {
            switch v {
                | None => Js.log({j|No match: $name|j})
                | _ => ()
            };

            v;
        }) */
};

let findByNapsterId = (napsterId) => {
    open Select;

    let json = Some(napsterId)
        |> option__to_json(string__to_json)
        |> Js.Json.stringify;

    _selectArtistFields()
        |> whereParam("napsterId = ?", ?? json)
        |> toString
        |> doQuery
        |> map(_parseSingleArtistResult)
        |> tapMaybe(({ Models.Artist.id, name }) => resolve(Js.log({j|Matched artist $name [$id] by Napster ID|j})));
};

let createFromNapster = (name, napsterId) => {
    open Insert;

    Js.log2("Inserting artist from Napster:", name);

    Insert.make(DbHelper.knex)
        |> into("artists")
        |> set("name", name)
        |> set("napsterId", Js.Json.stringify(option__to_json(string__to_json, Some(napsterId))))
        |> set("metadataSource", Js.Json.stringify(Models.metadataSource__to_json(Models.Napster)))
        |> set("createdUtc", Std.getCurrentUtc())
        |> toString
        |> Js.String.replaceByRe([%bs.re "/^insert/i"], "insert ignore")
        |> doQuery
        |> map(DbHelper.getInsertId)
        |> then_(id => {
            /* If insert failed, id will be 0. This can happen if the artist was already inserted */
            if(id === 0) {
                findByNapsterId(napsterId)
                    |> map(result => {
                        switch result {
                            | None => failwith({j|Error in createFromNapster: $name, $napsterId|j})
                            | Some(r) => r
                        };
                    });
            } else {
                resolve({
                    Models.Artist.id, name,
                    napsterId: Some(napsterId),
                    metadataSource: Models.Napster
                });
            };
        });
};

let setNapsterId = (napsterId, { Models.Artist.id }) => {
    open Update;

    Js.log2("Setting napster ID for artist", id);

    Update.make("artists", DbHelper.knex)
        |> set("napsterId", Js.Json.stringify(option__to_json(string__to_json, napsterId)))
        |> whereParam("id = ?", ?? id)
        |> toString
        |> doQuery
        |> map(testAffectedRows)
};

let matchNapster = (name, napsterId) =>
    findByNapsterId(napsterId)
        |> then_(fun
            | Some(v) => resolve(Some(v))
            | None => findByName(name)
                |> tapMaybe(setNapsterId(Some(napsterId)))
        )
        |> then_(fun
            | Some(a) => resolve(a)
            | None => createFromNapster(name, napsterId)
        )
