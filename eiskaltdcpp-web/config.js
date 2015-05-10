/*global define */
define([], function () {
    'use strict';
    return {
        "jsonrpc" : {
            "host" : "", // if empty it defaults to the host serving the script and falls back to localhost
            "port" : "3121"
        },
        "debugLevel" : 3,
        "userlinks": {
            "google": {
                "icon":  "http://www.google.com/favicon.ico",
                "url":  "https://www.google.de/search?q=%Filename%",
                "filter": function (text) {
                    return text.replace(/\.\w{2,}$/, '').replace(/[-._+]/g, ' ').replace(/\(.*\)/g, ' ').replace(/\[.*\]/g, ' ').replace(/\d{3,}.*/, '');
                }
            }
        }
    };
});
