open ReactStd;

type state = {
    currentTrack: option(Models.Track.t),
    currentPage: component
};

type action =
    | SetTrack(Models.Track.t)
    | SetPage(component);

let getComponent = (type d, key: pageKey(d), dynamicProps: d) => {
    module InitializedPage = {
        module NewPage = (val PageRegister.getPage(key));
        include NewPage;
        let make = make(~dynamicProps);
    };

    (module InitializedPage : Component);
};

let makeContext = ({ ReasonReact.send }) => {
    Context.playTrack: (trackId) =>
        send(SetTrack(trackId)),

    navigate: (key, dynamicProps) =>
        send(SetPage(getComponent(key, dynamicProps))),
};

let component = ReasonReact.reducerComponent("Foundation");
let make = (_) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state: { currentTrack, currentPage } } = self;

        let context = makeContext(self);

        <div className="foundation">
            <div className="top-bar">
                <TopBar context />
            </div>

            <div className="center-bar">
                <div className="left-pane">
                    (s2e("Leftbar"))
                </div>

                <MainPane context currentPage />
            </div>

            <div className="bottom-bar">
                <BottomBar currentTrack />
            </div>
        </div>
    },

    initialState: () => {
        currentTrack: None,
        currentPage: getComponent(HomePage, ())
    },

    reducer: (action, state) => {
        switch action {
            | SetTrack(id) => ReasonReact.Update({ ...state, currentTrack: Some(id) })
            | SetPage(page) => ReasonReact.Update({ ...state, currentPage: page })
        };
    }
};
