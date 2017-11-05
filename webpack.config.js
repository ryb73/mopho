module.exports = [
    require("./packages/rest-helper/webpack.config"),
    require("./packages/api-declarations/webpack.config"),
    require("./packages/api-server/webpack.config"),
    require("./packages/iframe-comm/webpack.config"),
    ...require("./packages/frame/webpack.config"),
];