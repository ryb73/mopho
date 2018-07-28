open BsNode;
open Express;
open Bluebird;
open MomentRe;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let (%>) = BatPervasives.(%>);

[@bs.new] external makeJsExn : exn => exn = "Error";

exception JsException(exn);

let generateRandomBase64 = () =>
    Bluebird.make((~resolve, ~reject) =>
          NodeCrypto.randomBytes(16, (result) =>
              switch result {
                  | Error(e) => reject(JsException(e))
                  | Ok(buffer) => resolve(Base64Url.fromBuffer(buffer))
              }
          )
    );

let _scryptParams = Scrypt.paramsSync(0.5, ());

let secureHash = (salt, key) => {
    Scrypt.hash(key, _scryptParams, 64, salt)
        |> fromPromise
        |> catch(makeJsExn %> reject)
        |> map(Base64Url.fromBuffer);
};

let getIp = (req) => Request.ip(req);

let getCurrentUtc = () => momentUtc() |> Moment.defaultFormat;