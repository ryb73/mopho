open FrameConfig;
open ReactStd;

type state = Context.t;
type action = unit;

NapsterPlayer.on(Error, (error) => Js.log2("Error:", error));

let component = ReasonReact.reducerComponent("Foundation");
let make = (_) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state: context } = self;

        <div className="foundation">
            <div className="top-bar"> <input _type="text" placeholder="Search" /> </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e("Leftbar"))
                </div>

                <MainPane context />
            </div>
            <div className="bottom-bar"> (s2e("Bottom")) </div>
        </div>
    },

    initialState: () : state => { Context.apiUrl: config.apiUrl },

    reducer: (_ : action, _ : state) => ReasonReact.NoUpdate
};
