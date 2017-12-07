open FrameConfig;
open PromiseEx;
open Js.Promise;

let s2e = ReasonReact.stringToElement;

module type Component = {
    let make : 'a => ReasonReact.componentSpec ReasonReact.stateless ReasonReact.stateless ReasonReact.noRetainedProps ReasonReact.noRetainedProps ReasonReact.actionless;
};

type state =
  | Initializing
  | Loaded Models.User.t
[@@noserialize];

type action =
  | SetUser Models.User.t
[@@noserialize];

let a : (module Component) = (module MainPane);

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
    ();
};

let logOut _ => {
    Apis.LogOut.request config.apiUrl ()
        |> map (fun () => Js.log "logged out")
        |> catch (fun exn => {
            Js.log2 "logout error" exn;
            resolve ();
        });
    ();
};

let getUser _ => doInitialLoad ();

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
                    <a href="#" onClick={logOut}>(s2e "Log Out")</a>
                    <a href="#" onClick={getUser}>(s2e "Get User")</a>
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
