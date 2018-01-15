module User = {
    [@autoserialize]
    type t = {
        id: int,
        name: string
    };
};
