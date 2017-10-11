type napsterConfig = {
    apiKey: string,
    secret: string
};

type config = {
    secure: bool,
    origin: string,
    sessionSecret: string,
    napster: napsterConfig
};
let config = Config.get "mopho-api-server"
    |> config__from_json;

let config = switch config {
    | Some c => c
    | None => Js.Exn.raiseError "Invalid config"
};