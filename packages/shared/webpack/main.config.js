process.env.SUPPRESS_NO_CONFIG_WARNING = true;

const path                  = require("path"),
      CopyWebpackPlugin     = require("copy-webpack-plugin"),
      fs                    = require("fs"),
      webpack               = require("webpack"),
      configModule          = require("config"),
      ClosureCompilerPlugin = require("webpack-closure-compiler");

function rel(relPath) {
    return path.resolve(__dirname, "../" + relPath)
}

const config = configModule.util.loadFileConfigs(rel("config/"));

fs.writeFileSync(rel("config/config-out.json"), JSON.stringify(config));

module.exports = {
    name: "shared-bs",

    entry: {
        index: rel("lib/js/src/frame/index.js"),
        "napster-auth": rel("lib/js/src/frame/NapsterAuth.js"),
    },

    output: {
        path: rel("html/js"),
        filename: "[name].js",
    },

    resolve: {
        alias: {
            config: rel("config/config-adapter"),
        }
    },

    plugins: [
        new webpack.WatchIgnorePlugin([
            rel("node_modules/"),
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
        },

        // new ClosureCompilerPlugin({
        //     compiler: {
        //         jar: rel("node_modules/google-closure-compiler/compiler.jar"),
        //         jscomp_off: ["suspiciousCode","nonStandardJsDocs"],
        //         compilation_level: "SIMPLE"
        //     }
        // })
    ],

    // devtool: "source-map",
};
