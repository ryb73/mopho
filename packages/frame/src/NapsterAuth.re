open Js.Promise;
open Js.Result;
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

let napsterReady = make @@ fun ::resolve reject::_ => {
    Napster.init apiKey "v2.2";
    Napster.on Ready (fun d => {
        Js.log2 "ready!" d;
        let u = ();
        resolve u [@bs];
    });
};

let getAuthUrl apiState => {
    open Webapi.Dom;

    let redirectUri = Location.href location;
    "https://api.napster.com/oauth/authorize?client_id=" ^ apiKey ^
        "&redirect_uri=" ^ redirectUri ^ "&response_type=code" ^
        "&state=" ^ apiState;
};

let doAuth authState => {
    open Webapi.Dom;
    Location.setHref location @@ getAuthUrl authState;
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

let loginSuccess () => {
    let window = Window.parent ReDom.window;
    IFrameComm.post IFrameComm.LoggedIn "http://www.mopho.local/" window;
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

            Napster.Api.me ();
        })
        |> then_ (fun result => {
            switch result {
                | Ok _ => loginSuccess ();
                | Error err => Js.log2 "Error" err
            };

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

let qs = Webapi.Dom.location
    |> Webapi.Dom.Location.search
    |> Js.String.substr from::1
    |> QsParser.parse;

switch (Js.Undefined.to_opt qs##code, Js.Undefined.to_opt qs##state) {
    | (Some code, Some state) => getTokens code state
    | _ => beginAuth ()
};