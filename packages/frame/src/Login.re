open ReDomSuite;
open Option;
open Option.Infix;

let flip = BatPervasives.flip;
let s2e = ReasonReact.stringToElement;

type state =
    | Options
    | InAuthFlow
    | ErrorOccurred [@@noserialize];

type action =
    | EnterAuthFlow
    | SetError [@@noserialize];

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

let go reduce action => reduce (fun _ => action) ();

let iFrameMounted element { ReasonReact.reduce } => {
    element
        |> Js.Null.to_opt
        |> map Element.make
        >>= IFrame.cast
        |> map IFrame.contentWindow
        |> map @@
            IFrameComm.listen IFrameComm.LoggedIn "http://www.mopho.local" (fun LoggedIn => {
                Js.log "logged in!!";
                ();
            });

    ();
};

let renderAuthIFrame { ReasonReact.handle } =>
    <iframe src="napster-auth.html" ref={handle iFrameMounted} />;

let make _ => {
    ...component,

    render: fun self => {
        let { ReasonReact.state } = self;

        let content = switch(state) {
            | InAuthFlow => renderAuthIFrame self
            | Options => renderLoginOptions self
            | ErrorOccurred => s2e "Error"
        };

        <div className="login">(content)</div>;
    },

    initialState: fun () => Options,

    reducer: fun action _ => {
        switch action {
            | EnterAuthFlow => ReasonReact.Update InAuthFlow
            | SetError => ReasonReact.Update ErrorOccurred
        };
    }
};
