const path              = require("path"),
      ExtractTextPlugin = require("extract-text-webpack-plugin");

function rel(relPath) {
    return path.resolve(__dirname, relPath);
}

module.exports = [
    require("./packages/rest-helper/webpack.config"),
    require("./packages/api-declarations/webpack.config"),
    require("./packages/db/webpack.config"),
    require("./packages/api-server/webpack.config"),
    require("./packages/iframe-comm/webpack.config"),
    ...require("./packages/frame/webpack.config"),
    {
        name: "merlin",
        entry: {
            "tpl": rel("merlin.template.js")
        },

        output: {
            path: "/dev/",
            filename: "null"
        },

        module: {
            rules: [{
                test: /\.js$/,
                use: "babel-loader"
            }, {
                test: /\.merlin/,
                use: ExtractTextPlugin.extract({
                    use: "raw-loader"
                })
            }]
        },

        resolve: {
            extensions: [".js",".merlin"]
        },

        plugins: [
            new ExtractTextPlugin(".." + rel(".merlin")) // relative to /dev/ (ugh)
        ],

        stats: {
            assets: true,

            errors: true, // I'd like to set this to false but it's causing errors to be suppressed for other children
            warnings: false,
            timings: false,
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
    }
];