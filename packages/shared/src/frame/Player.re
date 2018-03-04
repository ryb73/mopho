open ReactStd;
open ReDomSuite;
open Option;
open Option.Infix;

type retainedProps = option(Models.Track.napsterId);

type state = {
    playing: bool,
    playbackState: option((float, float))
};

type action =
    | SetPlaying(bool)
    | SetPlaybackState(float, float);

let renderPlaybackProgress = (playbackState) => {
    switch playbackState {
        | None => ReasonReact.nullElement
        | Some((progress, length)) => <PlaybackProgress progress length />
    };
};

let component = ReasonReact.reducerComponentWithRetainedProps("Player");
let make = (~playerUrl, ~trackId=?, _) => {
    let iFrameState = ref(None);

    let iFrameMounted = (elem, { ReasonReact.reduce }) => {
        /* Js.log2("mounted", elem); */

        iFrameState^
            |> may(((_, listener)) => IFrameComm.removeListener(listener));

        iFrameState := elem
            |> Js.Null.to_opt
            |> map(Element.fromDom)
            >>= IFrame.cast
            |> map(IFrame.contentWindow)
            |> map((window) => {
                let listener = IFrameComm.listen("http://www.mopho.local", (message) => {
                    /* Js.log2("message!",message); */
                    switch message {
                        | PlayEvent(playing) => go(reduce, SetPlaying(playing))
                        | PlayTimer(progress, length) => go(reduce, SetPlaybackState(progress, length))
                        | _ => ()
                    };
                }, window);

                (window, listener);
            });
    };

    let post = (message) =>
        iFrameState^
            |> Option.map(((window, _)) => window)
            |> Option.get
            |> IFrameComm.post(message, "http://www.mopho.local");

    let playTrack = (id) => post(IFrameComm.PlaySong(id));

    let pause = (_) => post(IFrameComm.PauseSong);

    let renderPlayPause = (classname, { ReasonReact.state: { playing } }) => {
        switch (playing, trackId) {
            | (false, Some(trackId)) =>
                <i className={Cn.make(["fa-play", classname])}
                    onClick={(_) => playTrack(trackId)} />

            | (false, None) =>
                <i className={Cn.make(["fa-play", classname])} />

            | (true, _) =>
                <i className={Cn.make(["fa-pause", classname])} onClick=pause />
        };
    };

    {
        ...component,

        render: ({ handle, state: { playbackState } } as self) => {
            let style = ReactDOMRe.Style.make(~display="none", ());

            let controlsClassname = Cn.make([
                "fa",
                "fa-3x",
                "disabled" |> Cn.ifBool(trackId === None)
            ]);

            <div className="player">
                <iframe src={playerUrl} ref={handle(iFrameMounted)} style />
                <button _type="button">
                    <i className={Cn.make(["fa-angle-double-left", controlsClassname])} />
                </button>
                <button _type="button">
                    (renderPlayPause(controlsClassname, self))
                </button>
                <button _type="button">
                    <i className={Cn.make(["fa-angle-double-right", controlsClassname])} />
                </button>

                (renderPlaybackProgress(playbackState))
            </div>
        },

        didUpdate: ({ oldSelf }) => {
            /* Js.log("didUpdate"); */
            if(oldSelf.retainedProps !== trackId) {
                /* Js.log("hoo boy"); */
                switch (trackId) {
                    | Some(trackId) => playTrack(trackId)
                    | None => ()
                };
            } else {
                ();
            };
        },

        retainedProps: { trackId },

        initialState: () => { playing: false, playbackState: None },

        reducer: (action, state) =>
            switch action {
                | SetPlaying(playing) => ReasonReact.Update({ ...state, playing })
                | SetPlaybackState(progress, length) => ReasonReact.Update({ ...state, playbackState: Some((progress, length)) })
            }
    }
};