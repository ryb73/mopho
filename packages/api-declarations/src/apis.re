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

module ExchangeAuthCode_impl = {
    type req = string;
    type resp = { authToken: string };

    let path = "/exchange-auth-code/";
    let reqMethod = Post;
};

module ExchangeAuthCode = Endpoint(ExchangeAuthCode_impl);