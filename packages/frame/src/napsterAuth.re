open Js.Promise;


module QsParser = Qs.MakeParser({
    type t = {.
        code: Js.undefined string
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

let doRequest endpoint => {
    Superagent.get @@ apiUrl ^ endpoint
    |> Superagent.Get.end_
    |> then_ (fun (err, res) => {
        resolve @@ switch res {
            | None => switch err {
                | None => `Error ("Unknown error in API request " ^ endpoint)
                | Some str => `Error str
            }

            | Some res => {
                switch (Falsy.to_opt res##error) {
                    | Some error => `Error error##message
                    | None => {
                        switch (Js.Null.to_opt res##body) {
                            | None => `Error ("No response body for API request " ^ endpoint)
                            | Some body => `Success body
                        };
                    }
                };
            }
        };
    });
};

let doAuth authState => {
    open Webapi.Dom;
    Location.setHref location @@ getAuthUrl authState;
    ();
};

let beginAuth () => {
    doRequest "/generate-state/"
        |> then_ (fun result => {
            switch result {
                | `Error message => {
                    Js.log message;
                }

                | `Success body => doAuth body##state
            };

            resolve ();
        })
        |> catch (fun exn => {
            Js.log exn;
            resolve ();
        });

    ();
};

let returnCode code => Js.log code;

let qs = Webapi.Dom.location
    |> Webapi.Dom.Location.search
    |> Js.String.substr from::1
    |> QsParser.parse;

switch (Js.Undefined.to_opt qs##code) {
    | Some code => returnCode code
    | None => beginAuth ()
};