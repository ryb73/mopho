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

module Artist = {
    [@autoserialize] type id = int;
    [@autoserialize] type napsterId = string;

    [@autoserialize]
    type t = {
        id: id,
        name: string,
        napsterId: option(napsterId),
        metadataSource: metadataSource
    };
};

module Album = {
    [@autoserialize] type id = int;
    [@autoserialize] type napsterId = string;

    [@autoserialize]
    type t = {
        id: id,
        name: string,
        napsterId: option(napsterId),
        metadataSource: metadataSource,
        primaryArtistId: Artist.id
    };
};

module Track = {
    [@autoserialize] type id = int;
    [@autoserialize] type napsterId = string;

    [@autoserialize]
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
    [@autoserialize]
    type t =
        | Track(Track.t);
};