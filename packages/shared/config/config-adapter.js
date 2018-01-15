const config = require("./config-out.json");

module.exports = {
    get(s) {
        return config[s];
    }
};