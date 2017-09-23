open Express;

let app = App.make ();

 /* (ExpressSession.make ()); */

App.get app path::"/generate-state" @@ Middleware.from (fun _ resp _ => {
    Response.sendString resp "boggle";
});

App.listen app onListen::(fun _ => Js.log "listening") ();