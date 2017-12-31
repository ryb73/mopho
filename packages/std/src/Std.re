open NodeEx;

open PromiseEx;

open Express;

exception JsException(exn);

let generateRandomBase64 = () =>
  Js.Promise.make @@
  (
    (~resolve, ~reject) =>
      Crypto.randomBytes(
        16,
        (result) =>
          switch result {
          | Error(e) => [@bs] reject(JsException(e))
          | Ok(buffer) => [@bs] resolve(Base64Url.fromBuffer(buffer))
          }
      )
  );

let _scryptParams = lazy (Scrypt.paramsSync(0.5, ()));

let secureHash = (salt, key) =>
  Scrypt.hash(key, Lazy.force(_scryptParams), 64, salt) |> map(Base64Url.fromBuffer);

let getIp = (req) => Request.ip(req);
