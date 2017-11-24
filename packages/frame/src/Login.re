open ReDomSuite;
open Option;
open Option.Infix;
open FrameConfig;
open Js.Promise;

let mapP = PromiseEx.map;

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

let loginWithCode onLoggedIn code => {
    Apis.LogInWithCode.request config.apiUrl code
        |> mapP (fun _ => {
            onLoggedIn ();
        })
        |> catch (fun exn => {
            Js.log2 "Error logging in" exn;
            resolve ();
        });
};

let iFrameMounted onLoggedIn element _ => {
    element
        |> Js.Null.to_opt
        |> map Element.make
        >>= IFrame.cast
        |> map IFrame.contentWindow
        |> map @@
            IFrameComm.listen "http://www.mopho.local" (fun message => {
                switch message {
                    | LoggedIn code => loginWithCode onLoggedIn code;
                };

                ();
            });

    ();
};

let renderAuthIFrame { ReasonReact.handle } onLoggedIn =>
    <iframe src="napster-auth.html" ref={handle (iFrameMounted onLoggedIn)} />;

let make ::onLoggedIn _ => {
    ...component,

    render: fun self => {
        let { ReasonReact.state } = self;

        let content = switch(state) {
            | InAuthFlow => renderAuthIFrame self onLoggedIn
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
