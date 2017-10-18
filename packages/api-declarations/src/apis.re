open EndpointFactory;

module GenerateState = Endpoint({
    type req = unit;
    type resp = string;

    let path = "/generate-state/";
    let reqMethod = Post;
});

module GetAccessTokens_impl = {
    type req = {
        code: string,
        state: string
    };

    type resp = {
        accessToken: string,
        refreshToken: string
    };

    let path = "/get-access-tokens/";
    let reqMethod = Get;
};

module GetAccessTokens = Endpoint(GetAccessTokens_impl);