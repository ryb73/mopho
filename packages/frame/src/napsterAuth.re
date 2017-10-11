open Js.Promise;

module QsParser = Qs.MakeParser({
    type t = {.
        code: Js.undefined string,
        state: Js.undefined string
    } [@@noserialize];
});

let apiKey = "MDk5MmZmNDUtNTA1ZC00NmNiLWE4YTUtODNiNmVmNWVkMWZl";
let hostname = "mopho.local";
let apiUrl = "//api.mopho.local";

module GetAccessTokens = {
    type req = {
        code: string,
        state: string
    };

    type resp = {
        accessToken: string,
        refreshToken: string
    };
};

let getAuthUrl apiState => {
    open Webapi.Dom;

    let redirectUri = Location.href location;
    "https://api.napster.com/oauth/authorize?client_id=" ^ apiKey ^
        "&redirect_uri=" ^ redirectUri ^ "&response_type=code" ^
        "&state=" ^ apiState;
};

let doGet endpoint => {
    Superagent.get @@ apiUrl ^ endpoint
    |> Superagent.Get.withCredentials
    |> Superagent.Get.end_
    |> then_ @@ Rest.parseResponse Js.Json.decodeString;
};

let doPost endpoint data => {
    Superagent.post @@ apiUrl ^ endpoint
    |> Superagent.Post.withCredentials
    |> Superagent.Post.send data
    |> Superagent.Post.end_
    |> then_ @@ Rest.parseResponse GetAccessTokens.resp__from_json;
};

let doAuth authState => {
    open Webapi.Dom;
    Location.setHref location @@ getAuthUrl authState;
    ();
};

let beginAuth () => {
    doGet "/generate-state/"
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

let setTokens { GetAccessTokens.accessToken, refreshToken } => {
    Js.log3 "tokens: " accessToken refreshToken;
};

let getTokens code state => {
    doPost "/get-access-tokens/" {
        "code": code,
        "state": state
    }
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