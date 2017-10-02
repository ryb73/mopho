open Js.Promise;

module QsParser = Qs.MakeParser({
    type t = {.
        code: Js.undefined string,
        state: Js.undefined string
    };
});

let apiKey = "MDk5MmZmNDUtNTA1ZC00NmNiLWE4YTUtODNiNmVmNWVkMWZl";
let hostname = "mopho.local";
let apiUrl = "//api.mopho.local";

let getAuthUrl apiState => {
    open Webapi.Dom;

    let redirectUri = Location.href location;
    "https://api.napster.com/oauth/authorize?client_id=" ^ apiKey ^
        "&redirect_uri=" ^ redirectUri ^ "&response_type=code" ^
        "&state=" ^ apiState;
};

let doGet endpoint => {
    Superagent.get @@ apiUrl ^ endpoint
    |> Superagent.Get.end_
    |> then_ @@ parseResponse;
};

let doPost endpoint data => {
    Superagent.post @@ apiUrl ^ endpoint
    |> Superagent.Post.send data
    |> Superagent.Post.end_
    |> then_ @@ parseResponse;
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
                | `Success body => doAuth body##state
                | _ => Js.log "An unknown error occurred"
            };

            resolve ();
        })
        |> catch (fun exn => {
            Js.log exn;
            resolve ();
        });

    ();
};

let getTokens code state => {
    doPost "/get-access-tokens/" {
        "code": code,
        "state": state
    };
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