type t = Js.t {.
    data: Js.Json.t,
    origin: string,
    source: Dom.window
} [@@noserialize];