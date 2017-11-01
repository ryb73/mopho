type state = {
    isAuthenticated: bool
} [@@noserialize];

type action =
  | SetAuthenticated [@@noserialize];

let component = ReasonReact.reducerComponent "Root";

let make _ => {
    ...component,

    render: fun { state } => {
        if(state.isAuthenticated) {
            <Foundation />;
        } else {
            <Login />;
        }
    },

    initialState: fun () => {
        isAuthenticated: false
    },

    reducer: fun action _ => {
        switch action {
            | SetAuthenticated => ReasonReact.Update { isAuthenticated: true }
        };
    }
};
