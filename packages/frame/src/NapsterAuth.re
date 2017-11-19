open Js.Promise;
open PromiseEx;
open ReDomSuite;

module QsParser = Qs.MakeParser({
    type t = {.
        code: Js.undefined string,
        state: Js.undefined string
    } [@@noserialize];
});

let apiKey = "MDk5MmZmNDUtNTA1ZC00NmNiLWE4YTUtODNiNmVmNWVkMWZl";
let hostname = "www.mopho.local";
let apiUrl = "//api.mopho.local";

module NapsterApi = NapsterApi.Make({ let apiKey = apiKey; });

let napsterReady = make @@ fun ::resolve reject::_ => {
    Napster.init apiKey "v2.2";
    Napster.on Ready (fun d => {
        Js.log2 "ready!" d;
        let u = ();
        resolve u [@bs];
    });
};

let getAuthUrl apiState => {
    let redirectUri = Location.href ReDom.location;
    "https://api.napster.com/oauth/authorize?client_id=" ^ apiKey ^
        "&redirect_uri=" ^ redirectUri ^ "&response_type=code" ^
        "&state=" ^ apiState;
};

let doAuth authState => {
    Location.setHref ReDom.location @@ getAuthUrl authState;
    ();
};

let beginAuth () => {
    Apis.GenerateState.request apiUrl ()
        |> map (fun result => {
            switch result {
                | `Success state => doAuth state
                | _ => Js.log2 "Error:" result
            };
        });
};

let sendCode mophoCode => {
    let window = Window.parent ReDom.window;
    IFrameComm.post (IFrameComm.LoggedIn mophoCode) "http://www.mopho.local/" window;
};

let getMophoCode code state => {
    let reqData = { Apis.NapsterAuth_impl.code, state };
    Apis.NapsterAuth.request apiUrl reqData
        |> map (fun result => {
            switch result {
                | `Success { Apis.NapsterAuth_impl.mophoCode  } => sendCode mophoCode
                | _ => Js.log2 "Error:" result
            };
        });
};

let qs = ReDom.location
    |> Location.search
    |> Js.String.substr from::1
    |> QsParser.parse;

let promise = switch (Js.Undefined.to_opt qs##code, Js.Undefined.to_opt qs##state) {
    | (Some code, Some state) => getMophoCode code state
    | _ => beginAuth ()
};

promise
    |> catch (fun exn => {
        Js.log exn;
        resolve ();
    });