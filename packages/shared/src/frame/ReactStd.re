module type PageChange = {
    type dynamicProps;
    type context;

    module type Component = {
        type retainedProps;
        type state;
        type action;
        let make : (~dynamicProps: dynamicProps, ~context: context, array(unit))
            => ReasonReact.component(state, retainedProps, action);
    };

    let dynamicProps : dynamicProps;
    let component : (module Component);
};

module Context = {
    type t = {
        navigate: (module PageChange) => unit
    };
};

module type Component = {
    type state;
    type action;
    type retainedProps;
    let make: (~context: Context.t, array(unit))
        => ReasonReact.component(state, retainedProps, action);
};

let go = (reduce, action) => reduce((_) => action, ());

let s2e = ReasonReact.stringToElement;
