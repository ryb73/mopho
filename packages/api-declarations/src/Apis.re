open EndpointFactory;

module GenerateState = Endpoint({
    type req = unit;
    type resp = string;

    let path = "/generate-state/";
    let reqMethod = Post;
});

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

module LogInWithCode = Endpoint({
    type req = string;
    type resp = unit;

    let path = "/login-with-code/";
    let reqMethod = Post;
});

module GetMyUserData_impl = {
    type req = unit;
    type resp = {
        id: int,
        name: string
    };

    let path = "/get-my-user-data/";
    let reqMethod = Get;
};

module GetMyUserData = Endpoint(GetMyUserData_impl);
