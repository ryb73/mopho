[@autoserialize]
type config = {
    napsterApiKey: string,
    apiUrl: string
};

let config =
    switch (Config.get("mopho-frame") |> config__from_json) {
        | Error(Some(s)) => Js.Exn.raiseError("Error loading mopho-frame config (" ++ s ++ ")")
        | Error(_) => Js.Exn.raiseError("Error loading mopho-frame config")
        | Ok(c) => c
    };