open Express;
open Node;
open Js.Promise;
open Option;

let flip = BatPervasives.flip;
let config = ConfigLoader.config;

let app = App.make ();

let corserOpts = Corser.opts origins::[| config.origin |] supportsCredentials::Js.true_ ();
App.use app @@ Corser.express corserOpts;

external bodyParserJson : unit => Middleware.t = "json" [@@bs.module "body-parser"];
App.use app @@ bodyParserJson ();

type session = string;
module Session = ExpressSession.Make({ type t = session [@@noserialize]; });

App.use app (ExpressSession.make @@ ExpressSession.opts
    secret::config.sessionSecret resave::false saveUninitialized::false
    cookie::(ExpressSession.cookieOpts secure::config.secure ()) ()
);

let generateState ()  => {
    Js.Promise.make @@ fun ::resolve ::reject => {
        Crypto.randomBytes 16 (fun result => {
            switch result {
                | `Exception e => reject (Js.Exn.internalToOCamlException @@ Obj.magic e) [@bs]
                | `Buffer buffer => resolve (Base64Url.fromBuffer buffer) [@bs]
            };
        });
    };
};

let return500 resp => Response.status resp 500
    |> Response.end_
    |> resolve;

generateState ();

module GenerateState = {
    type req = unit;
    type resp = string;
};

App.get app path::"/generate-state" @@ Middleware.fromAsync (fun req resp _ => {
    generateState ()
        |> then_ @@ fun state => {
            let success = Session.set req "napster" state;

            if(success) {
                resolve @@ Response.json resp (GenerateState.resp__to_json state);
            } else {
                return500 resp;
            };
        }

        |> catch @@ fun error => {
            Js.log error;
            return500 resp;
        };
});

module GetAccessTokens = {
    type req = {
        code: string,
        state: string
    };

    type resp = {
        accessToken: string,
        refreshToken: string
    };
};

type napsterApiAccessToken = {
    access_token: string,
    refresh_token: string,
    expires_in: int
};

let returnAccessTokens resp tokenBody => {
    switch tokenBody {
        | `Success body => {
            GetAccessTokens.{
                accessToken: body.access_token,
                refreshToken: body.refresh_token
            }
                |> GetAccessTokens.resp__to_json
                |> Response.json resp
                |> resolve;
        }
        | `Error error => { Js.log2 "Error:" error; return500 resp }
        | `NoBody => { Js.log "No Body"; return500 resp }
        | `NoResponse message => { Js.log2 "No response" message; return500 resp }
        | `UnknownError => { Js.log "Unknown error"; return500 resp }
        | `InvalidBody body => { Js.log2 "Invalid body" body; return500 resp }
    };
};

let requestAccessTokens resp code => {
    Superagent.post "https://api.napster.com/oauth/access_token"
        |> Superagent.Post.send
            {
                "client_id": config.napster.apiKey,
                "client_secret": config.napster.secret,
                "response_type": "code",
                "grant_type": "authorization_code",
                "code": code
            }
        |> Superagent.Post.end_
        |> then_ @@ Rest.parseResponse napsterApiAccessToken__from_json
        |> then_ @@ returnAccessTokens resp;
};

let compareState req resp code state => {
    switch (Session.get req "napster") {
        | None => return500 resp
        | Some actualState => {
            (actualState === state)
                ? requestAccessTokens resp code
                : return500 resp
        }
    };
};

App.post app path::"/get-access-tokens/" @@ Middleware.fromAsync (fun req resp _ => {
    let optAjaxReq = req
        |> Request.asJsonObject
        |> flip Js.Dict.get "body" |? Js.Json.null
        |> GetAccessTokens.req__from_json;

    switch optAjaxReq {
        | None => return500 resp
        | Some { code, state } => compareState req resp code state
    };
});

App.listen app onListen::(fun _ => Js.log "listening") ();
