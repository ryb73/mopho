type pageKey('a) =
    | HomePage: pageKey(unit)
    | SearchPage: pageKey(string);

module Context = {
    type t = {
        navigate: 'a. (pageKey('a), 'a) => unit,
        playTrack: Models.Track.id => unit
    };
};

module type Component = {
    type state;
    type action;
    type retainedProps;
    let make: (~context: Context.t, array(unit))
        => ReasonReact.component(state, retainedProps, action);
};

module type Page = {
    type dynamicProps;
    type state;
    type action;
    type retainedProps;
    let make: (~dynamicProps: dynamicProps, ~context: Context.t, array(unit))
        => ReasonReact.component(state, retainedProps, action);
};

type component = (module Component);
type page('d) = (module Page with type dynamicProps = 'd);

let go = (reduce, action) => reduce((_) => action, ());

let s2e = ReasonReact.stringToElement;
