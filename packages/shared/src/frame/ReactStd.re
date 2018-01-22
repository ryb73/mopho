module type Component = {
    type state;
    type action;
    type retainedProps;
    let make: (~context: Context.t, array(unit))
        => ReasonReact.component(state, retainedProps, action);
};

let go = (reduce, action) => reduce((_) => action, ());

let s2e = ReasonReact.stringToElement;
