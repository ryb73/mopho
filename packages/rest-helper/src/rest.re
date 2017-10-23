open Js.Promise;
open Js.Result;

let parseResponse bodyParser (err, res) => {
    resolve @@ switch res {
        | None => switch err {
            | None => `UnknownError
            | Some str => `NoResponse str
        }

        | Some res => switch (Falsy.to_opt res##error) {
            | Some error => `Error error
            | None => switch (Js.Nullable.to_opt res##body) {
                | None => `NoBody
                | Some body => switch (bodyParser body) {
                    | Error optKey => `InvalidBody (body, optKey)
                    | Ok parsedBody => `Success parsedBody
                }
            }
        }
    };
};