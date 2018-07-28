module User = {
    [@decco] type id = int;

    [@decco]
    type t = {
        id: id,
        name: string
    };
};

[@decco]
type metadataSource =
    | Napster;

module Artist = {
    [@decco] type id = int;
    [@decco] type napsterId = string;

    [@decco]
    type t = {
        id: id,
        name: string,
        napsterId: option(napsterId),
        metadataSource: metadataSource
    };
};

module Album = {
    [@decco] type id = int;
    [@decco] type napsterId = string;

    [@decco]
    type t = {
        id: id,
        name: string,
        napsterId: option(napsterId),
        metadataSource: metadataSource,
        primaryArtistId: Artist.id
    };
};

module Track = {
    [@decco] type id = int;
    [@decco] type napsterId = string;

    [@decco]
    type t = {
        id: id,
        name: string,
        napsterId: option(napsterId),
        metadataSource: metadataSource,
        albumId: Album.id,
        primaryArtistId: Artist.id
    };
};

module SearchResults = {
    [@decco]
    type t =
        | Track(Track.t);
};