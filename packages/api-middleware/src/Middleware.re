module Priveleged = {
    open EndpointFactory;
    open Express;
    open Option;
    open PromiseEx;

    let mapP = PromiseEx.map;
    let mapO = Option.map;

    let authTokenCookie = "auth_token";

    module Make = fun (Endpoint : Endpoint) => {
        include Endpoint;

        let handle app callback => {
            Endpoint.handle app (fun req resp next data => {
                Js.log "priv req";
                req
                    |> Request.cookies
                    |> flip Js.Dict.get authTokenCookie
                    |> mapO (Db.User.getUserFromToken (Std.getIp req))
                    |> invertOptional
                    |> mapP flatten
                    |> mapP (fun optUser => {
                        switch optUser {
                            | None => ErrorCode 403
                            | Some user => callback req resp next data user
                        };
                    });
            });
        };
    };
};
