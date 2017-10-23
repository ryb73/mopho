type state = {
    error: bool
} [@@noserialize];

type action =
  | Error [@@noserialize];

let component = ReasonReact.reducerComponent "Page";

let make _ => {
    let s2e = ReasonReact.stringToElement;

    /* Napster.init apiKey "v2.2";
    Napster.on Ready (fun _ => {
        Napster.load ();
        Napster.get Js.false_ "/me" (fun data => {
            Js.log data;
        });
    }); */

    Napster.on Error (fun error => {
        Js.log2 "Error:" error;
    });

    let renderError =
        <div>
            (s2e "An error occurred")
        </div>;

    let renderIFrame = {
        let styles = ReactDOMRe.Style.make width::"100%" height::"100%" border::"0" ();
        <iframe src="napster-auth.html" style=styles />
    };

    {
        ...component,

        render: fun { state } => {
            switch state.error {
                | true => renderError
                | false => renderIFrame
            };
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
    }
};
