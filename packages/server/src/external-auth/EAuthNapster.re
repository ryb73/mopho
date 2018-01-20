module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let config = ConfigLoader.config;

[@autoserialize]
type tokens = {
    access_token: string,
    refresh_token: string,
    expires_in: int
};

let _accessTokenApiCall = (reqData) => {
    Superagent.post("https://api.napster.com/oauth/access_token")
        |> Superagent.Post.send(reqData)
        |> Superagent.Post.end_
        |> map(RespParser.parse(tokens__from_json))
        |> unwrapResult;
};

let getTokensFromCode = (code) => {
    Js.Dict.fromList
        ([
            ("client_id", config.napster.apiKey),
            ("client_secret", config.napster.secret),
            ("response_type", "code"),
            ("grant_type", "authorization_code"),
            ("code", code)
        ])
        |> Js.Dict.map([@bs] ((s) => Js.Json.string(s)))
        |> Js.Json.object_
        |> _accessTokenApiCall;
};

let refreshAccessToken = (userId) => {
    NapsterUser.getRefreshToken(userId)
        |> thenMaybe((refreshToken) => {
            Js.Dict.fromList
                ([
                    ("client_id", config.napster.apiKey),
                    ("client_secret", config.napster.secret),
                    ("response_type", "code"),
                    ("grant_type", "refresh_token"),
                    ("refresh_token", refreshToken)
                ])
                |> Js.Dict.map([@bs] ((s) => Js.Json.string(s)))
                |> Js.Json.object_
                |> _accessTokenApiCall;
        });
};
