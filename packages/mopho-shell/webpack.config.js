const path              = require("path"),
      CopyWebpackPlugin = require("copy-webpack-plugin"),
      webpack           = require("webpack");

module.exports = {
    entry: {
        index: "./lib/js/src/index.js",
    },

    output: {
        path: path.resolve("./html"),
        filename: "js/[name].js",
    },

    plugins: [
        new webpack.DefinePlugin({ "global.GENTLY": false }),

        new CopyWebpackPlugin([{
            from: "node_modules/font-awesome/css",
            to: "css"
        }, {
            from: "node_modules/font-awesome/fonts",
            to: "fonts"
        }, {
            from: "src/**/*.js",
            to: path.resolve("./lib/js/[path][name].js")
        }])
    ],

    devtool: "source-map",
};
