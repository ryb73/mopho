open ReactStd;

type state = {
    currentPage: component,
    currentTrack: option(Models.Track.id)
};

type action =
  | SetPage(component)
  | SetTrack(Models.Track.id);

let getComponent = (type d, key: pageKey(d), dynamicProps: d) => {
    module InitializedPage = {
        module NewPage = (val PageRegister.getPage(key));
        include NewPage;
        let make = make(~dynamicProps);
    };

    (module InitializedPage : Component);
};

let makeContext = ({ ReasonReact.reduce }, playTrack) => {
    Context.playTrack,

    navigate: (key, dynamicProps) =>
        go(reduce, SetPage(getComponent(key, dynamicProps))),
};

let component = ReasonReact.reducerComponent("MainPane");
let make = (~playTrack, _) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state: { currentPage } } = self;

        module Page = (val currentPage : Component);

        <div className="main-pane">
            <Page context=(makeContext(self, playTrack)) />
        </div>
    },

    initialState: () => {
        currentPage: getComponent(HomePage, ()),
        currentTrack: None
    },

    reducer: (action, state) =>
        switch action {
            | SetPage(page) => ReasonReact.Update({ ...state, currentPage: page })
            | SetTrack(id) => ReasonReact.Update({ ...state, currentTrack: Some(id) })
        }
};
