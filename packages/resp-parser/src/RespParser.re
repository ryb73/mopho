open Superagent;
open Option.Infix;

exception RestError(option(string));

let parse = (bodyParser, resp) =>
    switch (resp.error) {
        | Some(error) => Js.Exn.raiseError(Js.Json.stringifyAny(error) |? "Unknown error")
        | None =>
            switch (resp.body) {
                | None => Js.Exn.raiseError("No response body")
                | Some(body) => bodyParser(body)
            }
    };
