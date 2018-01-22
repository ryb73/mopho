open FrameConfig;
open Bluebird;
open ReactStd;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

[@noserialize]
type state =
    | Initializing
    | LoggedOut
    | LoggedIn;

[@noserialize]
type action =
    | SetLoggedIn(bool);

let component = ReasonReact.reducerComponent("Root");

let getReqResult = [@bs.open] (fun
    | Superagent.ReqError(result) => result
);

/*
    Makes an arbitrary API call (Apis.GetMyUserData) and if it succeeds,
    it means the user is logged in. If we get a 401 error, they're
    logged out.
 */
let checkLoggedIn = ({ ReasonReact.reduce }) =>
    Apis.GetMyUserData.request(config.apiUrl, ())
        |> thenResolve(true)
        |> catch((exn) => {
            switch (getReqResult(exn)) {
                | Some({ statusCode: 401 }) => resolve(false)
                | _ => reject(Obj.magic(exn))
            };
        })
        |> map((loggedIn) => go(reduce, SetLoggedIn(loggedIn)))
        |> ignore;

let make = (_) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state, reduce } = self;

        (state === Initializing) ?
            checkLoggedIn(self)
        :
            ();

        let content =
            switch state {
                | Initializing => <span> (s2e("Initializing")) </span>
                | LoggedOut => <Login onLoggedIn=(reduce((_) => SetLoggedIn(true))) />
                | LoggedIn => <Foundation />
            };

        <div className="root"> content </div>
    },

    initialState: () => Initializing,

    reducer: (action, _) =>
        switch action {
            | SetLoggedIn(loggedIn) => ReasonReact.Update(loggedIn ? LoggedIn : LoggedOut)
        }
};
