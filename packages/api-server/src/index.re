open Express;
open Js.Promise;
open PromiseEx;

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
    handle app (fun req _ _ _ => {
        Std.generateRandomBase64 ()
            |> then_ @@ fun state => {
                let curSess = getSession req;

                if(Session.set req { ...curSess, state: Some state }) {
                    resolve @@ Result state;
                } else {
                    Js.log "Error setting session";
                    resolve @@ ErrorCode 500;
                };
            }
            |> catch @@ fun error => {
                Js.log error;
                resolve @@ ErrorCode 500;
            };
    })
);

let getUserIdFromNapsterMember { NapsterApi.id, realName } => {
    Db.User.getFromNapsterId id
        |> then_ (fun userId => {
            switch userId {
                | None => Db.User.create realName
                    |> tap @@ flip Db.User.setNapsterId id

                | Some userId => resolve userId
            };
        });
};

let saveNapsterTokens req accessToken refreshToken userId => {
    Db.User.setNapsterRefreshToken userId refreshToken
        |> map (fun () => {
            let session = getSession req;
            if(Session.set req { ...session, napsterAccessToken: Some accessToken }) {
                userId;
            } else {
                failwith "Error saving access token";
            };
        });
};

type napsterApiAccessToken = {
    access_token: string,
    refresh_token: string,
    expires_in: int
};

let () = {
    open Apis.NapsterAuth;

    let loginWithToken req tokenBody => {
        Js.log2 "ip:" (Request.ip req);

        switch tokenBody {
            | `Success body =>
                NapsterApi.me body.access_token
                    |> map (fun {  NapsterApi.me } => me)
                    |> then_ getUserIdFromNapsterMember
                    |> then_ @@ saveNapsterTokens req body.access_token body.refresh_token
                    |> then_ @@ Db.User.generateAuthCode (Request.ip req);
            | _ => {
                Js.log tokenBody;
                failwith "Couldn't get access token";
            }
        };
    };

    let returnAuthCode authCode => {
        Result { mophoCode: authCode };
    };

    let requestAccessTokens req code => {
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
            |> map @@ returnAuthCode;
    };

    handle app (fun req _ _ { Apis.NapsterAuth_impl.code, state } => {
        let session = getSession req;
        switch session.state {
            | None => resolve @@ ErrorCode 400
            | Some actualState => {
                (actualState === state)
                    ? requestAccessTokens req code
                    : resolve @@ ErrorCode 400
            }
        };
    });
};

App.listen app onListen::(fun _ => Js.log "listening") ();
