open Js.Promise;
open Js.Result;

let parseResponse bodyParser result => {
    resolve @@ switch result {
        | Error err => `NoResponse err
        | Ok resp => switch (Falsy.to_opt resp##error) {
            | Some error => `Error error
            | None => switch (Js.Nullable.to_opt resp##body) {
                | None => `NoBody
                | Some body => switch (bodyParser body) {
                    | Error optKey => `InvalidBody (body, optKey)
                    | Ok parsedBody => `Success parsedBody
                }
            }
        }
    };
};