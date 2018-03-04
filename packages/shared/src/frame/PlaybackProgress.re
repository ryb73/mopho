open ReactStd;

let component = ReasonReact.statelessComponent("PlaybackProgress");
let make = (~progress, ~length, _) => {
    ...component,

    render: (_) => {
        <span>(s2e(Js.String.make(progress) ++ "/" ++ Js.String.make(length)))</span>
    }
};