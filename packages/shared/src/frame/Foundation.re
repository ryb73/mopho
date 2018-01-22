open FrameConfig;
open Bluebird;
open ReactStd;

module BluebirdEx = PromiseEx.Make(Bluebird);
open BluebirdEx;

let s2e = ReasonReact.stringToElement;

module type Component = {
    let make: 'a => ReasonReact.componentSpec(
        ReasonReact.stateless,
        ReasonReact.stateless,
        ReasonReact.noRetainedProps,
        ReasonReact.noRetainedProps,
        ReasonReact.actionless
    );
};

type state = option(Apis.Search_impl.resp);

type action =
    | SetSearchResults(Apis.Search_impl.resp);

let a: (module Component) = (module MainPane);

let component = ReasonReact.reducerComponent("Foundation");

NapsterPlayer.on(Error, (error) => Js.log2("Error:", error));

let logOut = (_) => {
    Apis.LogOut.request(config.apiUrl, ())
        |> map(() => Js.log("logged out"))
        |> catch((exn) => {
            Js.log2("logout error", exn);
            resolve()
        });

    ();
};

let testSearch = (_, { ReasonReact.reduce }) => {
    Apis.Search.request(config.apiUrl, "mitski")
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

let make = (_) => {
    ...component,

    render: (self) => {
        let { ReasonReact.handle } = self;

        <div className="foundation">
            <div className="top-bar"> <input _type="text" placeholder="Search" /> </div>
            <div className="center-bar">
                <div className="left-pane">
                    (s2e("Leftbar"))
                    <a href="#" onClick=logOut> (s2e("Log Out")) </a>
                    <div>
                        <button onClick=(handle(testSearch))>(s2e("Search"))</button>
                        (renderSearchResults(self))
                    </div>
                </div>
                <MainPane />
            </div>
            <div className="bottom-bar"> (s2e("Bottom")) </div>
        </div>
    },

    initialState: () => None,

    reducer: (action, _) =>
        switch action {
            | SetSearchResults(res) => Update(Some(res))
        }
};
