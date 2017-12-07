open NodeEx;
open PromiseEx;
open Express;

exception JsException exn;

let generateRandomBase64 ()  => {
    Js.Promise.make @@ fun ::resolve ::reject => {
        Crypto.randomBytes 16 (fun result => {
            switch result {
                | Error e => reject (JsException e) [@bs]
                | Ok buffer => resolve (Base64Url.fromBuffer buffer) [@bs]
            };
        });
    };
};

let _scryptParams = lazy (Scrypt.paramsSync 0.5 ());
let secureHash salt key => {
    Scrypt.hash key (Lazy.force _scryptParams) 64 salt
        |> map Base64Url.fromBuffer;
};

let getIp req => Request.ip req;