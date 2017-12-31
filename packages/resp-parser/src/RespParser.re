open Js.Result;

open ResultEx;

exception RestError(option(string));

let parse = (bodyParser, result) =>
  result
  |> mapError(Option.some)
  |> bind(
       (resp) =>
         switch (Falsy.to_opt(resp##error)) {
         | Some(error) => Error(Js.Json.stringifyAny(error))
         | None =>
           switch (Js.Nullable.to_opt(resp##body)) {
           | None => Error(None)
           | Some(body) => Ok(body)
           }
         }
     )
  |> bind(bodyParser);
