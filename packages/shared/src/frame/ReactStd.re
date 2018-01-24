module type Page = {
    type context;
    type dynamicProps;
    type state;
    type action;
    type retainedProps;
    let make: (~dynamicProps: dynamicProps, ~context: context, array(unit))
        => ReasonReact.component(state, retainedProps, action);
};

module type Component = {
    type context;
    type state;
    type action;
    type retainedProps;
    let make: (~context: context, array(unit))
        => ReasonReact.component(state, retainedProps, action);
};

type page('c, 'd) = (module Page with type context = 'c and type dynamicProps = 'd);

type pageKey('a) =
    | HomePage: pageKey(unit)
    | SearchPage: pageKey(string);

module Context = {
    type t = {
        navigate: (module Component with type context = t) => unit,
        getPage: 'a. pageKey('a) => page(t, 'a)
    };
};

module type ContextPage = Page with type context = Context.t;

type component = (module Component with type context = Context.t);

let navv = (type d, navigate, component : page(Context.t, d), dprops : d) => {
    module InitializedPage = {
        module NewPage = (val component);
        include NewPage;
        let make = make(~dynamicProps=dprops);
    };

    navigate((module InitializedPage : Component with type context = Context.t));
};

let go = (reduce, action) => reduce((_) => action, ());

let s2e = ReasonReact.stringToElement;
