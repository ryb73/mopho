const path              = require("path"),
      webpack           = require("webpack");

function rel(relPath) {
    return path.resolve(__dirname, relPath)
}

module.exports = {
    name: "iframe-comm",

    entry: {
        IframeComm: rel("src/IframeComm.re"),
    },

    output: {
        path: "/dev/",
        filename: "null",
    },

    module: {
        noParse: /node_modules/,
        rules: [
            {
                test: /\.(re|ml)$/,
                use: {
                    loader: "bs-loader",
                    options: {
                        cwd: __dirname
                    }
                }
            },
        ]
    },

    resolve: {
        extensions: ['.re', '.ml', '.js'],
    },

    stats: {
        errors: true,
        errorDetails: true,
        warnings: true,
        colors: true,
        timings: true,

        assets: false,
        cached: false,
        cachedAssets: false,
        children: false,
        chunks: false,
        depth: false,
        entrypoints: false,
        hash: false,
        modules: false,
        performance: false,
        providedExports: false,
        publicPath: false,
        reasons: false,
        source: false,
        usedExports: false,
        version: false,
    }
};
