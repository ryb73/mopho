const path              = require("path"),
      CopyWebpackPlugin = require("copy-webpack-plugin"),
      fs                = require("fs"),
      webpack           = require("webpack");

function rel(relPath) {
    return path.resolve(__dirname, "../" + relPath)
}

module.exports = {
    name: "frame-bs",

    entry: {
        index: rel("src/index.re"),
        "napster-auth": rel("src/NapsterAuth.re"),
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
                        cwd: rel(""),
                        excludedWarnings: [ /.*Duplicated package:.+\n?/g ]
                    }
                }
            },
        ]
    },

    resolve: {
        extensions: ['.re', '.ml', '.js'],
    },

    plugins: [
        new webpack.WatchIgnorePlugin([
            rel("node_modules/"),
            ...getSymlinkedDirs()
        ]),

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
        }]),

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
