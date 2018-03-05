open ReDomSuite;
open Option;
open Option.Infix;

type retainedProps = option(Models.Track.napsterId);

type playingState =
    | NotPlaying
    | WaitingToPlay
    | Playing;

type state = {
    playing: playingState,
    playbackState: option((float, float))
};

type action =
    | SetPlayingState(playingState)
    | PlayTrack(Models.Track.napsterId)
    | SetPlaybackState(float, float)
    | Seek(float);

let renderPlaybackProgress = (playbackState, { ReasonReact.send }) => {
    switch playbackState {
        | None => ReasonReact.nullElement
        | Some((progress, length)) => <PlaybackProgress progress length onSeek=((pos) => send(Seek(pos))) />
    };
};

let component = ReasonReact.reducerComponentWithRetainedProps("Player");
let make = (~playerUrl, ~trackId=?, _) => {
    let iFrameState = ref(None);

    let iFrameMounted = (elem, { ReasonReact.send }) => {
        iFrameState^
            |> may(((_, listener)) => IFrameComm.removeListener(listener));

        iFrameState := elem
            |> Js.Nullable.to_opt
            |> map(Element.fromDom)
            >>= IFrame.cast
            |> map(IFrame.contentWindow)
            |> map((window) => {
                let listener = IFrameComm.listen("http://www.mopho.local", (message) => {
                    switch message {
                        | PlayEvent(true) => send(SetPlayingState(Playing))
                        | PlayEvent(false) => send(SetPlayingState(NotPlaying))
                        | PlayTimer(progress, length) => send(SetPlaybackState(progress, length))
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

    let pause = (_) => post(IFrameComm.PauseSong);

    let renderPlayPause = (classname, { ReasonReact.state: { playing }, send }) => {
        switch (playing, trackId) {
            | (WaitingToPlay, _) =>
                <i className={Cn.make(["fa-spinner", "fa-spin", classname])} />

            | (NotPlaying, Some(trackId)) =>
                <i className={Cn.make(["fa-play", classname])}
                    onClick={(_) => send(PlayTrack(trackId))} />

            | (NotPlaying, None) =>
                <i className={Cn.make(["fa-play", classname])} />

            | (Playing, _) =>
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

                (renderPlaybackProgress(playbackState, self))
            </div>
        },

        didUpdate: ({ oldSelf, newSelf }) => {
            if(oldSelf.retainedProps !== trackId) {
                switch (trackId) {
                    | Some(trackId) => newSelf.send(PlayTrack(trackId))
                    | None => ()
                };
            } else {
                ();
            };
        },

        retainedProps: { trackId },

        initialState: () => { playing: NotPlaying, playbackState: None },

        reducer: (action, { playing } as state) =>
            switch action {
                | SetPlayingState(playing) =>
                    ReasonReact.Update({ ...state, playing })

                | SetPlaybackState(progress, length) =>
                    ReasonReact.Update({...state, playbackState: Some((progress, length)) })

                | PlayTrack(id) => {
                    post(IFrameComm.PlaySong(id));
                    ReasonReact.Update({ ...state, playing: WaitingToPlay });
                }

                | Seek(pos) => {
                    post(IFrameComm.Seek(pos));

                    switch playing {
                        | Playing =>
                            ReasonReact.Update(
                                { ...state, playing: WaitingToPlay }
                            );

                        | _ => ReasonReact.NoUpdate
                    };
                }
            }
    }
};