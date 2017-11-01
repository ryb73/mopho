let s2e = ReasonReact.stringToElement;

type state = {
    error: bool
} [@@noserialize];

type action =
  | Error [@@noserialize];

let component = ReasonReact.reducerComponent "Foundation";

Napster.on Error (fun error => {
    Js.log2 "Error:" error;
});

let make _ => {
    ...component,

    render: fun _ => {
        <div className="foundation">
            <div className="top-bar">
                <input _type="text" placeholder="Search" />
            </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e "Leftbar")
                </div>

                <MainPane />
            </div>
            <div className="bottom-bar">
                (s2e "Bottom")
            </div>
        </div>
    },

    initialState: fun () => {
        error: false
    },

    reducer: fun action _ => {
        switch action {
            | Error =>
                ReasonReact.Update { error: true }
        };
    }
};
