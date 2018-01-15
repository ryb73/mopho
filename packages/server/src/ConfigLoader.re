open Js.Result;

[@autoserialize]
type napsterConfig = {
    apiKey: string,
    secret: string
};

[@autoserialize]
type config = {
    secure: bool,
    origin: string,
    sessionSecret: string,
    napster: napsterConfig
};

let config = Config.get("mopho-api-server") |> config__from_json;

let config =
    switch config {
        | Ok(c) => c
        | Error(Some(key)) => Js.Exn.raiseError("Error loading mopho-api-server config: " ++ key)
        | Error(_) => Js.Exn.raiseError("Error loading mopho-api-server config")
    };
