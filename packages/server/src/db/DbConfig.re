[@autoserialize]
type config = {
    host: string,
    user: string,
    password: string,
    database: string
};

let config =
    switch (config__from_json(Config.get("mopho-db"))) {
        | Error(_) => Js.Exn.raiseError("Error loading mopho-db config")
        | Ok(c) => c
    };
