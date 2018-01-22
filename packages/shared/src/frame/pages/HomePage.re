open FrameConfig;
open ReactStd;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

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

let testSearch = ({ Context.navigate }, _, { ReasonReact.reduce }) => {
    module PageChange = {
        type dynamicProps = SearchPage.dynamicProps;
        type context = Context.t;

        module type Component = {
            type retainedProps;
            type state;
            type action;
            let make: (~dynamicProps: dynamicProps, ~context: context, array(unit))
                => ReasonReact.component(state, retainedProps, action);
        };

        let component = (module SearchPage : Component);
        let dynamicProps = "mitski";
    };

    navigate((module PageChange));

    /* Apis.Search.request(config.apiUrl, "mitski")
        |> map(results => go(reduce, SetSearchResults(results)))
        |> catch((exn) => {
            Js.log2("search error", exn);
            resolve();
        })
        |> ignore; */
};

let component = ReasonReact.reducerComponent("HomePage");

let make = (~context, _) => {
    ...component,

    render: (self) => {
        let { ReasonReact.handle } = self;

        <div>
            <button onClick=(logOut)>(s2e("Log Out"))</button>
            <button onClick=(handle(testSearch(context)))>(s2e("Search"))</button>
        </div>;
    },

    initialState: () : state => None,

    reducer: (action, _) => {
        switch action {
            | SetSearchResults(results) => ReasonReact.Update(Some(results))
        };
    }
};
