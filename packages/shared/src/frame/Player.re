open ReDomSuite;
open Option;
open Option.Infix;

type retainedProps = option(Models.Track.napsterId);

let component = ReasonReact.statelessComponentWithRetainedProps("Player");
let make = (~playerUrl, ~trackId=?, _) => {
    let iFrameWindow = ref(None);
    let iFrameMounted = (elem, _) => {
        iFrameWindow := elem
            |> Js.Null.to_opt
            |> map(Element.fromDom)
            >>= IFrame.cast
            |> map(IFrame.contentWindow);
    };

    let playTrack = (id) => {
        Option.get(iFrameWindow^)
            |> IFrameComm.post(IFrameComm.PlaySong(id), "http://www.mopho.local");

        Js.log("posted");
    };

    {
        ...component,

        render: ({ handle }) => {
            let style = ReactDOMRe.Style.make(~display="none", ());
            <iframe src={playerUrl} ref={handle(iFrameMounted)} style />
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

        retainedProps: { trackId }
    }
};