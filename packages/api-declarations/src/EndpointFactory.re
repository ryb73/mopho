open Express;
open Js.Promise;
open Js.Result;
open Option.Infix;
open PromiseEx;

let flip = BatPervasives.flip;

type allowedMethods =
    | Get
    | Post;

module type Definition = {
    type req;
    type resp;

    let path : string;
    let reqMethod : allowedMethods;
};

let returnCode code resp => Response.status resp code
    |> Response.end_;

module Endpoint = fun (Definition : Definition) => {
    let _tryOpt f v => {
        try (Some (f v)) {
            | _ => None
        };
    };

    let _parseGetReq req => {
        req
            |> Request.query
            |> flip Js.Dict.get "json" |? Js.Json.null
            |> Js.Json.decodeString
            |> flip Option.bind @@ _tryOpt Js.Json.parseExn |? Js.Json.null
            |> Definition.req__from_json;
    };

    let _parsePostReq req => {
        req
            |> Request.asJsonObject
            |> flip Js.Dict.get "body" |? Js.Json.null
            |> Definition.req__from_json;
    };

    type handlerResult =
      | Result Definition.resp
      | ErrorCode int
      | ExpressAction done_
      [@@noserialize];

    let _handle app callback => {
        let (methodFunc, reqParser) = switch Definition.reqMethod {
            | Get => (App.get, _parseGetReq)
            | Post => (App.post, _parsePostReq)
        };

        methodFunc app path::Definition.path @@ Middleware.fromAsync (fun req resp next => {
            switch (reqParser req) {
                | Error _ => resolve @@ returnCode 400 resp
                | Ok data =>
                    callback req resp next data
                        |> map (fun result => {
                            switch result {
                                | ExpressAction a => a
                                | ErrorCode code => returnCode code resp
                                | Result r =>
                                    Definition.resp__to_json r
                                        |> Response.sendJson resp
                            };
                        })
                        |> catch (fun err => {
                            Js.log2 ("Error in " ^ Definition.path ^ ":") err;
                            resolve @@ returnCode 500 resp;
                        });
            };
        });
    };

    let _queryJson json req => {
        let encoded = Js.Json.stringify json;
        let dict = Js.Dict.fromList [("json", encoded)];
        Superagent.Get.query dict req;
    };

    let _doGet apiUrl data => {
        apiUrl
            |> Superagent.get
            |> Superagent.Get.withCredentials
            |> _queryJson @@ Definition.req__to_json data
            |> Superagent.Get.end_
            |> then_ @@ Rest.parseResponse Definition.resp__from_json;
    };

    let _doPost apiUrl data => {
        apiUrl
            |> Superagent.post
            |> Superagent.Post.withCredentials
            |> Superagent.Post.send @@ Definition.req__to_json data
            |> Superagent.Post.end_
            |> then_ @@ Rest.parseResponse Definition.resp__from_json;
    };

    let _request apiUrlBase data => {
        let reqMethod = switch Definition.reqMethod {
            | Post => _doPost
            | Get => _doGet
        };

        reqMethod (apiUrlBase ^ Definition.path) data
    };

    let make () => (_handle, _request);
};