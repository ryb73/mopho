open Express;
open Option;
open Js.Promise;

let flip = BatPervasives.flip;

type allowedMethods =
    | Get
    | Post;

external encodeUriComponent : string => string = "encodeURIComponent" [@@bs.val];

module type Definition = {
    type req;
    type resp;

    let path : string;
    let reqMethod : allowedMethods;
};

let return400 resp => Response.status resp 400
    |> Response.end_
    |> resolve;

module Endpoint = fun (Definition : Definition) => {
    type handlerResult =
      | Result Definition.resp
      | ExpressAction done_
      [@@noserialize];

    let _handle (expressMethod : App.t => path::string => Middleware.t => unit) (app : App.t) callback =>
        expressMethod app path::Definition.path @@ Middleware.fromAsync (fun req resp next => {
            let data = req
                |> Request.asJsonObject
                |> flip Js.Dict.get "body" |? Js.Json.null
                |> Definition.req__from_json;

            switch data {
                | None => return400 resp
                | Some data =>
                    callback req resp next data
                        |> then_ (fun result => {
                            switch result {
                                | ExpressAction a => resolve a
                                | Result r =>
                                    Definition.resp__to_json r
                                        |> Response.sendJson resp
                                        |> resolve
                            };
                        });
            };
        });

    let handle app callback => {
        let methodFunc = switch Definition.reqMethod {
            | Get => App.get
            | Post => App.post
        };

        _handle methodFunc app callback;
    };

    let queryJson json req => {
        Js.Json.stringify json
            |> encodeUriComponent
            |> (fun encoded => {
                let dict = Js.Dict.fromList [("json", encoded)];
                Superagent.Get.query dict req;
            });
    };

    let _doGet apiUrl data => {
        apiUrl
            |> Superagent.get
            |> Superagent.Get.withCredentials
            |> queryJson @@ Definition.req__to_json data
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

    let request apiUrlBase data => {
        let reqMethod = switch Definition.reqMethod {
            | Post => _doPost
            | Get => _doGet
        };

        reqMethod (apiUrlBase ^ Definition.path) data
    };
};