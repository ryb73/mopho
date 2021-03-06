open Express;
open Bluebird;
open Belt.Result;
open Option.Infix;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let flip = BatPervasives.flip;

type allowedMethods =
    | Get
    | Post;

module type Definition = {
    [@decco] type req;
    [@decco] type resp;
    let path: string;
    let reqMethod: allowedMethods;
};

let returnCode = (code, resp) => Response.status(resp, code) |> Response.end_;

module type Endpoint = {
    type resp;
    type req;

    type handlerResult =
        | Result(resp)
        | ErrorCode(int)
        | ExpressAction(done_);

    let handle:
      (App.t, (Request.t, Response.t, Middleware.next, req) => Bluebird.t(handlerResult)) => unit;
    let request: (string, req) => Bluebird.t(resp);
};

module Endpoint = (Definition: Definition) => {
    type resp = Definition.resp;
    type req = Definition.req;

    let _tryOpt = (f, v) =>
        try (Some(f(v))) {
            | _ => None
        };

    let _parseGetReq = (req) => req
        |> Request.query
        |> flip(Js.Dict.get, "json") |? Js.Json.null
        |> Js.Json.decodeString
        |> flip(Option.bind) @@ _tryOpt(Js.Json.parseExn) |? Js.Json.null
        |> Definition.req_decode;

    let _parsePostReq = (req) => req
        |> Request.asJsonObject
        |> flip(Js.Dict.get, "body") |? Js.Json.null
        |> Definition.req_decode;

    type handlerResult =
        | Result(Definition.resp)
        | ErrorCode(int)
        | ExpressAction(done_);

    let handle = (app, callback) => {
        let (methodFunc, reqParser) =
            switch Definition.reqMethod {
                | Get => (App.get, _parseGetReq)
                | Post => (App.post, _parsePostReq)
            };

        methodFunc(app, ~path=Definition.path, Middleware.fromAsync((req, resp, next) =>
            switch (reqParser(req)) {
                | Error(_) => resolve(returnCode(400, resp))

                | Ok(data) =>
                    callback(req, resp, next, data)
                        |> BluebirdEx.map((result) =>
                            switch result {
                                | ExpressAction(a) => a
                                | ErrorCode(code) => returnCode(code, resp)
                                | Result(r) => Definition.resp_encode(r) |> Response.sendJson(resp)
                            }
                        )
                        |> catch((err) => {
                            Js.log("Error in " ++ Definition.path ++ ":");
                            Js.log(err);
                            resolve(returnCode(500, resp))
                        })
            }
            |> toPromise
        ));
    };

    let _queryJson = (json, req) => {
        let encoded = Js.Json.stringify(json);
        let dict = Js.Dict.fromList([("json", encoded)]);
        Superagent.Get.query(dict, req)
    };

    let _doGet = (apiUrl, data) =>
        apiUrl
            |> Superagent.get
            |> Superagent.Get.withCredentials
            |> _queryJson(Definition.req_encode(data))
            |> Superagent.Get.end_;

    let _doPost = (apiUrl, data) =>
        apiUrl
            |> Superagent.post
            |> Superagent.Post.withCredentials
            |> Superagent.Post.send(Definition.req_encode(data))
            |> Superagent.Post.end_;

    let request = (apiUrlBase, data) => {
        let reqMethod =
            switch Definition.reqMethod {
                | Post => _doPost
                | Get => _doGet
            };

        reqMethod(apiUrlBase ++ Definition.path, data)
            |> BluebirdEx.map(RespParser.parse(Definition.resp_decode))
            |> unwrapResult;
    };
};
