type config = {
    host: string,
    user: string,
    password: string,
    database: string
};

let config =
    switch (config__from_json(Config.get("mopho-db"))) {
        | Error(_) => failwith("Error loading mopho-db config")
        | Ok(c) => c
    };
