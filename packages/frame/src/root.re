open Js.Promise;

type state = {
    authState: option string,
    error: bool
};

type action =
  | SetAuthState string
  | Error;

let apiUrl = "//api.mopho.local";
let apiKey = "API_KEY";

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

let component = ReasonReact.reducerComponent "Page";

let make _ => {
    let s2e = ReasonReact.stringToElement;

    let getAuthUrl apiState => {
        "https://api.napster.com/oauth/authorize?client_id=" ^ apiKey ^
            "&redirect_uri=http://" ^ "yourdomain.com" ^ "/&response_type=code" ^
            "&state=" ^ apiState;
    };

    let renderError () => {
        <div>
            (s2e "An error occurred")
        </div>;
    };

    let renderAuth state => {
        switch state.authState {
            | None => <div>(s2e "Loading")</div>;
            | Some authState => {
                <div>
                    (s2e @@ getAuthUrl authState)
                </div>;
            }
        };
    };

    let go reduce action => reduce (fun _ => action) ();

    {
        ...component,

        render: fun { state } => {
            switch state.error {
                | true => renderError ()
                | false => renderAuth state
            };
        },

        initialState: fun () => {
            authState: None,
            error: false
        },

        reducer: fun action state => {
            switch action {
                | SetAuthState authState =>
                    ReasonReact.Update { ...state, authState: Some authState }
                | Error =>
                    ReasonReact.Update { ...state, error: true }
            };
        },

        didMount: fun { reduce } => {
            doRequest "/generate-state/"
                |> then_ (fun result => {
                    switch result {
                        | `Error message => {
                            Js.log message;
                            go reduce @@ Error;
                        }

                        | `Success body => go reduce @@ SetAuthState body##state
                    };

                    resolve ();
                })
                |> catch (fun exn => {
                    Js.log exn;
                    go reduce Error;
                    resolve ();
                });

            ReasonReact.NoUpdate;
        }
    }
};
