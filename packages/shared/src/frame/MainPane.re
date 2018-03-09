open ReactStd;

let component = ReasonReact.statelessComponent("MainPane");
let make = (~context, ~currentPage, _) => {
    ...component,

    render: (_) => {
        module Page = (val currentPage : Component);

        <div className="main-pane">
            <Page context />
        </div>
    }
};