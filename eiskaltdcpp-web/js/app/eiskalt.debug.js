/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser: true */
/*global define */

define(
    ['jquery', 'app/eiskalt', 'config.js'],

    function ($, eiskalt, config) {
        'use strict';

        var my = {

            levels: {
                FATAL : 1,
                ERROR : 2,
                WARN : 3,
                INFO : 4,
                DEBUG : 5
            },

            print: function (debugLevel, text) {
                if (debugLevel <= config.debugLevel) {
                    $('#debugdiv').show();
                    $('#debugout').prepend((new Date()).toLocaleTimeString() + ': ' + text + '\n');
                }
            },

            onError: function (data) {
                my.print(my.levels.ERROR, 'Error: ' + data.error.message);
            },

            onSuccess: function (data) {
                my.print(my.levels.INFO, 'Success: ' + (data.result === 0));
            }
        };

        eiskalt.debug = my;
        return my;
    }
);
