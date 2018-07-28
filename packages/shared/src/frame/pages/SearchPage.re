open FrameConfig;
open ReactStd;
open Bluebird;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

type dynamicProps = string;
type retainedProps = ReasonReact.noRetainedProps;
type state = option(Apis.Search_impl.resp);
type action = Apis.Search_impl.resp;

let goBack = ({ Context.navigate }, _) => {
    navigate(HomePage, ());
};

let doSearch = ({ ReasonReact.send }, query) => {
    Apis.Search.request(config.apiUrl, query)
        |> map(send)
        |> catch((exn) => {
            Js.log2("search error", exn);
            resolve();
        });

    <span>
        (s2e("searching..."))
    </span>
};

let renderArtist = ({ Models.Artist.id, name }) =>
    <li key=(string_of_int(id))>
        (s2e({j|$id – $name|j}))
    </li>;

let renderAlbum = ({ Models.Album.id, name, primaryArtistId}) =>
    <li key=(string_of_int(id))>
        (s2e({j|$id – $name ($primaryArtistId)|j}))
    </li>;

let renderTrack = (
    { Context.playTrack },
    { Models.Track.id, name, primaryArtistId, albumId } as track
) =>
    <li key=(string_of_int(id))>
        <a href="#" onClick={(_) => playTrack(track)}>
            (s2e({j|$id – $name ($primaryArtistId – $albumId)|j}))
        </a>
    </li>;

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
                        (ReasonReact.array(Js.Array.map(renderArtist, results.Apis.Search_impl.artists)))
                    </ul>

                    <h2>(s2e("Albums"))</h2>
                    <ul>
                        (ReasonReact.array(Js.Array.map(renderAlbum, results.albums)))
                    </ul>

                    <h2>(s2e("Tracks"))</h2>
                    <ul>
                        (ReasonReact.array(Js.Array.map(renderTrack(context), results.tracks)))
                    </ul>
                </div>
        };
    },

    initialState: () => None,

    reducer: (results, _) => ReasonReact.Update(Some(results))
};