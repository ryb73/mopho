open Squel;
open Params.Infix;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let testAffectedRows = DbHelper.testAffectedRows;
let pool = DbPool.pool;

let doQuery = (query) => Mysql.Queryable.query(pool, query) |> fromPromise;

[@autoserialize] type artistsResult = array(Models.Artist.t);

let _selectArtistFields = Select.(
    Select.make()
        |> from("artists")
        |> field("id")
        |> field("name")
        |> field("napsterId")
);

let findByName = (name) => {
    open Select;

    _selectArtistFields
        |> where("name = ?" |?. name)
        |> toString
        |> doQuery
        |> map(((result, _)) => {
            switch (artistsResult__from_json(result)) {
                | Ok([| artist |]) => Some(artist)
                | Ok([||]) => None
                | Ok(_) => Js.Exn.raiseError("Multiple matches found")
                | Error(_) => Js.Exn.raiseError("Error converting artistsResult")
            };
        });
};

let findByNapsterId = (napsterId) => {
    open Select;

    _selectArtistFields
        |> where("napsterId = ?" |?. napsterId)
        |> toString
        |> doQuery
        |> map(((result, _)) => {
            switch (artistsResult__from_json(result)) {
                | Ok([| artist |]) => Some(artist)
                | Ok([||]) => None
                | Ok(_) => Js.Exn.raiseError("Multiple matches found")
                | Error(_) => Js.Exn.raiseError("Error converting artistsResult")
            };
        });
};

let setNapsterId = (napsterId, { Models.Artist.id }) => {
    open Update;

    Update.make()
        |> table("artists")
        |> set("napsterId", napsterId)
        |> where("id = ?" |?. id)
        |> toString
        |> doQuery
        |> map(testAffectedRows)
};
