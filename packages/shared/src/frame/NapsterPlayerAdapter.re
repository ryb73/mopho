open FrameConfig;
open ReDomSuite;

let player = ref(None);

switch (NapsterPlayer.init(config.napsterApiKey, "v2.2")) {
    | Some(error) => Js.log2("Error initializing napster", error)
    | None => {
        NapsterPlayer.onReady((p) => {
            Js.log("ready!");
            player := Some(p);
        });

        NapsterPlayer.onError(Js.log2("Napster error"));
    }
};

let playSong = (id) => {
    Js.log2("playing song", id);
    NapsterPlayer.play(id, Option.get(player^));
};

IFrameComm.listen("http://www.mopho.local", (message) => {
    Js.log2("got message!",message);

    switch message {
        | PlaySong(id) => playSong(id)
        | _ => ()
    };
}, Window.parent(ReDom.window));