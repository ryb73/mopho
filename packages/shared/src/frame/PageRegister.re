open ReactStd;

let getPage = (type a, key: pageKey(a)) : page(a) =>
    switch key {
        | HomePage => (module HomePage : Page with type dynamicProps = a)
        | SearchPage => (module SearchPage : Page with type dynamicProps = a)
    };