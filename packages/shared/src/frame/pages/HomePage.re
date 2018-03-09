open FrameConfig;
open ReactStd;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

type dynamicProps = unit;
type retainedProps = ReasonReact.noRetainedProps;
type state = option(Apis.Search_impl.resp);
type action =
    | SetSearchResults(Apis.Search_impl.resp);

let logOut = (_) => {
    Apis.LogOut.request(config.apiUrl, ())
        |> map(() => Js.log("logged out"))
        |> catch((exn) => {
            Js.log2("logout error", exn);
            resolve()
        });

    ();
};

let component = ReasonReact.reducerComponent("HomePage");

let make = (~dynamicProps as (), ~context, _) => {
    ...component,

    render: (self) => {
        let { ReasonReact.handle } = self;

        <div>
            <button onClick=(logOut)>(s2e("Log Out"))</button>
        </div>;
    },

    initialState: () : state => None,

    reducer: (action, _) => {
        switch action {
            | SetSearchResults(results) => ReasonReact.Update(Some(results))
        };
    }
};
