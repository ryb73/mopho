open Express;

let secret = "secret";
let secure = false;

let app = App.make ();

type session = Js.t {.
    authState: string
};
module Session = ExpressSession.Make({ type t = session; });

App.use app (ExpressSession.make @@ ExpressSession.opts
    ::secret resave::false saveUninitialized::false
    cookie::(ExpressSession.cookieOpts ::secure ()) ()
);

App.get app path::"/generate-state" @@ Middleware.from (fun req resp _ => {
    let success = Session.set req "napster" {
        "authState": "allstate"
    };

    if(success) {
        Response.sendString resp "success";
    } else {
        Response.sendString resp "noo";
    }
});

App.get app path::"/read-state" @@ Middleware.from (fun req resp _ => {
    let optSessionData = Session.get req "napster";
    switch optSessionData {
        | None => Response.sendString resp "none"
        | Some sessionData => Response.sendString resp sessionData##authState
    };
});

App.listen app onListen::(fun _ => Js.log "listening") ();