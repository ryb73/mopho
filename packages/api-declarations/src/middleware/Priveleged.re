let authTokenCookie = "auth_token";
exception AuthenticationException;

let apply (handle, request) => {
    let handle req _ _ _ i => {
        req
            |> Express.Request.cookies
            |> BatPervasives.flip Js.Dict.get authTokenCookie
            |> Option.map @@ Db.User.getUserFromToken @@ Std.getIp req
            |> PromiseEx.invertOptional
            |> PromiseEx.map (fun user => {
                switch user {
                    | None => GetMyUserData.ErrorCode 403
                    | Some user => GetMyUserData.Result user;
                };
            });
    };

    (handle, request);
};