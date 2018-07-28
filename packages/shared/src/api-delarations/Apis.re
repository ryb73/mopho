open EndpointFactory;

module GenerateState = Endpoint({
    [@decco] type req = unit;
    [@decco] type resp = string;
    let path = "/generate-state/";
    let reqMethod = Post;
});

module NapsterAuth_impl = {
    [@decco]
    type req = {
        code: string,
        state: string
    };

    [@decco] type resp = { mophoCode: string };

    let path = "/napster-auth/";
    let reqMethod = Get;
};
module NapsterAuth = Endpoint(NapsterAuth_impl);

module GetNapsterCredentials_impl = {
    [@decco] type req = unit;

    [@decco]
    type tokens = {
        accessToken: string,
        refreshToken: string
    };

    [@decco] type resp = option(tokens);

    let path = "/get-napster-credentials/";
    let reqMethod = Get;
};
module GetNapsterCredentials = Endpoint(GetNapsterCredentials_impl);

module LogInWithCode = Endpoint({
    [@decco] type req = string;
    [@decco] type resp = unit;
    let path = "/login-with-code/";
    let reqMethod = Post;
});

module GetMyUserData_impl = {
    [@decco] type req = unit;
    [@decco] type resp = Models.User.t;
    let path = "/get-my-user-data/";
    let reqMethod = Get;
};
module GetMyUserData = Endpoint(GetMyUserData_impl);

module LogOut = Endpoint({
    [@decco] type req = unit;
    [@decco] type resp = unit;
    let path = "/log-out/";
    let reqMethod = Post;
});

module Search_impl = {
    [@decco] type req = string;
    [@decco] type resp = {
        artists: array(Models.Artist.t),
        albums: array(Models.Album.t),
        tracks: array(Models.Track.t)
    };
    let path = "/search/";
    let reqMethod = Get;
};
module Search = Endpoint(Search_impl);