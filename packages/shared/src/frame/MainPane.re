open ReactStd;

/* let s = PageRegister.register((module SearchPage)); */

type state = component;

type action =
  | SetPage(component);

let component = ReasonReact.reducerComponent("MainPane");

let make = (_) => {
    {
        ...component,

        render: ({ state, reduce }) => {
            module Page = (val state : Component with type context = Context.t);

            let context = {
                Context.navigate: (page) => {
                    go(reduce, SetPage(page));
                    ();
                },

                getPage: PageRegister.getPage
            };

            <div className="main-pane">
                <Page context />
            </div>
        },

        initialState: () : state => {
            module InitializedPage = {
                include HomePage;
                let make = make(~dynamicProps=());
            };

            (module InitializedPage);
        },

        reducer: (action, _) =>
            switch action {
                | SetPage(page) => ReasonReact.Update(page)
            }
    };
};
