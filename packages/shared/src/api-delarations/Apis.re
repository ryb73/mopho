open EndpointFactory;

module GenerateState = Endpoint({
    [@autoserialize] type req = unit;
    [@autoserialize] type resp = string;
    let path = "/generate-state/";
    let reqMethod = Post;
});

module NapsterAuth_impl = {
    [@autoserialize]
    type req = {
        code: string,
        state: string
    };

    [@autoserialize] type resp = {mophoCode: string};
    let path = "/napster-auth/";
    let reqMethod = Get;
};
module NapsterAuth = Endpoint(NapsterAuth_impl);

module LogInWithCode = Endpoint({
    [@autoserialize] type req = string;
    [@autoserialize] type resp = unit;
    let path = "/login-with-code/";
    let reqMethod = Post;
});

module GetMyUserData_impl = {
    [@autoserialize] type req = unit;
    [@autoserialize] type resp = Models.User.t;
    let path = "/get-my-user-data/";
    let reqMethod = Get;
};
module GetMyUserData = Endpoint(GetMyUserData_impl);

module LogOut = Endpoint({
    [@autoserialize] type req = unit;
    [@autoserialize] type resp = unit;
    let path = "/log-out/";
    let reqMethod = Post;
});

module Search_impl = {
    [@autoserialize] type req = string;
    [@autoserialize] type resp = {
        artists: array(Models.Artist.t),
        albums: array(Models.Album.t),
        tracks: array(Models.Track.t)
    };
    let path = "/search/";
    let reqMethod = Get;
};
module Search = Endpoint(Search_impl);