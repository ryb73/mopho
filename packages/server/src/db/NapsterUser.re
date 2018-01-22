open BsSquel;
open BsSquel.Params.Infix;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;


let doQuery = (query) => Mysql.Queryable.query(DbPool.pool, query) |> fromPromise;

[@autoserialize] type refreshTokenResult = { refreshToken: string };
[@autoserialize] type getRefreshTokenResult = array(refreshTokenResult);
let getRefreshToken = (userId : Models.User.id) => {
    open Select;

    Select.make()
        |> from("napster_users")
        |> field("refreshToken")
        |> where("userId = ?" |?. userId)
        |> toString
        |> doQuery
        |> map(((result, _)) => {
            switch (getRefreshTokenResult__from_json(result)) {
                | Error(_) => Js.Exn.raiseError("Error converting select result")
                | Ok([| { refreshToken } |]) => Some(refreshToken)
                | Ok([||]) => None
                | _ => Js.Exn.raiseError("Multiple refreshTokens found for user")
            };
        });
};