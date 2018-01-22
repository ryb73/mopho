open ReactStd;

type state = (module Component);

type action =
  | SetPage((module Component));

let component = ReasonReact.reducerComponent("MainPane");

let make = (_) => {
    {
        ...component,

        render: ({ state, reduce }) => {
            module Page = (val state : Component);

            let context = {
                Context.navigate: (pageChange) => {
                    module PageChange = (val pageChange);
                    module NewPage = {
                        module PageComponent = (val PageChange.component);
                        include PageComponent;
                        let make = make(~dynamicProps=PageChange.dynamicProps);
                    };

                    go(reduce, SetPage((module NewPage)));
                    ();
                }
            };

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
};
