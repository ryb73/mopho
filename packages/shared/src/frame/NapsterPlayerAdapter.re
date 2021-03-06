open FrameConfig;
open ReDomSuite;
open Bluebird;

let player = ref(None);
let getPlayer = () => Option.get(player^);

let post = (message) =>
    IFrameComm.post(message, "http://www.mopho.local", Window.parent(ReDom.window));

let registerEvents = () => {
    NapsterPlayer.onReady((p) => {
        Apis.GetNapsterCredentials.request(config.apiUrl, ())
            |> then_((result) => {
                switch result {
                    | None => failwith("TODO: reauth")
                    | Some({ Apis.GetNapsterCredentials_impl.accessToken, refreshToken }) => {
                        NapsterPlayer.setAuth({
                            "accessToken": accessToken,
                            "refreshToken": refreshToken
                        });

                        NapsterPlayer.testConnection();
                    }
                };
            })
            |> catch((exn) => {
                Js.log2("Caught error in onReady", exn);
                resolve();
            });

        Js.log("ready!");
        player := Some(p);
    });

    NapsterPlayer.onError(Js.log2("Napster error"));

    NapsterPlayer.onMetadata(Js.log2("onMetadata"));
    NapsterPlayer.onPlaySessionExpired(Js.log2("onPlaySessionExpired"));
    NapsterPlayer.onPlayStopped(Js.log2("onPlayStopped"));

    NapsterPlayer.onPlayEvent(({ playing } as msg) => {
        Js.log(NapsterPlayer.playEvent_encode(msg));
        post(IFrameComm.PlayEvent(playing));
    });

    NapsterPlayer.onPlayTimer(({ NapsterPlayer.currentTime, totalTime }) => {
        /* Js.log3("timer",currentTime,totalTime); */
        post(IFrameComm.PlayTimer(currentTime, totalTime));
    });
};

switch (NapsterPlayer.init(config.napsterApiKey, "v2.2")) {
    | Some(error) => Js.log2("Error initializing napster", error)
    | None => registerEvents()
};

IFrameComm.listen("http://www.mopho.local", (message) => {
    switch message {
        | PlaySong(id) => NapsterPlayer.play(id, getPlayer())
        | PauseSong => NapsterPlayer.pause(getPlayer())
        | Seek(pos) => NapsterPlayer.seek(pos, getPlayer())
        | _ => ()
    };
}, Window.parent(ReDom.window));
