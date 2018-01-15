const path              = require("path"),
      ExtractTextPlugin = require("extract-text-webpack-plugin"),
      fs                = require("fs"),
      webpack           = require("webpack");

function rel(relPath) {
    return path.resolve(__dirname, "../" + relPath)
}

module.exports = {
    name: "shared-css",

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
        new webpack.WatchIgnorePlugin([
            rel("node_modules/"),
            ...getSymlinkedDirs()
        ]),

        new ExtractTextPlugin("[name].css"),
        {
            apply: (compiler) => {
                compiler.plugin("invalid", (fileName) => {
                    console.log("File changed: " + path.relative(rel("../../"), fileName));
                });
            }
        }
    ],

    devtool: "source-map",
};

function getSymlinkedDirs(nodeModulesPath) {
    if(!nodeModulesPath)
        return getSymlinkedDirs(rel("node_modules/"))
            .filter((pathname) => {
                return path.relative(rel("../.."), pathname).indexOf("..") >= 0;
            });

    if(!fs.existsSync(nodeModulesPath))
        return [];

    return fs.readdirSync(nodeModulesPath)
        .map((dirName) => path.join(nodeModulesPath, dirName))
        .filter((path) => fs.lstatSync(path).isSymbolicLink())
        .map((path) => fs.realpathSync(path))
        .reduce((previousPaths, pathname) => {
            return [ ...previousPaths, pathname, ...getSymlinkedDirs(path.join(pathname, "node_modules/")) ];
        }, []);
}
