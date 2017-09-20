let component = ReasonReact.statelessComponent "Page";

let make _children => {
    ...component,

    render: fun _ => {
        let url = "https://open.spotify.com/embed?uri=spotify:user:spotify:playlist:3rgsDhGHZxZ9sB9DQWQfuf";
        <div>
            <iframe src={url} />
        </div>
    }
};