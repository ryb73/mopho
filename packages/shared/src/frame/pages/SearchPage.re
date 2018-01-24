open FrameConfig;
open ReactStd;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

type context = Context.t;
type dynamicProps = string;
type retainedProps = ReasonReact.noRetainedProps;
type state = option(Apis.Search_impl.resp);
type action = Apis.Search_impl.resp;

let goBack = ({ Context.navigate, getPage }, _) => {
    navv(navigate, getPage(ReactStd.HomePage), ());
    ();
};

let doSearch = ({ ReasonReact.reduce }, query) => {
    Apis.Search.request(config.apiUrl, query)
        |> map(go(reduce))
        |> catch((exn) => {
            Js.log2("search error", exn);
            resolve();
        });

    <span>
        (s2e("searching..."))
    </span>
};

let renderArtist = (artist) =>
    <li key=(string_of_int(artist.Models.Artist.id))>(s2e(artist.name))</li>;

let renderAlbum = (album) =>
    <li key=(string_of_int(album.Models.Album.id))>(s2e(album.name))</li>;

let renderTrack = (track) =>
    <li key=(string_of_int(track.Models.Track.id))>(s2e(track.name))</li>;

let component = ReasonReact.reducerComponent("SearchPage");
let make = (~dynamicProps as query, ~context, _) => {
    ...component,

    render: (self) => {
        let { ReasonReact.state } = self;

        switch state {
            | None => doSearch(self, query)
            | Some(results) =>
                <div>
                    <button onClick=(goBack(context))>(s2e("back"))</button>

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