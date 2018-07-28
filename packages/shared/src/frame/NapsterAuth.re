open Bluebird;
open ReDomSuite;
open FrameConfig;
open Belt.Result;

module BluebirdEx = PromiseEx.Make(Bluebird);

[@decco]
type queryString = {
    code: string,
    state: string
};

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
        |> BluebirdEx.map(doAuth);

let sendCode = (mophoCode) => {
    let window = Window.parent(ReDom.window);
    IFrameComm.post(IFrameComm.LoggedIn(mophoCode), "http://www.mopho.local/", window);
};

let getMophoCode = (code, state) => {
    let reqData = { Apis.NapsterAuth_impl.code, state };
    Apis.NapsterAuth.request(config.apiUrl, reqData)
        |> BluebirdEx.map(({ Apis.NapsterAuth_impl.mophoCode }) => sendCode(mophoCode));
};

let qs = ReDom.location
    |> Location.search
    |> Js.String.substr(~from=1)
    |> Qs.parse
    |> queryString_decode;

switch qs {
    | Ok({ code, state }) => getMophoCode(code, state)
    | _ => beginAuth()
}
|> catch((exn) => {
    Js.log2("Error in NapsterAuth: ", exn);
    resolve();
});
