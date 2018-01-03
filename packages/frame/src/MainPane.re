let component = ReasonReact.statelessComponent("MainPane");

let make = (_) => {
    ...component,
    render: (_) => <div className="main-pane"> (ReasonReact.stringToElement("main panneeee")) </div>
};
