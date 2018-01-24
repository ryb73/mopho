open ReactStd;

type state = component;

type action =
  | SetPage(component);

let component = ReasonReact.reducerComponent("MainPane");

let getComponent = (type d, key: pageKey(d), dynamicProps: d) => {
    module InitializedPage = {
        module NewPage = (val PageRegister.getPage(key));
        include NewPage;
        let make = make(~dynamicProps);
    };

    (module InitializedPage : Component);
};

let makeContext = ({ ReasonReact.reduce }) => {
    Context.navigate: (key, dynamicProps) =>
        go(reduce, SetPage(getComponent(key, dynamicProps)))
};

let make = (_) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state } = self;

        module Page = (val state : Component);

        <div className="main-pane">
            <Page context=(makeContext(self)) />
        </div>
    },

    initialState: () => getComponent(HomePage, ()),

    reducer: (action, _) =>
        switch action {
            | SetPage(page) => ReasonReact.Update(page)
        }
};
