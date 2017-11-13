const path              = require("path"),
      fs                = require("fs"),
      webpack           = require("webpack");

function rel(relPath) {
    return path.resolve(__dirname, relPath)
}

module.exports = {
    name: "api-server",

    entry: {
        index: rel("src/index.re"),
    },

    output: {
        path: "/dev/",
        filename: "null",
    },

    module: {
        noParse: /node_modules/,
        rules: [{
            test: /\.(re|ml)$/,
            use: {
                loader: "bs-loader",
                options: {
                    cwd: __dirname
                }
            }
        }]
    },

    resolve: {
        extensions: ['.re', '.ml', '.js'],
        alias: {
            express: rel("dummy.js"),
            mysql: rel("dummy.js"),
            browser: rel("dummy.js"),
        }
    },

    plugins: [
        new webpack.WatchIgnorePlugin([
            rel("node_modules/"),
            ...getSymlinkedDirs()
        ]),

        {
            apply: (compiler) => {
                compiler.plugin("invalid", (fileName) => {
                    console.log("File changed: " + path.relative(rel(".."), fileName));
                });
            }
        }
    ],

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
