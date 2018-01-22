open BsNapsterApi;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let matchArtist = ({ Types.Artist.id: napsterId, name }) => {
    Db.Artist.findByNapsterId(napsterId)
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None =>
                Db.Artist.findByName(name)
                    |> tapMaybe(Db.Artist.setNapsterId(Some(napsterId)))
        )
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None => Db.Artist.createFromNapster(name, napsterId)
                |> map(Option.some)
        )
};