open Express;
open Node;
open Js.Promise;

let flip = BatPervasives.flip;
let config = ConfigLoader.config;

let app = App.make ();

let corserOpts = Corser.opts origins::[| config.origin |] supportsCredentials::Js.true_ ();
App.use app @@ Corser.express corserOpts;

/* external bodyParserJson : unit => Middleware.t = "json" [@@bs.module "body-parser"];
App.use app @@ bodyParserJson (); */

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
    |> Response.end_;

Apis.GenerateState.(
    handle app (fun req resp _ _ => {
        generateState ()
            |> then_ @@ fun state => {
                if(Session.set req "napster" state) {
                    resolve @@ Result state;
                } else {
                    resolve @@ ExpressAction (return500 resp);
                };
            }

            |> catch @@ fun error => {
                Js.log error;
                resolve @@ ExpressAction (return500 resp);
            };
    })
);

type napsterApiAccessToken = {
    access_token: string,
    refresh_token: string,
    expires_in: int
};

let () = {
    open Apis.GetAccessTokens;

    let returnAccessTokens resp tokenBody => {
        switch tokenBody {
            | `Success body =>
                resolve @@ Result {
                    accessToken: body.access_token,
                    refreshToken: body.refresh_token
                }
            | `Error error => resolve { Js.log2 "Error:" error; ExpressAction (return500 resp) }
            | `NoBody => resolve { Js.log "No Body"; ExpressAction (return500 resp) }
            | `NoResponse message => resolve { Js.log2 "No response" message; ExpressAction (return500 resp) }
            | `UnknownError => resolve { Js.log "Unknown error"; ExpressAction (return500 resp) }
            | `InvalidBody body => resolve { Js.log2 "Invalid body" body; ExpressAction (return500 resp) }
        };
    };

    let requestAccessTokens resp code => {
        let reqData = Js.Dict.fromList [
            ("client_id", config.napster.apiKey),
            ("client_secret", config.napster.secret),
            ("response_type", "code"),
            ("grant_type", "authorization_code"),
            ("code", code)
        ]
            |> Js.Dict.map ((fun s => Js.Json.string s) [@bs])
            |> Js.Json.object_;

        Superagent.post "https://api.napster.com/oauth/access_token"
            |> Superagent.Post.send reqData
            |> Superagent.Post.end_
            |> then_ @@ Rest.parseResponse napsterApiAccessToken__from_json
            |> then_ @@ returnAccessTokens resp;
    };

    let compareState req resp code state => {
        switch (Session.get req "napster") {
            | None => resolve @@ ExpressAction (return500 resp)
            | Some actualState => {
                (actualState === state)
                    ? requestAccessTokens resp code
                    : resolve @@ ExpressAction (return500 resp)
            }
        };
    };

    handle app (fun req resp _ body => {
        let { Apis.GetAccessTokens_impl.code, state } = body;
        compareState req resp code state;
    });
};

App.listen app onListen::(fun _ => Js.log "listening") ();
