open ReactStd;

type dynamicProps = string;
type retainedProps = ReasonReact.noRetainedProps;
type state = option(Apis.Search_impl.resp);
type action = Apis.Search_impl.resp;

let renderArtist = (artist) =>
    <li key=(string_of_int(artist.Models.Artist.id))>(s2e(artist.name))</li>;

let renderAlbum = (album) =>
    <li key=(string_of_int(album.Models.Album.id))>(s2e(album.name))</li>;

let renderTrack = (track) =>
    <li key=(string_of_int(track.Models.Track.id))>(s2e(track.name))</li>;

let component = ReasonReact.reducerComponent("SearchPage");
let blah = (~dynamicProps: dynamicProps, ()) => ();
let make = (~dynamicProps: dynamicProps, ~context as _, _) => {
    ...component,

    render: ({ state }) => {
        switch state {
            | None => (s2e("searching..."))
            | Some(results) =>
                <div>
                    <h2>(s2e("Artists"))</h2>
                    <ul>
                        (ReasonReact.arrayToElement(Js.Array.map(renderArtist, results.Apis.Search_impl.artists)))
                    </ul>

                    <h2>(s2e("Albums"))</h2>
                    <ul>
                        (ReasonReact.arrayToElement(Js.Array.map(renderAlbum, results.albums)))
                    </ul>

                    <h2>(s2e("Tracks"))</h2>
                    <ul>
                        (ReasonReact.arrayToElement(Js.Array.map(renderTrack, results.tracks)))
                    </ul>
                </div>
        };
    },

    initialState: () => None,

    reducer: (results, _) => ReasonReact.Update(Some(results))
};