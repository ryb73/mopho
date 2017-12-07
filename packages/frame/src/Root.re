open FrameConfig;
open PromiseEx;
open Js.Promise;

let s2e =  ReasonReact.stringToElement;

type state =
    | Initializing
    | LoggedOut
    | LoggedIn
[@@noserialize];

type action =
  | SetLoggedIn
  | SetLoggedOut
[@@noserialize];

let component = ReasonReact.reducerComponent "Root";

let go reduce action => reduce (fun _ => action) ();

let checkLoggedIn { ReasonReact.reduce } => Apis.GetMyUserData.request config.apiUrl ()
    |> map (fun _ => go reduce SetLoggedIn)
    |> thenResolve ()
    |> catch (fun exn => {
        Js.log2 "doInitialLoad Error" exn;
        resolve ();
    });

let make _ => {
    ...component,

    render: fun self => {
        let { ReasonReact.state, reduce } = self;

        /* checkLoggedIn self; */

        let content = switch state {
            | Initializing => <span>(s2e "Initializing")</span>
            | LoggedOut => <Login onLoggedIn={reduce (fun _ => SetLoggedIn)} />
            | LoggedIn => <Foundation />
        };

        <div className="root">(content)</div>
    },

    initialState: fun () => LoggedOut /* Initializing */,

    reducer: fun action _ => {
        switch action {
            | SetLoggedIn => ReasonReact.Update LoggedIn
            | SetLoggedOut => ReasonReact.Update LoggedOut
        };
    }
};
