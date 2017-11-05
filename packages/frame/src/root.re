type state = {
    isAuthenticated: bool
} [@@noserialize];

type action =
  | SetAuthenticated [@@noserialize];

let component = ReasonReact.reducerComponent "Root";

let make _ => {
    ...component,

    render: fun { state } => {
        let content = if(state.isAuthenticated) {
            <Foundation />;
        } else {
            <Login />;
        };

        <div className="root">(content)</div>
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
