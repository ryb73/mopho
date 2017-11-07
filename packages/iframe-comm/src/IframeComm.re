external onMessage : _ [@bs.as "message"] => (MessageEvent.t => unit) => unit = "" [@@bs.send.pipe: Dom.window];

type message = LoggedIn;

let post message targetOrigin => Webapi.Dom.Window.postMessage (message__to_json message) targetOrigin Webapi.Dom.window;

let listen fromOrigin callback => Webapi.Dom.window |> onMessage (fun event => {
    if (event##origin === fromOrigin) {
        switch (message__from_json event##data) {
            | Ok message => callback message
            | _ => ()
        }
    } else { () };
});
