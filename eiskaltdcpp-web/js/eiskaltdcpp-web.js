/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser:true */
/*global require, eiskalt */

require.config({
    baseUrl: 'js/lib',
    paths: {
        app: '../app'
    },
    shim: {
        'jquery.tablesorter.min': ['jquery.min'],
        'jquery.easytabs.min': ['jquery.min', 'jquery.hashchange.min'],
        'jquery.hashchange.min': ['jquery.min'],
        'jquery.plugin': ['jquery.min'],
        'jquery.timer': ['jquery.min'],
        'jquery.jsonrpc': ['jquery.min'],
        'eiskalt.search': ['jquery.min', 'config.js', 'app/eiskalt'],
        'eiskalt.queue': ['jquery.min', 'app/eiskalt'],
        'eiskalt': ['jquery.min', 'config.js']
    }
});

require(
    [
        'jquery.min',
        'jquery.tablesorter.min',
        'jquery.hashchange.min',
        'jquery.easytabs.min',
        'jquery.jsonrpc',
        'jquery.plugin',
        'jquery.timer',
        'config.js',
        'app/eiskalt',
        'app/eiskalt.search',
        'app/eiskalt.queue'
    ],
    function () {
        'use strict';
        eiskalt.onLoad();
    }
);
