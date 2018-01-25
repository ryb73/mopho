open BsNapsterApi;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let matchArtist = ({ Types.Artist.id: napsterId, name }) => {
    DbArtist.findByNapsterId(napsterId)
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None =>
                DbArtist.findByName(name)
                    |> tapMaybe(DbArtist.setNapsterId(Some(napsterId)))
        )
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None => DbArtist.createFromNapster(name, napsterId)
                |> map(Option.some)
        )
};

let matchAlbum = (album) => {
    let { Types.Album.id: napsterId, name } = album;

    DbAlbum.findByNapsterId(napsterId)
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None =>
                DbAlbum.findByName(name)
                    |> tapMaybe(DbAlbum.setNapsterId(Some(napsterId)))
        )
        |> then_(fun
            | Some(a) => resolve(Some(a))
            | None => DbAlbum.createFromNapster(album)
                |> map(Option.some)
        )
};