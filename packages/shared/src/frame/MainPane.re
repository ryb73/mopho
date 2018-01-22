open ReactStd;

type state = (module Component);

type action =
  | SetPage((module Component));

let component = ReasonReact.reducerComponent("MainPane");

let make = (~context, _) => {
    ...component,

    render: ({ state }) => {
        module Page = (val state : Component);

        <div className="main-pane">
            <Page context />
        </div>
    },

    initialState: () : state => (module HomePage),

    reducer: (action, _) =>
        switch action {
            | SetPage(page) => ReasonReact.Update(page)
        }
};
