open Js.Promise;
open Js.Result;
open ResultEx;
open PromiseEx;
open Squel;
open Params.Infix;
open Option.Infix;
open MomentRe;

let flip = BatPervasives.flip;
let map = PromiseEx.map;
let pool = DbPool.pool;

let doQuery = (query) => Mysql.Queryable.query(pool, query);

let setNapsterId = (userId: int, napsterId: string) => {
    open Insert;

    Insert.make()
        |> into("napster_users")
        |> set("userId", userId)
        |> set("napsterUserId", napsterId)
        |> toString
        |> doQuery
        |> map((_) => ());
};

type insertResult = { insertId: int };

let create = (name: string) => {
    open Insert;

    Insert.make()
        |> into("users")
        |> set("name", name)
        |> toString
        |> doQuery
        |> then_(((result, _)) =>
            switch (insertResult__from_json(result)) {
                | Error(_) => failwith("Error converting insert result")
                | Ok({insertId}) => resolve(insertId)
            }
        );
};

type userIdResult = { userId: int };
type userIdSelectResult = array(userIdResult);

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
                | Error(_) => failwith("Error converting select result")

                | Ok(results) =>
                    switch (Js.Array.length(results)) {
                        | 0 => None
                        | _ => Some(results[0].userId)
                    }
            }
        )
};

let _testAffectedRows = (~expected=1, (result, _)) => {
    let affectedRows = Js.Json.decodeObject(result)
        >>= flip(Js.Dict.get, "affectedRows")
        >>= Js.Json.decodeNumber;

    switch affectedRows {
        | Some(actual) =>
            if (Js.Math.floor(actual) === expected) {
                ();
            } else {
                failwith("Affected rows " ++ Js.String.make(actual) ++ " <> " ++ Js.String.make(expected));
            }
        | _ => failwith("Affected rows empty")
    };
};

let setNapsterRefreshToken = (userId: int, refreshToken: string) => {
    open Update;

    Update.make()
        |> table("napster_users")
        |> set("refreshToken", refreshToken)
        |> where("userId = ?" |?. userId)
        |> toString
        |> doQuery
        |> map(_testAffectedRows);
};

let _generateAndHash = (salt) =>
    Std.generateRandomBase64()
        |> then_((code) =>
            Std.secureHash(salt, code)
                |> map((hash) => (code, hash))
        );

let getCurrentUtc = () => momentUtc() |> Moment.defaultFormat;

let generateAuthCode = (salt, userId: int) => Insert.(
    _generateAndHash(salt)
        |> then_(((code, hash)) => {
            Insert.make()
                |> into("auth_codes")
                |> set("userId", userId)
                |> set("authCodeHash", hash)
                |> set("createdUtc", getCurrentUtc())
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
                |> set("userId", userId)
                |> set("tokenHash", hash)
                |> set("createdUtc", getCurrentUtc())
                |> toString
                |> doQuery
                |> tap(_testAffectedRows)
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
                        | Error(_) => failwith("Invalid code")
                        | Ok(result) => (result[0].userId, hash)
                    }
                )
        ))
        |> tap(((_, hash)) => _deleteHash(hash))
        |> then_(((userId, _)) => _createAuthToken(salt, userId));

type userArray = array(Models.User.t);

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
