open Js.Promise;
open Js.Result;
open PromiseEx;
open Squel;
open Params.Infix;
open Option.Infix;
open MomentRe;

let flip = BatPervasives.flip;

let pool = DbPool.pool;

let setNapsterId (userId : int) (napsterId : string) => {
    open Insert;

    let query = Insert.make ()
        |> into "napster_users"
        |> set "userId" userId
        |> set "napsterUserId" napsterId
        |> toString;

    Mysql.Queryable.query pool query
        |> map (fun _ => ());
};

type insertResult = {
    insertId: int
};

let create (name : string) => {
    open Insert;

    let query = Insert.make ()
        |> into "users"
        |> set "name" name
        |> toString;

    Mysql.Queryable.query pool query
        |> then_ (fun (result, _) => {
            switch (insertResult__from_json result) {
                | Error _ => failwith "Error converting insert result"
                | Ok { insertId } => resolve insertId
            };
        });
};

type userIdResult = { userId: int };
type userIdSelectResult = array userIdResult;

let getFromNapsterId (napsterId : string) => {
    open Select;

    let query = Select.make ()
        |> from "napster_users"
        |> field "userId"
        |> where ("napsterUserId = ?" |?. napsterId)
        |> toString;

     Mysql.Queryable.query pool query
        |> map (fun (result, _) => {
            switch (userIdSelectResult__from_json result) {
                | Error _ => failwith "Error converting select result"
                | Ok results =>
                    switch (Js.Array.length results) {
                        | 0 => None
                        | _ => Some results.(0).userId
                    }
            };
        });
};

let _testAffectedRows ::expected=1 (result, _) => {
    let affectedRows = Js.Json.decodeObject result
        >>= flip Js.Dict.get "affectedRows"
        >>= Js.Json.decodeNumber;

    switch affectedRows {
        | Some actual =>
            if(Js.Math.floor actual === expected) {
                ()
            } else {
                failwith ("Affected rows " ^ (Js.String.make actual) ^ " <> " ^ (Js.String.make expected))
            }
        | _ => failwith ("Affected rows empty")
    }
};

let setNapsterRefreshToken (userId : int) (refreshToken : string) => {
    open Update;

    let query = Update.make ()
        |> table "napster_users"
        |> set "refreshToken" refreshToken
        |> where ("userId = ?" |?. userId)
        |> toString;

    Mysql.Queryable.query pool query
        |> map _testAffectedRows;
};

let _generateAndHash salt => {
    Std.generateRandomBase64 ()
        |> then_ (fun code => {
            Std.secureHash salt code
                |> map @@ fun hash => (code, hash);
        });
};

let getCurrentUtc () => momentUtc ()
    |> Moment.defaultFormat;

let generateAuthCode salt (userId : int) => {
    open Insert;

    _generateAndHash salt
        |> then_ (fun (code, hash) => {
            let query = Insert.make ()
                |> into "auth_codes"
                |> set "userId" userId
                |> set "authCodeHash" hash
                |> set "createdUtc" (getCurrentUtc ())
                |> toString;

            Mysql.Queryable.query pool query
                |> thenResolve code;
        });
};

let _deleteHash (hash : string) => {
    open Delete;

    let query = Delete.make ()
        |> from "auth_codes"
        |> where ("authCodeHash = ?" |?. hash)
        |> toString;

    Mysql.Queryable.query pool query;
};

let _createAuthToken salt (userId : int) => {
    open Insert;

    _generateAndHash salt
        |> then_ (fun (token, hash) => {
            let query = Insert.make ()
                |> into "auth_tokens"
                |> set "userId" userId
                |> set "tokenHash" hash
                |> set "createdUtc" (getCurrentUtc ())
                |> toString;

            Mysql.Queryable.query pool query
                |> tap _testAffectedRows
                |> thenResolve token;
        });
};

let useCode salt (code : string) => {
    Std.secureHash salt code
        |> then_ (fun hash => {
            open Select;

            let query = Select.make ()
                |> from "auth_codes"
                |> field "userId"
                |> where ("authCodeHash = ?" |?. hash)
                |> toString;

            Mysql.Queryable.query pool query
                |> map (fun (result, _) => {
                    switch (userIdSelectResult__from_json result) {
                        | Error _ => failwith "Invalid code"
                        | Ok result => (result.(0).userId, hash)
                    };
                });
        })
        |> tap (fun (_, hash) => _deleteHash hash)
        |> then_ (fun (userId, _) => _createAuthToken salt userId);
};