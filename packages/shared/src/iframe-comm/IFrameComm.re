open ReDomSuite;

[@decco]
type message =
    | LoggedIn(string) /* code */
    | PlaySong(Models.Track.napsterId)
    | PlayEvent(bool) /* playing */
    | PlayTimer(float, float) /* progress, totalLength */
    | PauseSong
    | Seek(float);

let post = (message, targetOrigin, targetWindow) =>
  Window.postMessage(targetWindow, message_encode(message), targetOrigin);

let listen = (fromOrigin, callback, fromWindow) => {
    let listener = (event) => {
        if (event##origin === fromOrigin && event##source === fromWindow) {
            switch (message_decode(event##data)) {
                | Ok(message) => callback(message)
                | Error(errorMsg) => Js.log3("Unhandled IFrame message:", errorMsg, event##data)
            };
        } else {
            ();
        };
    };

    Window.onMessage(ReDom.window, listener);
    listener;
};

let removeListener = (listener) => Window.clearMessageListener(ReDom.window, listener);
