open ReactStd;
open ReDomSuite;

type state = string;
type action = SetSearchText(string);

let onSearchTermChanged = (e, { ReasonReact.send }) => {
    let value = ReactEventRe.Form.currentTarget(e)
        |> Element.fromDom
        |> Input.cast
        |> Option.get
        |> Input.value;

    send(SetSearchText(value));
};

let search = ({ Context.navigate }, term) => {
    navigate(SearchPage, term);
};

let component = ReasonReact.reducerComponent("TopBar");
let make = (~context, _) => {
    ...component,

    render: ({ state, handle }) => {
        <div className="top-bar">
            <input _type="text" placeholder="Search" value=state onChange=handle(onSearchTermChanged) />
            <button _type="button" onClick={(_) => search(context, state)}>
                (s2e("Search"))
            </button>
        </div>
    },

    initialState: () => "",

    reducer: (SetSearchText(str), _) =>
        ReasonReact.Update(str)
};