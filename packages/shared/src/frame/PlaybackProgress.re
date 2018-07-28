open ReDomSuite;
open Option.Infix;

type state = option(float);
type action =
    | SetPosition(float)
    | ClearPosition;

let onDragStart = (e) => {
    (DragStartEvent.unsafeCast(e)##dataTransfer)
        |> DataTransfer.setDragImage(Element.Img.blank, 0, 0);

    ();
};

let component = ReasonReact.reducerComponent("PlaybackProgress");
let make = (~progress, ~length, ~onSeek, _) => {
    let getPositionFromMouseEvent = (e) => {
        /* Find where the bar is relative to the screen,
           then get mouse position relative to the bar */
        let targetX = ReactEventRe.Mouse.currentTarget(e)
            |> Element.fromDom
            |> Element.scrollLeft;

        let scrubX = float_of_int(ReactEventRe.Mouse.clientX(e) - targetX);

        let width = ReactEventRe.Mouse.currentTarget(e)
            |> Element.fromDom
            |> Element.scrollWidth
            |> float_of_int;

        (scrubX /. width) *. length
    };

    let onDrag = (e, { ReasonReact.send }) => {
        let position = getPositionFromMouseEvent(e);

        /* TODO: fix this hack. After mouseup a drag event is called with x=0
           because the drop event isn't handled */
        if(position > 0.0) {
            send(SetPosition(position));
        } else {
            ();
        }
    };

    let onDragEnd = (_, { ReasonReact.send, state: position }) => {
        onSeek(Option.get(position));
        send(ClearPosition);
    };

    let onClick = (e) => {
        let position = getPositionFromMouseEvent(e);
        onSeek(position);
    };

    {
        ...component,

        render: ({ handle, state: position }) => {
            let position = position |? progress;
            let progressWidth = Js.String.make(position /. length *. 100.0) ++ "%";
            let progressStyle = ReactDOMRe.Style.make(~width=progressWidth, ());

            <div className="playback-progress" draggable=true onDragStart
                 onDrag=handle(onDrag) onDragEnd=handle(onDragEnd) onClick
            >
                <div className="progress" style=progressStyle />
                <div className="scrub" />
            </div>
        },

        initialState: () => None,

        reducer: (action, _) => {
            switch action {
                | SetPosition(pos) => ReasonReact.Update(Some(pos))
                | ClearPosition => ReasonReact.Update(None)
            }
        }
    };
};