open ReDomSuite;
open Option;
open Option.Infix;
open FrameConfig;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);

let mapP = BluebirdEx.map;
let s2e = ReasonReact.string;

type state =
    | Options
    | InAuthFlow
    | ErrorOccurred;

type action =
    | EnterAuthFlow
    | SetError;

let component = ReasonReact.reducerComponent("Login");

let renderLoginOptions = ({ ReasonReact.send }) =>
    <div>
        <h1> (s2e("welcome to mopho.")) </h1>
        <p> (s2e("sign in with:")) </p>
        <ul>
            <li> <a href="#" onClick=(_ => send(EnterAuthFlow))> (s2e("Napster")) </a> </li>
        </ul>
    </div>;

let loginWithCode = (onLoggedIn, code) =>
    Apis.LogInWithCode.request(config.apiUrl, code)
    |> mapP((_) => onLoggedIn())
    |> catch((exn) => {
        Js.log2("Error logging in", exn);
        resolve()
    });

let iFrameMounted = (onLoggedIn, element, _) => {
    element
        |> Js.Nullable.toOption
        |> map(Element.fromDom)
        >>= IFrame.cast
        |> map(IFrame.contentWindow)
        |> map(IFrameComm.listen("http://www.mopho.local", (message) => {
            ignore @@ switch message {
                | LoggedIn(code) => loginWithCode(onLoggedIn, code)
                | _ => resolve()
            };
        }));

    ();
};

let renderAuthIFrame = ({ ReasonReact.handle }, onLoggedIn) =>
    <iframe src="napster-auth.html" ref=(handle(iFrameMounted(onLoggedIn))) />;

let make = (~onLoggedIn, _) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state } = self;
        let content =
            switch state {
                | InAuthFlow => renderAuthIFrame(self, onLoggedIn)
                | Options => renderLoginOptions(self)
                | ErrorOccurred => s2e("Error")
            };

        <div className="login"> content </div>
    },

    initialState: () => Options,

    reducer: (action, _) =>
        switch action {
            | EnterAuthFlow => ReasonReact.Update(InAuthFlow)
            | SetError => ReasonReact.Update(ErrorOccurred)
        }
};
