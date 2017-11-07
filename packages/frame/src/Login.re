let s2e = ReasonReact.stringToElement;

type state = {
    inAuthFlow: bool,
    iframeRef: ref (option Dom.element)
 } [@@noserialize];

type action =
  | EnterAuthFlow [@@noserialize];

let component = ReasonReact.reducerComponent "Login";

let renderLoginOptions { ReasonReact.reduce } =>
    <div>
        <h1>(s2e "welcome to mopho.")</h1>
        <p>
            (s2e "sign in with:")
        </p>

        <ul>
            <li>
                <a href="#" onClick={reduce (fun _ => EnterAuthFlow)}>(s2e "Napster")</a>
            </li>
        </ul>
    </div>;

IframeComm.post LoggedIn ;

let renderAuthIFrame () => <iframe src="napster-auth.html" />;

let make _ => {
    ...component,

    render: fun self => {
        let { ReasonReact.state } = self;

        let content = if(state) {
            renderAuthIFrame ();
        } else {
            renderLoginOptions self;
        };

        <div className="login">(content)</div>;
    },

    initialState: fun () => { inAuthFlow: false, iframeRef: ref None },

    reducer: fun action state => {
        switch action {
            | EnterAuthFlow => ReasonReact.Update true /* { ...state, inAuthFlow: true } */
        };
    }
};
