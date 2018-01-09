open Std;
open Js.Promise;
open PromiseEx;
open MomentRe;
open Middleware;

module App = Express.App;
module Response = Express.Response;

let flip = BatPervasives.flip;
let config = ConfigLoader.config;

module NapsterApi = NapsterApi.Make({ let apiKey = config.napster.apiKey; });

let app = App.make();

let corserOpts = Corser.opts(~origins=[|config.origin|], ~supportsCredentials=Js.true_, ());
App.use(app) @@ Corser.express(corserOpts);
App.use(app) @@ BodyParser.json(~strict=false, ());

[@bs.module] external cookieParser : unit => Express.Middleware.t = "cookie-parser";
App.use(app) @@ cookieParser();

type session = {
    state: option(string),
    napsterAccessToken: option(string)
};

module Session = ExpressSession.Make({
    type t = session;
    let key = "mopho-api-server";
});

App.use(app, ExpressSession.make(ExpressSession.opts(
    ~secret=config.sessionSecret,
    ~resave=false,
    ~saveUninitialized=false,
    ~cookie=ExpressSession.cookieOpts(~secure=config.secure, ()),
    ()
)));

let getSession = (req) =>
    switch (Session.get(req)) {
        | Some(session) => session
        | None => { state: None, napsterAccessToken: None }
    };

Apis.GenerateState.(
    handle(app, (req, _, _, _) =>
        Std.generateRandomBase64()
            |> then_((state) => {
                let curSess = getSession(req);
                if (Session.set(req, { ...curSess, state: Some(state) })) {
                    resolve(Result(state));
                } else {
                    Js.log("Error setting session");
                    resolve(ErrorCode(500));
                };
            })
            |> catch((error) => {
                Js.log(error);
                resolve(ErrorCode(500));
            })
    )
);

let getUserIdFromNapsterMember = ({ NapsterApi.id, realName }) =>
    Db.User.getFromNapsterId(id)
        |> then_((userId) =>
              switch userId {
                  | None => {
                      Db.User.create(realName)
                          |> tap(flip(Db.User.setNapsterId, id))
                  }

                  | Some(userId) => resolve(userId)
              }
          );

let saveNapsterTokens = (req, accessToken, refreshToken, userId) =>
    Db.User.setNapsterRefreshToken(userId, refreshToken)
        |> map(() => {
            let session = getSession(req);
            if (Session.set(req, { ...session, napsterAccessToken: Some(accessToken) })) {
                userId;
            } else {
                Js.Exn.raiseError("Error saving access token");
            };
        });

type napsterApiAccessToken = {
    access_token: string,
    refresh_token: string,
    expires_in: int
};

let () = {
    open Apis.NapsterAuth;

    let loginWithToken = (req, body) => {
        Js.log2("ip:", getIp(req));
        NapsterApi.me(body.access_token)
            |> map(({ NapsterApi.me }) => me)
            |> then_(getUserIdFromNapsterMember)
            |> then_(saveNapsterTokens(req, body.access_token, body.refresh_token))
            |> then_(Db.User.generateAuthCode(getIp(req)));
    };

    let returnAuthCode = (authCode) => Result({mophoCode: authCode });

    let requestAccessTokens = (req, code) => {
        let reqData =
            Js.Dict.fromList([
                ("client_id", config.napster.apiKey),
                ("client_secret", config.napster.secret),
                ("response_type", "code"),
                ("grant_type", "authorization_code"),
                ("code", code)
            ])
            |> Js.Dict.map([@bs] ((s) => Js.Json.string(s)))
            |> Js.Json.object_;

        Superagent.post("https://api.napster.com/oauth/access_token")
            |> Superagent.Post.send(reqData)
            |> Superagent.Post.end_
            |> map(RespParser.parse(napsterApiAccessToken__from_json))
            |> unwrapResult
            |> then_(loginWithToken(req))
            |> map(returnAuthCode);
    };

    handle(app, (req, _, _, { Apis.NapsterAuth_impl.code, state }) => {
        let session = getSession(req);
        switch session.state {
            | None => resolve(ErrorCode(400))

            | Some(actualState) =>
                actualState === state ? requestAccessTokens(req, code) : resolve(ErrorCode(400))
        };
    });
};

Apis.LogInWithCode.handle(app, (req, resp, _, code) =>
    Db.User.useCode(getIp(req), code)
        |> map((token) => {
            let expires = momentUtc()
                |> Moment.add(~duration=duration(55, `years))
                |> MomentRe.Moment.toDate;

            let opts = Response.cookieOpts(
                ~expires,
                ~httpOnly=true,
                ~secure=config.secure,
                ~sameSite="lax", ()
            );

            Response.setCookie(resp, Priveleged.authTokenCookie, token, ~opts, ());
            Apis.LogInWithCode.Result();
        })
);

module GetMyUserData = Priveleged.Make(Apis.GetMyUserData);

GetMyUserData.handle(app, (_, _, _, _, user) => GetMyUserData.Result(user));

module LogOut = Priveleged.Make(Apis.LogOut);

LogOut.handle(app, (_, resp, _, _, _) => {
    Response.clearCookie(resp, Priveleged.authTokenCookie);
    LogOut.Result();
});

App.listen(app, ~onListen=(_) => Js.log("listening"), ());
