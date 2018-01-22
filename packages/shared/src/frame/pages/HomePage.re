open ReactStd;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

type retainedProps = ReasonReact.noRetainedProps;
type state = option(Apis.Search_impl.resp);
type action =
    | SetSearchResults(Apis.Search_impl.resp);

let logOut = ({ Context.apiUrl }, _) => {
    Apis.LogOut.request(apiUrl, ())
        |> map(() => Js.log("logged out"))
        |> catch((exn) => {
            Js.log2("logout error", exn);
            resolve()
        });

    ();
};

let testSearch = ({ Context.apiUrl }, _, { ReasonReact.reduce }) => {
    Apis.Search.request(apiUrl, "mitski")
        |> map(results => go(reduce, SetSearchResults(results)))
        |> catch((exn) => {
            Js.log2("search error", exn);
            resolve();
        })
        |> ignore;
};

let renderArtist = (artist) =>
    <li key=(string_of_int(artist.Models.Artist.id))>(s2e(artist.name))</li>;

let renderAlbum = (album) =>
    <li key=(string_of_int(album.Models.Album.id))>(s2e(album.name))</li>;

let renderTrack = (track) =>
    <li key=(string_of_int(track.Models.Track.id))>(s2e(track.name))</li>;

let renderSearchResults = ({ ReasonReact.state }) =>
    switch state {
        | None => ReasonReact.nullElement
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

let component = ReasonReact.reducerComponent("HomePage");

let make : ((~context: Context.t, array(unit))
=> ReasonReact.component(_, _, _)) = (~context, _) => {
    ...component,

    render: (self) => {
        let { ReasonReact.handle } = self;

        <div>
            <button onClick=(logOut(context))>(s2e("Log Out"))</button>
            <button onClick=(handle(testSearch(context)))>(s2e("Search"))</button>
            (renderSearchResults(self))
        </div>;
    },

    initialState: () : state => None,

    reducer: (action, _) => {
        switch action {
            | SetSearchResults(results) => ReasonReact.Update(Some(results))
        };
    }
};
