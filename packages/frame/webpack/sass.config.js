const path              = require("path"),
      ExtractTextPlugin = require("extract-text-webpack-plugin");

function rel(relPath) {
    return path.resolve(__dirname, "../" + relPath)
}

module.exports = {
    name: "frame-css",

    entry:  {
        index: rel("scss/index.scss"),
    },

    output: {
        path: rel("html/css"),
        filename: "[name].css",
    },

    module: {
        rules: [{
            test: /\.scss?$/,
            use: ExtractTextPlugin.extract({
                use: ["css-loader", "sass-loader"]
            }),
        }],
    },

    plugins: [
        new ExtractTextPlugin("[name].css")
    ],

    devtool: "source-map",
};
