open BsKnex;
open BsKnex.Params.Infix;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let doQuery = (query) => Mysql.Queryable.query(DbPool.pool, query) |> fromPromise;

[@decco] type refreshTokenResult = { refreshToken: string };
[@decco] type getRefreshTokenResult = array(refreshTokenResult);
let getRefreshToken = (userId : Models.User.id) => {
    open Select;

    Select.make(DbHelper.knex)
        |> from("napster_users")
        |> column("refreshToken")
        |> whereParam("userId = ?", ?? userId)
        |> toString
        |> doQuery
        |> map(((result, _)) => {
            switch (getRefreshTokenResult_decode(result)) {
                | Error(_) => Js.Exn.raiseError("Error converting select result")
                | Ok([| { refreshToken } |]) => Some(refreshToken)
                | Ok([||]) => None
                | _ => Js.Exn.raiseError("Multiple refreshTokens found for user")
            };
        });
};