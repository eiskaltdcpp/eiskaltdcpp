/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser: true */
/*global require */

require.config({
    baseUrl: 'js/lib',
    paths: {
        app: '../app'
    },
    shim: {
        'jquery.tablesorter': ['jquery'],
        'jquery.easytabs': ['jquery', 'jquery.hashchange'],
        'jquery.hashchange': ['jquery'],
        'jquery.plugin': ['jquery'],
        'jquery.timer': ['jquery', 'jquery.plugin'],
        'jquery.jsonrpc': ['jquery']
    }
});

require(
    [
        'app/eiskalt',
        'app/eiskalt.debug',
        'app/eiskalt.status',
        'app/eiskalt.search',
        'app/eiskalt.queue'
    ],
    function (eiskalt) {
        'use strict';
        eiskalt.onLoad();
        eiskalt.status.onLoad();
        eiskalt.search.onLoad();
        eiskalt.queue.onLoad();
    }
);
