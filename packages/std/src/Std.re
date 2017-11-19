open NodeEx;

exception JsException Js.Exn.t;

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

let _scryptParams = Scrypt.paramsSync 0.5 ();
let secureHash salt key => Scrypt.hash key _scryptParams 64 salt;