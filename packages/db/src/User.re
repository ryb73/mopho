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

let testAffectedRows ::expected=1 (result, _) => {
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
        |> map testAffectedRows;
};

let generateAuthCode salt (userId : int) => {
    open Insert;

    Std.generateRandomBase64 ()
        |> then_ (fun code => {
            Std.secureHash salt code
                |> then_ (fun buf => {
                    let currentUtc = momentUtc ()
                        |> Moment.defaultFormat;

                    let query = Insert.make ()
                        |> into "auth_codes"
                        |> set "userId" userId
                        |> set "authCodeHash" (Base64Url.fromBuffer buf)
                        |> set "createdUtc" currentUtc
                        |> toString;

                    Mysql.Queryable.query pool query
                })
                |> map testAffectedRows
                |> thenResolve code;
        });
};