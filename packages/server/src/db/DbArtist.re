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

[@decco] type artist = {
    id: int,
    name: string,
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

[@decco] type artistsResult = array(artist);

let _convertDbArtist = ({ id, name, napsterId, metadataSource }) =>
    { Models.Artist.id, name, napsterId, metadataSource };

let _selectArtistFields = () => Select.(
    Select.make(DbHelper.knex)
        |> from("artists")
        |> column("*")
);

let _parseSingleArtistResult = ((result, _)) => {
    switch (artistsResult_decode(result)) {
        | Ok([| artist |]) => Some(_convertDbArtist(artist))
        | Ok([||]) => None
        | Ok(_) => Js.Exn.raiseError("Multiple matches found")
        | Error(e) => Js.log2(result, e); Js.Exn.raiseError("Error converting artistsResult")
    };
};

let findByName = (name) => {
    open Select;

    Js.log2("Trying to match artist", name);

    _selectArtistFields()
        |> whereParam("name = ?", ?? name)
        |> where("napsterId IS NULL") /* TODO: this'll have to be generalized */
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

    _selectArtistFields()
        |> whereParam("napsterId = ?", ?? napsterId)
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
        |> set("napsterId", napsterId)
        |> set("metadataSource", Js.Json.stringify(Models.metadataSource_encode(Models.Napster)))
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
        |> set("napsterId", napsterId)
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
