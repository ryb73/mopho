open ReactStd;
open ReDomSuite;
open Option;
open Option.Infix;

type retainedProps = option(Models.Track.napsterId);

type state = { playing: bool };

type action = SetPlaying(bool);

let renderPlayPause = (classname, { ReasonReact.state: { playing } }) => {
    switch playing {
        | false => <i className={Cn.make(["fa-play", classname])} />
        | true => <i className={Cn.make(["fa-pause", classname])} />
    };
};

let component = ReasonReact.reducerComponentWithRetainedProps("Player");
let make = (~playerUrl, ~trackId=?, _) => {
    let iFrameWindow = ref(None);
    let iFrameMounted = (elem, { ReasonReact.reduce }) => {
        let contentWindow = elem
            |> Js.Null.to_opt
            |> map(Element.fromDom)
            >>= IFrame.cast
            |> map(IFrame.contentWindow);

        contentWindow
            |> may((contentWindow) =>
                IFrameComm.listen("http://www.mopho.local", (message) => {
                    switch message {
                        | PlayEvent(playing) => go(reduce, SetPlaying(playing))
                        | _ => ()
                    };
                }, contentWindow)
            );

        iFrameWindow := contentWindow;
    };

    let playTrack = (id) => {
        Option.get(iFrameWindow^)
            |> IFrameComm.post(IFrameComm.PlaySong(id), "http://www.mopho.local");

        Js.log("posted");
    };

    {
        ...component,

        render: ({ handle } as self) => {
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
            </div>
        },

        didUpdate: ({ oldSelf }) => {
            if(oldSelf.retainedProps !== trackId) {
                switch (trackId) {
                    | Some(trackId) => playTrack(trackId)
                    | None => ()
                };
            } else {
                ();
            };
        },

        retainedProps: { trackId },

        initialState: () => { playing: false },

        reducer: (SetPlaying(playing), _) => ReasonReact.Update({ playing: playing })
    }
};