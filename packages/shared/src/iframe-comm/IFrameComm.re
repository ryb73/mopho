open ReDomSuite;

[@autoserialize]
type message =
    | LoggedIn(string);

let post = (message, targetOrigin, targetWindow) =>
  Window.postMessage(targetWindow, message__to_json(message), targetOrigin);

let listen = (fromOrigin, callback, fromWindow) =>
    Window.onMessage(ReDom.window, (event) =>
        if (event##origin === fromOrigin && event##source === fromWindow) {
            switch (message__from_json(event##data)) {
                | Ok(message) => callback(message)
                | Error(_) => Js.log2("Unhandled IFrame message:", event##data)
            };
        } else {
          ();
        }
    );
