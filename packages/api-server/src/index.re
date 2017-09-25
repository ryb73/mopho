open Express;
open Node;
open Js.Promise;

let secret = "secret";
let secure = false;
let uri = "http://www.bc.local:54380";

let app = App.make ();

type session = Js.t {.
    authState: string
};
module Session = ExpressSession.Make({ type t = session; });

App.use app (ExpressSession.make @@ ExpressSession.opts
    ::secret resave::false saveUninitialized::false
    cookie::(ExpressSession.cookieOpts ::secure ()) ()
);

let generateState ()  => {
    Js.Promise.make @@ fun ::resolve ::reject => {
        Crypto.randomBytes 16 (fun result => {
            switch result {
                | `Exception e => reject (Js.Exn.internalToOCamlException @@ Obj.magic e) [@bs]
                | `Buffer buffer => resolve (Buffer.toStringWithEncoding buffer encoding::"base64") [@bs]
            };
        });
    };
};

generateState ();

App.get app path::"/generate-state" @@ Middleware.fromAsync (fun req resp _ => {
    generateState ()
        |> then_ @@ fun state => {
            Js.log @@ "got state: " ^ state;
            let success = Session.set req "napster" {
                "authState": state
            };

            if(success) {
                resolve @@ Response.sendString resp "success";
            } else {
                resolve @@ Response.sendString resp "noo";
            };
        }

        |> catch @@ fun error => {
            Js.log error;
            Response.status resp 500
                |> Response.end_
                |> resolve;
        };
});

App.get app path::"/read-state" @@ Middleware.from (fun req resp _ => {
    /* Express.Response.set resp "Access-Control-Allow-Origin" uri;
    Express.Response.status resp 500
        |> Response.end_; */

    Express.Response.set resp "Access-Control-Allow-Origin" uri;

    let optSessionData = Session.get req "napster";
    switch optSessionData {
        | None => Response.sendJson resp ({ "none": true })
        | Some sessionData => Response.sendString resp sessionData##authState
    };
});

App.listen app onListen::(fun _ => Js.log "listening") ();