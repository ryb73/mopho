open Js.Promise;
open PromiseEx;
open ReDomSuite;
open FrameConfig;

module QsParser = Qs.MakeParser({
    [@noserialize]
    type t = {.
        code: Js.undefined(string),
        state: Js.undefined(string)
    };
});

let napsterReady = make((~resolve, ~reject as _) => {
    Napster.init(config.napsterApiKey, "v2.2");
    Napster.on(Ready, (d) => {
        Js.log2("ready!", d);
        let u = ();
        [@bs] resolve(u);
    });
});

let getAuthUrl = (apiState) => {
    "https://api.napster.com/oauth/authorize?client_id="
        ++ config.napsterApiKey
        ++ "&redirect_uri="
        ++ Location.href(ReDom.location)
        ++ "&response_type=code"
        ++ "&state="
        ++ apiState;
};

let doAuth = (authState) => {
    ignore @@ Location.setHref(ReDom.location, getAuthUrl(authState));
};

let beginAuth = () =>
    Apis.GenerateState.request(config.apiUrl, ())
        |> map(doAuth);

let sendCode = (mophoCode) => {
    let window = Window.parent(ReDom.window);
    IFrameComm.post(IFrameComm.LoggedIn(mophoCode), "http://www.mopho.local/", window);
};

let getMophoCode = (code, state) => {
    let reqData = { Apis.NapsterAuth_impl.code, state };
    Apis.NapsterAuth.request(config.apiUrl, reqData)
        |> map(({ Apis.NapsterAuth_impl.mophoCode }) => sendCode(mophoCode));
};

let qs = ReDom.location
    |> Location.search
    |> Js.String.substr(~from=1)
    |> QsParser.parse;

let promise =
    switch (Js.Undefined.to_opt(qs##code), Js.Undefined.to_opt(qs##state)) {
        | (Some(code), Some(state)) => getMophoCode(code, state)
        | _ => beginAuth()
    };

promise
    |> catch((exn) => {
        Js.log2("Error in NapsterAuth: ", exn);
        resolve();
    });
