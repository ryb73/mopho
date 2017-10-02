open Express;
open Node;
open Js.Promise;

let apiKey = "MDk5MmZmNDUtNTA1ZC00NmNiLWE4YTUtODNiNmVmNWVkMWZl";
let secret = "secret";
let secure = false;
let origin = "http://mopho.local";

let app = App.make ();

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
                | `Buffer buffer => resolve (Buffer.toStringWithEncoding buffer encoding::"base64") [@bs]
            };
        });
    };
};

let return500 resp => Response.status resp 500
    |> Response.end_
    |> resolve;

generateState ();

App.get app path::"/generate-state" @@ Middleware.fromAsync (fun req resp _ => {
    Response.set resp "Access-Control-Allow-Origin" origin;

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

type gatReq = Js.undefined (Js.t {.
    state: Js.undefined string,
    code: Js.undefined string
});
external gatToReq : Js.Dict.t Js.Json.t => gatReq = "%identity";

let parseAccessTokenResponse req resp (err, res) => {
    switch res {
        | None => switch err {
            | None => `Error ("Unknown error in API request " ^ endpoint)
            | Some str => `Error str
        }
        | Some res => {
            switch (Falsy.to_opt res##error) {
                | Some error => Some error##message
            };
        }
    };
};

let requestAccessTokens req resp code => {
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
        |> then_ parseAccessTokenResponse req resp;
};

let compareState req resp code state => {
    let optSessionData = Session.get req "napster";

    switch optSessionData {
        | None => return500 resp
        | Some actualState => switch (actualState === state) {
            | false => return500 resp
            | true => requestAccessTokens req resp code
        };
    };
};

App.post app path::"/get-access-token/" @@ Middleware.from (fun req resp _ => {
    let undefReqObj = req |> Request.asJsonObject |> gatToReq;
    switch (Js.Undefined.to_opt undefReqObj) {
        | None => return500 resp
        | Some reqObj => {
            switch (Js.Undefined.to_opt reqObj##code, Js.Undefined.to_opt reqObj##state) {
                | (Some code, Some state) => compareState req resp code state
                | _ => return500 resp
            };
        }
    };

    Response.sendArray resp [||];
});

App.get app path::"/read-state" @@ Middleware.from (fun req resp _ => {
    let optSessionData = Session.get req "napster";

    switch optSessionData {
        | None => Response.sendObject resp { "none": Js.true_ }
        | Some authState => Response.sendString resp authState
    };
});

App.listen app onListen::(fun _ => Js.log "listening") ();
