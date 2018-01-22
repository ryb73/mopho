open ReactStd;

NapsterPlayer.on(Error, (error) => Js.log2("Error:", error));

let component = ReasonReact.statelessComponent("Foundation");
let make = (_) => {
    ...component,

    render: (_) => {
        <div className="foundation">
            <div className="top-bar"> <input _type="text" placeholder="Search" /> </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e("Leftbar"))
                </div>

                <MainPane />
            </div>
            <div className="bottom-bar"> (s2e("Bottom")) </div>
        </div>
    }

};
