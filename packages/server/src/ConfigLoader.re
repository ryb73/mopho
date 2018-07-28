open Belt.Result;

[@decco]
type napsterConfig = {
    apiKey: string,
    secret: string
};

[@decco]
type config = {
    secure: bool,
    origin: string,
    sessionSecret: string,
    napster: napsterConfig
};

let config = Config.get("mopho-api-server") |> config_decode;

let config =
    switch config {
        | Ok(c) => c
        | Error(e) => Js.log(e); Js.Exn.raiseError("Error loading mopho-api-server config")
    };
