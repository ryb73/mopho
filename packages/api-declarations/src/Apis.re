open EndpointFactory;

module GenerateState = Endpoint({
    type req = unit;
    type resp = string;

    let path = "/generate-state/";
    let reqMethod = Post;
});

let generateState = GenerateState.make ();

module NapsterAuth_impl = {
    type req = {
        code: string,
        state: string
    };

    type resp = {
        mophoCode: string
    };

    let path = "/napster-auth/";
    let reqMethod = Get;
};

module NapsterAuth = Endpoint(NapsterAuth_impl);
let napsterAuth = NapsterAuth.make ();

module LogInWithCode = Endpoint({
    type req = string;
    type resp = unit;

    let path = "/login-with-code/";
    let reqMethod = Post;
});

module GetMyUserData_impl = {
    type req = unit;
    type resp = Db.User.t;

    let path = "/get-my-user-data/";
    let reqMethod = Get;
};

module GetMyUserData = Endpoint(GetMyUserData_impl);
let getMyUserData = GetMyUserData.make ()
    |> (fun (handle, request) => {
        let newHandle app callback => {
            handle app (fun req resp next reqArgs => {
                callback req resp next reqArgs 42;
            });
        };

        (newHandle, request);
    });
