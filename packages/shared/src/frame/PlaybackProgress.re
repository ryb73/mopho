let component = ReasonReact.statelessComponent("PlaybackProgress");
let make = (~progress, ~length, _) => {
    ...component,

    render: (_) => {
        let progressWidth = Js.String.make(progress /. length *. 100.0) ++ "%";
        let progressStyle = ReactDOMRe.Style.make(~width=progressWidth, ());

        <div className="playback-progress">
            <div className="progress" style=progressStyle />
            <div className="scrub" />
        </div>
    }
};