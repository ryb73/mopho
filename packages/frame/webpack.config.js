const path              = require("path"),
      CopyWebpackPlugin = require("copy-webpack-plugin"),
      webpack           = require("webpack");

function rel(relPath) {
    return path.resolve(__dirname, relPath)
}

module.exports = {
    name: "frame",

    entry: {
        index: rel("src/index.re"),
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
                        cwd: __dirname
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
            to: rel("html/css")
        }, {
            from: rel("node_modules/font-awesome/fonts"),
            to: rel("html/fonts")
        }, {
            from: rel("src/**/*.js"),
            to: rel("lib/js/[path][name].js")
        }])
    ],

    devtool: "source-map",
};
