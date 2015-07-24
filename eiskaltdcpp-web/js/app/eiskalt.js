/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser: true */
/*global define */

define(
    ['jquery', 'config.js', 'jquery.tablesorter', 'jquery.easytabs', 'jquery.jsonrpc'],

    function ($, config) {
        'use strict';

        var my = {

            getURLParameter: function (name) {
                return decodeURIComponent(
                    (new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.search) || ['', ''])[1].replace(/\+/g, '%20')
                ) || null;
            },

            onLoad: function () {
                $('#tab-container').easytabs();

                // determine jsonrpc host in this order: config -> http host -> localhost
                if (!config.jsonrpc.host) {
                    config.jsonrpc.host = location.hostname;
                    if (!config.jsonrpc.host) {
                        config.jsonrpc.host = 'localhost';
                    }
                }

                $.jsonRPC.setup({
                    endPoint : 'http://' + config.jsonrpc.host + ':' + config.jsonrpc.port,
                    namespace : ''
                });

                $.tablesorter.addParser({
                    id: 'filesize',
                    is: function (s) {
                        return s.match(new RegExp(/[0-9]+(\.[0-9]+)?\s*(K|M|G|T|P|E|Z|Y)?i?B/i));
                    },
                    format: function (s) {
                        var parts, suffix, num, exponent = 0;
                        parts = s.match(new RegExp(/([0-9]+(\.[0-9]+)?)\s*((K|M|G|T|P|E|Z|Y)?i?B)?/i));
                        if (parts === null) {
                            return 0;
                        }
                        suffix = parts[3];
                        if (suffix !== undefined) {
                            exponent = Math.max(0, 'bkmgtpey'.indexOf(suffix[0].toLowerCase()));
                        }
                        num = parseFloat(parts[1]);
                        return num * Math.pow(1024, exponent, 0);
                    },
                    type: 'numeric'
                });
            }
        };

        return my;
    }
);
