open ReactStd;

/* NapsterPlayer.on(Error, (error) => Js.log2("Error:", error)); */

type state = {
    currentTrack: option(Models.Track.id)
};

type action =
    | SetTrack(Models.Track.id);

let playTrack = ({ ReasonReact.reduce }, trackId) =>
    go(reduce, SetTrack(trackId));

let component = ReasonReact.reducerComponent("Foundation");
let make = (_) => {
    ...component,

    render: (self) => {
        let {  ReasonReact.state: { currentTrack } } = self;

        <div className="foundation">
            <div className="top-bar"> <input _type="text" placeholder="Search" /> </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e("Leftbar"))
                </div>

                <MainPane playTrack={playTrack(self)} />
            </div>

            <BottomBar currentTrack />
        </div>
    },

    initialState: () => { currentTrack: None },

    reducer: (action, _) => {
        switch action {
            | SetTrack(id) => ReasonReact.Update({ currentTrack: Some(id) })
        };
    }
};
