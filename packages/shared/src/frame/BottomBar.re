open Option;

let renderNapsterPlayer = (track) => {
    let napsterId = bind(track, ({ Models.Track.napsterId }) => napsterId);
    <Player playerUrl="napster-player-adapter.html" trackId=?napsterId />;
};

let component = ReasonReact.statelessComponent("BottomBar");
let make = (~currentTrack, _) => {
    ...component,

    render: (_) => {
        <div className="bottom-bar">
            (renderNapsterPlayer(currentTrack))
        </div>
    }
};