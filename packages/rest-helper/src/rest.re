open Js.Promise;

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
                    | None => `InvalidBody body
                    | Some parsedBody => `Success parsedBody
                }
            }
        }
    };
};