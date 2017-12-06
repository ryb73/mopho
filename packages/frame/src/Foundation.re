open FrameConfig;
open PromiseEx;
open Js.Promise;

let s2e = ReasonReact.stringToElement;

type state =
  | Initializing
  | Loaded Models.User.t
[@@noserialize];

type action =
  | SetUser Models.User.t
[@@noserialize];

let component = ReasonReact.reducerComponent "Foundation";

Napster.on Error (fun error => {
    Js.log2 "Error:" error;
});

let doInitialLoad () => {
    Apis.GetMyUserData.request config.apiUrl ()
        |> map @@ Js.log2 "user!"
        |> catch (fun exn => {
            Js.log2 "doInitialLoad Error" exn;
            resolve ();
        });
};

let make _ => {
    ...component,

    render: fun _ => {
        doInitialLoad ();

        <div className="foundation">
            <div className="top-bar">
                <input _type="text" placeholder="Search" />
            </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e "Leftbar")
                </div>

                <MainPane />
            </div>
            <div className="bottom-bar">
                (s2e "Bottom")
            </div>
        </div>
    },

    initialState: fun () => Initializing,

    reducer: fun action _ => {
        switch action {
            | SetUser user => ReasonReact.Update (Loaded user)
        };
    }
};
