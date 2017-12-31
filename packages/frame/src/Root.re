open FrameConfig;
open PromiseEx;
open Js.Promise;

let s2e = ReasonReact.stringToElement;

[@noserialize]
type state =
    | Initializing
    | LoggedOut
    | LoggedIn;

[@noserialize]
type action =
    | SetLoggedIn
    | SetLoggedOut;

let component = ReasonReact.reducerComponent("Root");

let go = (reduce, action) => reduce((_) => action, ());

let checkLoggedIn = ({ ReasonReact.reduce }) =>
    Apis.GetMyUserData.request(config.apiUrl, ())
        |> map((_) => go(reduce, SetLoggedIn))
        |> thenResolve()
        |> catch((exn) => {
            Js.log2("doInitialLoad Error", exn);
            resolve()
        });

let make = (_) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state, reduce } = self;

        /* checkLoggedIn self; */

        let content =
            switch state {
                | Initializing => <span> (s2e("Initializing")) </span>
                | LoggedOut => <Login onLoggedIn=(reduce((_) => SetLoggedIn)) />
                | LoggedIn => <Foundation />
            };

        <div className="root"> content </div>
    },

    initialState: () => LoggedOut, /* Initializing */

    reducer: (action, _) =>
        switch action {
            | SetLoggedIn => ReasonReact.Update(LoggedIn)
            | SetLoggedOut => ReasonReact.Update(LoggedOut)
        }
};
