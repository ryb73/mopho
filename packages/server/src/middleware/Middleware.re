open EndpointFactory;
open Express;
open Option;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let mapP = BluebirdEx.map;
let mapO = Option.map;

module Priveleged = {
    let authTokenCookie = "auth_token";

    module Make = (Endpoint: Endpoint) => {
        include Endpoint;

        let handle = (app, callback) =>
            Endpoint.handle(app, (req, resp, next, data) => {
                Js.log("priv req");
                req
                    |> Request.cookies
                    |> flip(Js.Dict.get, authTokenCookie)
                    |> mapO(DbUser.getUserFromToken(Std.getIp(req)))
                    |> invertOptional
                    |> mapP(flatten)
                    |> then_((optUser) =>
                        switch optUser {
                            | None => resolve(ErrorCode(401))
                            | Some(user) => callback(req, resp, next, data, user)
                        }
                    );
            });
    };
};
