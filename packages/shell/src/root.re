let s2e = ReasonReact.stringToElement;

let component = ReasonReact.statelessComponent "Page";

let make _children => {
    ...component,

    render: fun _ => {
        let url = "https://api.napster.com/oauth/authorize?client_id=" ^ "API_KEY" ^
            "&redirect_uri=http://" ^ "yourdomain.com" ^ "/&response_type=code";
        <div>
            (s2e url)
        </div>
    }
};