open Std;
open Bluebird;
open MomentRe;
open Option.Infix;
open! Middleware; /* Ignore warnings because of ppx_autoserialize */

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let (%>) = BatPervasives.(%>);

module App = Express.App;
module Response = Express.Response;

let flip = BatPervasives.flip;
let config = ConfigLoader.config;

module NapsterApi = BsNapsterApi.Api.Make({ let apiKey = config.napster.apiKey; });

let app = App.make();

let corserOpts = Corser.opts(~origins=[|config.origin|], ~supportsCredentials=Js.true_, ());
App.use(app) @@ Corser.express(corserOpts);
App.use(app) @@ BodyParser.json(~strict=false, ());

[@bs.module] external cookieParser : unit => Express.Middleware.t = "cookie-parser";
App.use(app) @@ cookieParser();

[@autoserialize]
type session = {
    state: option(string),
    napsterAccessToken: option(string)
};

module Session = ExpressSession.Make({
    [@autoserialize] type t = session;
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
    )
);

let getUserIdFromNapsterMember = ({ BsNapsterApi.Types.Member.id, realName }) =>
    DbUser.getFromNapsterId(id)
        |> then_((userId) =>
              switch userId {
                  | None => {
                      DbUser.create(realName)
                          |> tap(flip(DbUser.setNapsterId, id))
                  }

                  | Some(userId) => resolve(userId)
              }
          );

let setSessionNapsterToken = (req, accessToken) => {
    let session = getSession(req);
    if (Session.set(req, { ...session, napsterAccessToken: Some(accessToken) })) {
        ();
    } else {
        Js.Exn.raiseError("Error saving access token");
    };
};

let () = {
    open Apis.NapsterAuth;

    let loginWithToken = (req, body) => {
        Js.log2("ip:", getIp(req));
        NapsterApi.me(body.EAuthNapster.access_token)
            |> tap(Js.log2("1"))
            |> map(({ NapsterApi.me }) => me)
            |> tap(Js.log2("2"))
            |> then_(getUserIdFromNapsterMember)
            |> tap(Js.log2("3"))
            |> tap(flip(DbUser.setNapsterRefreshToken, body.refresh_token))
            |> tap(Js.log2("4"))
            |> tap((_) => setSessionNapsterToken(req, body.access_token))
            |> tap(Js.log2("5"))
            |> then_(DbUser.generateAuthCode(getIp(req)))
            |> tap(Js.log2("6"))
    };

    let returnAuthCode = (authCode) => Result({mophoCode: authCode });

    let requestAccessTokens = (req, code) => {
        EAuthNapster.getTokensFromCode(code)
            |> then_(loginWithToken(req))
            |> map(returnAuthCode);
    };

    handle(app, (req, _, _, { Apis.NapsterAuth_impl.code, state }) => {
        let session = getSession(req);
        switch session.state {
            | None => resolve(ErrorCode(400))

            | Some(actualState) =>
                actualState === state ?
                    requestAccessTokens(req, code)
                :
                    resolve(ErrorCode(400))
        };
    });
};

Apis.LogInWithCode.handle(app, (req, resp, _, code) =>
    DbUser.useCode(getIp(req), code)
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

GetMyUserData.handle(app, (_, _, _, _, user) =>
    resolve(GetMyUserData.Result(user))
);

module LogOut = Priveleged.Make(Apis.LogOut);

LogOut.handle(app, (_, resp, _, _, _) => {
    Response.clearCookie(resp, Priveleged.authTokenCookie);
    resolve(LogOut.Result());
});

let getNapsterAccessToken = (req, userId) => {
    let token = getSession(req).napsterAccessToken;
    (Option.is_some(token)) ?
        resolve(token)
    :
        EAuthNapster.refreshAccessToken(userId)
            |> mapMaybe(({ EAuthNapster.access_token }) => access_token);
};

let runMatcher = (matcher, results) =>
    results
        |> Js.Array.map(matcher)
        |> all
        |> map(Js.Array.filter(Option.is_some))
        |> map(Js.Array.map(Option.get));

module Search = Priveleged.Make(Apis.Search);
Search.handle(app, (req, _, _, query, user) => {
    getNapsterAccessToken(req, user.id)
        |> tapMaybe(setSessionNapsterToken(req) %> resolve)
        |> thenMaybe((accessToken) => {
            flip(
                NapsterApi.search(~types=[| BsNapsterApi.Types.Search.Artists, Albums, Tracks |]),
                query
            )(accessToken)
        })
        |> thenMaybe(({ BsNapsterApi.Types.Search.search: { data }}) => {
            all3
                ((
                    data.artists |? [||]
                        |> runMatcher(NapsterSource.matchArtist),

                    data.albums |? [||]
                        |> runMatcher(NapsterSource.matchAlbum),

                    resolve([||])
                ))
                |> map(((artists, albums, tracks)) => Search.Result({ artists, albums, tracks }));
        })
        |> map(fun
            | Some(v) => v
            | None => Search.ErrorCode(500)
        );
});

App.listen(app, ~onListen=(_) => Js.log("listening"), ());
