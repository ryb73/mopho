type config = {
  host: string,
  user: string,
  password: string,
  database: string
};

let config =
  switch (Config.get("mopho-db") |> config__from_json) {
  | Error(_) => failwith("Error loading mopho-db config")
  | Ok(c) => c
  };
