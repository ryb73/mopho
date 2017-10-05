open Express;
open Node;
open Js.Promise;
open Option;

let flip = BatPervasives.flip;

let apiKey = "MDk5MmZmNDUtNTA1ZC00NmNiLWE4YTUtODNiNmVmNWVkMWZl";
let secret = "secret";
let secure = false;
let origin = "http://mopho.local";

let app = App.make ();

let corserOpts = Corser.opts origins::[| origin |] supportsCredentials::Js.true_ ();
App.use app @@ Corser.express corserOpts;

external bodyParserJson : unit => Middleware.t = "json" [@@bs.module "body-parser"];
App.use app @@ bodyParserJson ();

type session = string;
module Session = ExpressSession.Make({ type t = session; });

App.use app (ExpressSession.make @@ ExpressSession.opts
    ::secret resave::false saveUninitialized::false
    cookie::(ExpressSession.cookieOpts ::secure ()) ()
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

App.get app path::"/generate-state" @@ Middleware.fromAsync (fun req resp _ => {
    generateState ()
        |> then_ @@ fun state => {
            let success = Session.set req "napster" state;

            if(success) {
                resolve @@ Response.sendObject resp { "state": state };
            } else {
                return500 resp;
            };
        }

        |> catch @@ fun error => {
            Js.log error;
            return500 resp;
        };
});

let returnAccessTokens resp tokenBody => {
    switch tokenBody {
        | `Success body => {
            Js.log body;
            resolve @@ Response.sendString resp "ok";
        }
        | `Error error => { Js.log2 "Error:" error; return500 resp }
        | `NoBody => { Js.log "No Body"; return500 resp }
        | `NoResponse message => { Js.log2 "No response" message; return500 resp }
        | `UnknownError => { Js.log "Unknown error"; return500 resp }
        /* | _ => return500 resp; */
    };
};

let requestAccessTokens resp code => {
    Js.log "rat";
    Superagent.post "https://api.napster.com/oauth/access_token"
        |> Superagent.Post.send
            {
                "client_id": apiKey,
                "client_secret": secret,
                "response_type": "code",
                "grant_type": "authorization_code",
                "code": code
            }
        |> Superagent.Post.end_
        |> then_ Rest.parseResponse
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
    req
        |> Request.asJsonObject
        |> flip Js.Dict.get "body"
        |> flip bind Js.Json.decodeObject
        |> map (fun reqObj => {
            let code = reqObj |> flip Js.Dict.get "code" |> flip bind Js.Json.decodeString;
            let state = reqObj |> flip Js.Dict.get "state" |> flip bind Js.Json.decodeString;
            switch (code, state) {
                | (Some code, Some state) => compareState req resp code state
                | _ => return500 resp
            };
        })
        |? return500 resp;
});

App.get app path::"/read-state" @@ Middleware.from (fun req resp _ => {
    let optSessionData = Session.get req "napster";

    switch optSessionData {
        | None => Response.sendObject resp { "none": Js.true_ }
        | Some authState => Response.sendString resp authState
    };
});

App.listen app onListen::(fun _ => Js.log "listening") ();
