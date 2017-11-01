let component = ReasonReact.statelessComponent "MainPane";

let make _ => {
    ...component,

    render: fun _ => {
        <div className="main-pane">
            (ReasonReact.stringToElement "main panneeee")
        </div>
    }
};
