open Bluebird;
open Belt.Result;
open ResultEx;
open BsKnex;
open BsKnex.Params.Infix;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let testAffectedRows = DbHelper.testAffectedRows;

let flip = BatPervasives.flip;
let map = BluebirdEx.map;
let pool = DbPool.pool;

let doQuery = (query) => Mysql.Queryable.query(pool, query) |> fromPromise;

let setNapsterId = (userId: int, napsterId: string) => {
    open Insert;

    Insert.make(DbHelper.knex)
        |> into("napster_users")
        |> set("userId", userId)
        |> set("napsterUserId", napsterId)
        |> toString
        |> doQuery
        |> map((_) => ());
};

let create = (name: string) => {
    open Insert;

    Insert.make(DbHelper.knex)
        |> into("users")
        |> set("name", name)
        |> toString
        |> doQuery
        |> map(DbHelper.getInsertId)
};

[@decco] type userIdResult = { userId: int };
[@decco] type userIdSelectResult = array(userIdResult);

let getFromNapsterId = (napsterId: string) => {
    open Select;

    Select.make(DbHelper.knex)
        |> from("napster_users")
        |> column("userId")
        |> whereParam("napsterUserId = ?", ?? napsterId)
        |> toString
        |> doQuery
        |> map(((result, _)) =>
            switch (userIdSelectResult_decode(result)) {
                | Error(_) => Js.Exn.raiseError("Error converting select result")

                | Ok(results) =>
                    switch (Js.Array.length(results)) {
                        | 0 => None
                        | _ => Some(results[0].userId)
                    }
            }
        )
};

/* TODO: encrypt refresh token? */
let setNapsterRefreshToken = (userId: int, refreshToken: string) => {
    open Update;

    Update.make("napster_users", DbHelper.knex)
        |> set("refreshToken", refreshToken)
        |> whereParam("userId = ?", ?? userId)
        |> toString
        |> doQuery
        |> map(testAffectedRows);
};

let _generateAndHash = (salt) =>
    Std.generateRandomBase64()
        |> then_((code) =>
            Std.secureHash(salt, code)
                |> map((hash) => (code, hash))
                |> catch((exn) => {
                    Js.log2("errrr",exn);
                    Js.Exn.raiseError("oops");
                })
        );



let generateAuthCode = (salt, userId: int) => Insert.(
    _generateAndHash(salt)
        |> then_(((code, hash)) => {
            Insert.make(DbHelper.knex)
                |> into("auth_codes")
                |> set("userId", userId)
                |> set("authCodeHash", hash)
                |> set("createdUtc", Std.getCurrentUtc())
                |> toString
                |> doQuery
                |> thenResolve(code);
        })
);

let _deleteHash = (hash: string) => {
    open Delete;

    Delete.make("auth_codes", DbHelper.knex)
        |> whereParam("authCodeHash = ?", ?? hash)
        |> toString
        |> doQuery;
};

let _createAuthToken = (salt, userId: int) => Insert.(
    _generateAndHash(salt)
        |> then_(((token, hash)) =>
            Insert.make(DbHelper.knex)
                |> into("auth_tokens")
                |> set("userId", userId)
                |> set("tokenHash", hash)
                |> set("createdUtc", Std.getCurrentUtc())
                |> toString
                |> doQuery
                |> tap(testAffectedRows)
                |> thenResolve(token)
        )
);

let useCode = (salt, code: string) =>
    Std.secureHash(salt, code)
        |> then_((hash) => Select.(
            Select.make(DbHelper.knex)
                |> from("auth_codes")
                |> column("userId")
                |> whereParam("authCodeHash = ?", ?? hash)
                |> toString
                |> doQuery
                |> map(((result, _)) =>
                    switch (userIdSelectResult_decode(result)) {
                        | Error(_) => Js.Exn.raiseError("Invalid code")
                        | Ok(result) => (result[0].userId, hash)
                    }
                )
        ))
        |> tap(((_, hash)) => _deleteHash(hash))
        |> then_(((userId, _)) => _createAuthToken(salt, userId));

[@decco] type userArray = array(Models.User.t);

let getUserFromToken = (salt, token: string) =>
    Std.secureHash(salt, token)
        |> map((hash) => Select.(
            Select.make(DbHelper.knex)
                |> from(~alias="at", "auth_tokens")
                |> innerJoin("users as u", "u.id", "=", "at.userId")
                |> column("u.*")
                |> whereParam("at.tokenHash = ?", ?? hash)
                |> toString
        ))
        |> then_(doQuery)
        |> map(((result, _)) => userArray_decode(result))
        |> map(toOpt)
        |> map(flip(Option.bind, Js.Array.pop));
