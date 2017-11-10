open ReDomSuite;

type message = LoggedIn;

let post message targetOrigin targetWindow =>
    Window.postMessage targetWindow (message__to_json message) targetOrigin;

let listen messageToForward fromOrigin callback fromWindow =>
    Window.onMessage ReDom.window (fun event => {
        if (event##origin === fromOrigin && event##source === fromWindow) {
            switch (message__from_json event##data) {
                | Ok message => {
                    if(message === messageToForward) {
                        callback message;
                    } else { (); }
                }
                | _ => ()
            }
        } else { () };
    });
