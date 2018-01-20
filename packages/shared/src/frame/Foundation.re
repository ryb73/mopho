open FrameConfig;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let s2e = ReasonReact.stringToElement;

module type Component = {
    let make: 'a => ReasonReact.componentSpec(
        ReasonReact.stateless,
        ReasonReact.stateless,
        ReasonReact.noRetainedProps,
        ReasonReact.noRetainedProps,
        ReasonReact.actionless
    );
};

[@noserialize]
type state =
    | Initializing
    | Loaded(Models.User.t);

[@noserialize]
type action =
    | SetUser(Models.User.t);

let a: (module Component) = (module MainPane);

let component = ReasonReact.reducerComponent("Foundation");

NapsterPlayer.on(Error, (error) => Js.log2("Error:", error));

let doInitialLoad = () => {
    Apis.GetMyUserData.request(config.apiUrl, ())
        |> map(Js.log2("user!"))
        |> catch((exn) => {
            Js.log2("doInitialLoad Error", exn);
            resolve()
        });

    ();
};

let logOut = (_) => {
    Apis.LogOut.request(config.apiUrl, ())
        |> map(() => Js.log("logged out"))
        |> catch((exn) => {
            Js.log2("logout error", exn);
            resolve()
        });

    ();
};

let getUser = (_) => doInitialLoad();

let testSearch = (_) => {
    Apis.Search.request(config.apiUrl, "mitski")
        |> map(Js.log2("search results"))
        |> catch((exn) => {
            Js.log2("search error", exn);
            resolve();
        })
        |> ignore;
};

let make = (_) => {
    ...component,

    render: (_) => {
        doInitialLoad();
        <div className="foundation">
            <div className="top-bar"> <input _type="text" placeholder="Search" /> </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e("Leftbar"))
                    <a href="#" onClick=logOut> (s2e("Log Out")) </a>
                    <a href="#" onClick=getUser> (s2e("Get User")) </a>
                    <p>
                        <button onClick=testSearch>(s2e("Search"))</button>
                    </p>
                </div>
                <MainPane />
            </div>
            <div className="bottom-bar"> (s2e("Bottom")) </div>
        </div>
    },

    initialState: () => Initializing,

    reducer: (action, _) =>
        switch action {
            | SetUser(user) => ReasonReact.Update(Loaded(user))
        }
};
