open Option.Infix;
open BatPervasives;

let testAffectedRows = (~expected=1, (result, _)) => {
    let affectedRows = Js.Json.decodeObject(result)
        >>= flip(Js.Dict.get, "affectedRows")
        >>= Js.Json.decodeNumber;

    switch affectedRows {
        | Some(actual) =>
            if (Js.Math.floor(actual) === expected) {
                ();
            } else {
                Js.Exn.raiseError("Affected rows " ++ Js.String.make(actual) ++ " <> " ++ Js.String.make(expected));
            }
        | _ => Js.Exn.raiseError("Affected rows empty")
    };
};