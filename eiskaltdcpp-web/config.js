/*global define */
define([], function () {
    'use strict';
    return {
        "jsonrpc" : {
            "host" : "127.0.0.1",
            "port" : "3121"
        },
        "debugLevel" : 5,
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
