let s2e =  ReasonReact.stringToElement;

type state =
    | Initializing
    | LoggedOut
    | LoggedIn
[@@noserialize];

type action =
  | SetLoggedIn
[@@noserialize];

let component = ReasonReact.reducerComponent "Root";

let make _ => {
    ...component,

    render: fun self => {
        let { ReasonReact.state, reduce } = self;

        let content = switch state {
            | Initializing => <span>(s2e "Initializing")</span>
            | LoggedOut => <Login onLoggedIn={reduce (fun _ => SetLoggedIn)} />
            | LoggedIn => <Foundation />
        };

        <div className="root">(content)</div>
    },

    initialState: fun () => LoggedOut,

    reducer: fun action _ => {
        switch action {
            | SetLoggedIn => ReasonReact.Update LoggedIn
        };
    }
};
