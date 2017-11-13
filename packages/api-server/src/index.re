open Express;
open Node;
open Js.Promise;

let flip = BatPervasives.flip;
let config = ConfigLoader.config;

module NapsterApi = NapsterApi.Make({ let apiKey = config.napster.apiKey });

let app = App.make ();

let corserOpts = Corser.opts origins::[| config.origin |] supportsCredentials::Js.true_ ();
App.use app @@ Corser.express corserOpts;

/* external bodyParserJson : unit => Middleware.t = "json" [@@bs.module "body-parser"];
App.use app @@ bodyParserJson (); */

type session = {
    state: option string,
    napsterAccessToken: option string
};

module Session = ExpressSession.Make({
    type t = session;
    let key = "mopho-api-server";
});

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

let getSession req => {
    switch (Session.get req) {
        | Some session => session
        | None => {
            Js.log "No session";
            { state: None, napsterAccessToken: None }
        };
    };
};

Apis.GenerateState.(
    handle app (fun req resp _ _ => {
        generateState ()
            |> then_ @@ fun state => {
                let curSess = getSession req;

                if(Session.set req { ...curSess, state: Some state }) {
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

let getUserIdFromNapsterMember { NapsterApi.id, realName } => {
    Db.User.getFromNapsterId id
        |> then_ (fun userId => {
            switch userId {
                | None => Db.User.create realName
                | Some userId => resolve userId
            };
        });
};

let saveNapsterTokens req accessToken refreshToken userId => {
    Db.User.saveNapsterRefreshToken userId refreshToken;

    let session = getSession req;
    if(Session.set req { ...session, napsterAccessToken: Some accessToken }) {
        resolve userId;
    } else {
        failwith "Error saving access token";
    }
};

let setLoggedInAs _userId => failwith "TODO";

type napsterApiAccessToken = {
    access_token: string,
    refresh_token: string,
    expires_in: int
};

let () = {
    open Apis.GetAccessTokens;

    let loginWithToken req tokenBody => {
        switch tokenBody {
            | `Success body =>
                NapsterApi.me body.access_token
                    |> then_ (fun {  NapsterApi.me } => resolve me)
                    |> then_ getUserIdFromNapsterMember
                    |> then_ @@ saveNapsterTokens req body.access_token body.refresh_token
                    |> then_ setLoggedInAs;

            | _ => {
                Js.log tokenBody;
                failwith "Couldn't get access token";
            }
        };
    };

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

    let requestAccessTokens req resp code => {
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
            |> then_ @@ loginWithToken req
            |> then_ @@ returnAccessTokens resp;
    };

    let compareState req resp code state => {
        let session = getSession req;
        switch session.state {
            | None => resolve @@ ExpressAction (return500 resp)
            | Some actualState => {
                (actualState === state)
                    ? requestAccessTokens req resp code
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
