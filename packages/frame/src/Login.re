let s2e = ReasonReact.stringToElement;

type state = bool [@@noserialize];

type action =
  | EnterAuthFlow [@@noserialize];

let component = ReasonReact.reducerComponent "Login";

let goNapster _ _ => ();

let renderLoginOptions { ReasonReact.handle } =>
    <div className="login">
        <h1>(s2e "welcome to mopho.")</h1>
        <p>
            (s2e "sign in with:")
            <ul>
                <li>
                    <a href="#" onClick={handle goNapster}>(s2e "Napster")</a>
                </li>
            </ul>
        </p>
    </div>;

let renderAuthIFrame () => {
    let styles = ReactDOMRe.Style.make width::"100%" height::"100%" border::"0" ();
    <iframe src="napster-auth.html" style=styles />
};

let make _ => {
    ...component,

    render: fun self => {
        let { ReasonReact.state } = self;

        if(state) {
            renderAuthIFrame ();
        } else {
            renderLoginOptions self;
        }
    },

    initialState: fun () => false,

    reducer: fun action _ => {
        switch action {
            | EnterAuthFlow => ReasonReact.Update true
        };
    }
};
