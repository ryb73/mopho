module User = {
    [@autoserialize] type id = int;

    [@autoserialize]
    type t = {
        id: id,
        name: string
    };
};

module Album = {
    [@autoserialize]
    type t = {
        id: int,
        name: string
    };
};

module Artist = {
    [@autoserialize]
    type t = {
        id: int,
        name: string,
        napsterId: option(string)
    };
};

module Track = {
    [@autoserialize]
    type t = {
        id: int,
        name: string
    };
};

module SearchResults = {
    [@autoserialize]
    type t =
        | Track(Track.t);
};