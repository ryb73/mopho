open Bluebird;
open Js.Result;
open ResultEx;
open BsSquel;
open BsSquel.Params.Infix;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let testAffectedRows = DbHelper.testAffectedRows;

let flip = BatPervasives.flip;
let map = BluebirdEx.map;
let pool = DbPool.pool;

let doQuery = (query) => Mysql.Queryable.query(pool, query) |> fromPromise;

let setNapsterId = (userId: int, napsterId: string) => {
    open Insert;

    Insert.make()
        |> into("napster_users")
        |> setInt("userId", userId)
        |> setString("napsterUserId", napsterId)
        |> toString
        |> doQuery
        |> map((_) => ());
};

let create = (name: string) => {
    open Insert;

    Insert.make()
        |> into("users")
        |> setString("name", name)
        |> toString
        |> doQuery
        |> map(DbHelper.getInsertId)
};

[@autoserialize] type userIdResult = { userId: int };
[@autoserialize] type userIdSelectResult = array(userIdResult);

let getFromNapsterId = (napsterId: string) => {
    open Select;

    Select.make()
        |> from("napster_users")
        |> field("userId")
        |> where("napsterUserId = ?" |?. napsterId)
        |> toString
        |> doQuery
        |> map(((result, _)) =>
            switch (userIdSelectResult__from_json(result)) {
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

    Update.make()
        |> table("napster_users")
        |> setString("refreshToken", refreshToken)
        |> where("userId = ?" |?. userId)
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
            Insert.make()
                |> into("auth_codes")
                |> setInt("userId", userId)
                |> setString("authCodeHash", hash)
                |> setString("createdUtc", Std.getCurrentUtc())
                |> toString
                |> doQuery
                |> thenResolve(code);
        })
);

let _deleteHash = (hash: string) => {
    open Delete;

    Delete.make()
        |> from("auth_codes")
        |> where("authCodeHash = ?" |?. hash)
        |> toString
        |> doQuery;
};

let _createAuthToken = (salt, userId: int) => Insert.(
    _generateAndHash(salt)
        |> then_(((token, hash)) =>
            Insert.make()
                |> into("auth_tokens")
                |> setInt("userId", userId)
                |> setString("tokenHash", hash)
                |> setString("createdUtc", Std.getCurrentUtc())
                |> toString
                |> doQuery
                |> tap(testAffectedRows)
                |> thenResolve(token)
        )
);

let useCode = (salt, code: string) =>
    Std.secureHash(salt, code)
        |> then_((hash) => Select.(
            Select.make()
                |> from("auth_codes")
                |> field("userId")
                |> where("authCodeHash = ?" |?. hash)
                |> toString
                |> doQuery
                |> map(((result, _)) =>
                    switch (userIdSelectResult__from_json(result)) {
                        | Error(_) => Js.Exn.raiseError("Invalid code")
                        | Ok(result) => (result[0].userId, hash)
                    }
                )
        ))
        |> tap(((_, hash)) => _deleteHash(hash))
        |> then_(((userId, _)) => _createAuthToken(salt, userId));

[@autoserialize] type userArray = array(Models.User.t);

let getUserFromToken = (salt, token: string) =>
    Std.secureHash(salt, token)
        |> map((hash) => Select.(
            Select.make()
                |> from(~alias="at", "auth_tokens")
                |> join(~alias="u", "users", "u.id = at.userId")
                |> field("u.*")
                |> where("at.tokenHash = ?" |?. hash)
                |> toString
        ))
        |> then_(doQuery)
        |> map(((result, _)) => userArray__from_json(result))
        |> map(toOpt)
        |> map(flip(Option.bind, Js.Array.pop));
