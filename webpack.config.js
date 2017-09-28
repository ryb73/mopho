const requireGlob = require("require-glob"),
      _           = require("lodash");

module.exports = _.map(
    Object.values(
        requireGlob.sync("./packages/*/webpack.config.js")
    ),
    "webpackConfig"
);