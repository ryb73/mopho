open Option.Infix;
open BatPervasives;
open Bluebird;
open BsKnex;

let knex = Core.make("mysql");

[@decco] type insertResult = { insertId: int };

let testAffectedRows = (~expected=1, (result, _)) => {
    let affectedRows = Js.Json.decodeObject(result)
        >>= flip(Js.Dict.get, "affectedRows")
        >>= Js.Json.decodeNumber;

    switch affectedRows {
        | Some(actual) =>
            if (Js.Math.floor(actual) === expected) {
                ();
            } else {
                Js.Exn.raiseError("Affected rows " ++ Js.String.make(actual) ++ " <> " ++ Js.String.make(expected));
            }
        | _ => Js.Exn.raiseError("Affected rows empty")
    };
};

let getInsertId = ((result, _)) =>
    switch (insertResult_decode(result)) {
        | Error(_) => Js.Exn.raiseError("Error converting insert result")
        | Ok({insertId}) => insertId
    };

let doQuery = (query) => Mysql.Queryable.query(DbPool.pool, query) |> fromPromise;

let selectAll = (table) => Select.(
    Select.make(knex)
        |> from(table)
        |> column("*")
);