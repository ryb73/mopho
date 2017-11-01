const path              = require("path"),
      CopyWebpackPlugin = require("copy-webpack-plugin"),
      webpack           = require("webpack");

function rel(relPath) {
    return path.resolve(__dirname, "../" + relPath)
}

module.exports = {
    name: "frame-bs",

    entry: {
        index: rel("src/index.re"),
        "napster-auth": rel("src/napsterAuth.re"),
    },

    output: {
        path: rel("html/js"),
        filename: "[name].js",
    },

    module: {
        rules: [
            {
                test: /\.(re|ml)$/,
                use: {
                    loader: "bs-loader",
                    options: {
                        cwd: rel("")
                    }
                }
            },
        ]
    },

    resolve: {
        extensions: ['.re', '.ml', '.js'],
    },

    plugins: [
        new webpack.DefinePlugin({ "global.GENTLY": false }),

        new CopyWebpackPlugin([{
            from: rel("node_modules/font-awesome/css"),
            to: rel("html/vendor")
        }, {
            from: rel("node_modules/font-awesome/fonts"),
            to: rel("html/vendor")
        }, {
            from: rel("node_modules/gridlex/docs/gridlex.css"),
            to: rel("html/vendor")
        }, {
            from: rel("src/**/*.js"),
            to: rel("lib/js/[path][name].js")
        }])
    ],

    devtool: "source-map",
};
