open Js.Promise;
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
        |> then_ (fun result => {
            switch result {
                | `Error error => {
                    Js.log error##message;
                }

                | `NoResponse message => Js.log message
                | `Success state => doAuth state
                | `InvalidBody body => Js.log2 "invalid body" body
                | `NoBody => Js.log "No body"
                | `UnknownError => Js.log "unknown error"
            };

            resolve ();
        })
        |> catch (fun exn => {
            Js.log exn;
            resolve ();
        });

    ();
};

let loginSuccess _ => {
    let window = Window.parent ReDom.window;
    IFrameComm.post IFrameComm.LoggedIn "http://www.mopho.local/" window;
    resolve ();
};

let setTokens { Apis.GetAccessTokens_impl.accessToken, refreshToken  } => {
    Js.log3 "tokens: " accessToken refreshToken;

    napsterReady
        |> then_ (fun () => {
            Napster.setAuth {
                "accessToken": accessToken,
                "refreshToken": refreshToken
            };

            /* Napster.load (); */

            NapsterApi.me accessToken;
        })
        |> then_ loginSuccess
        |> catch (fun err => {
            Js.log2 "Error" (Js.String.make err);
            resolve ();
        });

    ();
};

let getTokens code state => {
    let reqData = { Apis.GetAccessTokens_impl.code, state };
    Apis.GetAccessTokens.request apiUrl reqData
        |> then_ (fun result => {
            switch result {
                | `Error error => {
                    Js.log error##message;
                }

                | `NoResponse message => Js.log message
                | `Success tokens => setTokens tokens
                | _ => Js.log "An unknown error occurred"
            };

            resolve ();
        });

    ();
};

let qs = ReDom.location
    |> Location.search
    |> Js.String.substr from::1
    |> QsParser.parse;

switch (Js.Undefined.to_opt qs##code, Js.Undefined.to_opt qs##state) {
    | (Some code, Some state) => getTokens code state
    | _ => beginAuth ()
};
