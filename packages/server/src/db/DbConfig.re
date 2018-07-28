[@decco]
type config = {
    host: string,
    user: string,
    password: string,
    database: string
};

let config =
    switch (config_decode(Config.get("mopho-db"))) {
        | Error(e) => Js.log(e); Js.Exn.raiseError("Error loading mopho-db config")
        | Ok(c) => c
    };
