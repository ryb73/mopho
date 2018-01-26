module User = {
    [@autoserialize] type id = int;

    [@autoserialize]
    type t = {
        id: id,
        name: string
    };
};

[@autoserialize]
type metadataSource =
    | Napster;

module Album = {
    [@autoserialize]
    type t = {
        id: int,
        name: string,
        napsterId: option(string),
        metadataSource: metadataSource,
        primaryArtistId: int
    };
};

module Artist = {
    [@autoserialize]
    type t = {
        id: int,
        name: string,
        napsterId: option(string),
        metadataSource: metadataSource
    };
};

module Track = {
    [@autoserialize]
    type t = {
        id: int,
        name: string,
        napsterId: option(string),
        metadataSource: metadataSource,
        albumId: int,
        primaryArtistId: int
    };
};

module SearchResults = {
    [@autoserialize]
    type t =
        | Track(Track.t);
};