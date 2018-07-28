[@decco]
type config = {
    napsterApiKey: string,
    apiUrl: string
};

let config =
    switch (Config.get("mopho-frame") |> config_decode) {
        | Error(e) => Js.log(e); Js.Exn.raiseError("Error loading mopho-frame config")
        | Ok(c) => c
    };
