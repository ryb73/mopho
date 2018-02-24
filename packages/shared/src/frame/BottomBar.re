open ReactStd;

let component = ReasonReact.statelessComponent("BottomBar");
let make = (~currentTrack, _) => {
    ...component,

    render: (_) => {
        <div className="bottom-bar">
            (switch (currentTrack) {
                | None => s2e("Nothing playing")
                | Some(id) => s2e({j|Playing $id|j})
            })
        </div>
    }
};