open Js.Promise;

let parseResponse (err, res) => {
    resolve @@ switch res {
        | None => switch err {
            | None => `UnkownError
            | Some str => `NoResponse str
        }

        | Some res => {
            switch (Falsy.to_opt res##error) {
                | Some error => `Error error
                | None => {
                    switch (Js.Null.to_opt res##body) {
                        | None => `NoBody
                        | Some body => `Success body
                    };
                }
            };
        }
    };
};