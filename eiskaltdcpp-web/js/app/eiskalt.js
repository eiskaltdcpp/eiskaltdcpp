/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser:true */
/*global define */

define(
    ['jquery', 'config.js', 'jquery.tablesorter', 'jquery.timer', 'jquery.easytabs', 'jquery.jsonrpc'],

    function ($, config) {
        'use strict';

        var my = {

            debugLevels: {
                FATAL : 1,
                ERROR : 2,
                WARN : 3,
                INFO : 4,
                DEBUG : 5
            },

            connectedHubs: [],
            statisticalData: [],

            debugOut: function (debugLevel, text) {
                if (debugLevel <= config.debugLevel) {
                    $('#debugdiv').show();
                    $('#debugout').prepend((new Date()).toLocaleTimeString() + ': ' + text + '\n');
                }
            },

            onError: function (data) {
                my.debugOut(my.debugLevels.ERROR, 'Error: ' + data.error.message);
            },

            onSuccess: function (data) {
                my.debugOut(my.debugLevels.INFO, 'Success: ' + (data.result === 0));
            },

            showConnectionStatus: function (error, message) {
                if (error) {
                    $('#connectedhubs').attr('class', 'error');
                    $('#connectedhubs').text(message);
                    $('#statisticaldata').attr('class', 'error');
                    $('#statisticaldata').text(message);
                    $('#connectionerror').attr('class', 'error');
                    $('#connectionerror').text(message);
                } else {
                    $('#connectedhubs').attr('class', '');
                    $('#connectedhubs').text(message);
                    $('#connectionerror').attr('class', 'hidden');
                    $('#connectionerror').text('');
                }
            },

            updateConnectedHubs: function (data) {
                //my.debugOut(my.debugLevels.DEBUG, 'updateConnectedHubs: ' + data.result);
                var connectedHubs = data.result.split(';').filter(function (x) { return (x !== ''); }).sort();
                if (my.connectedHubs !== connectedHubs) {
                    my.connectedHubs = connectedHubs;
                    if (connectedHubs.length === 0) {
                        my.showConnectionStatus(true, 'Not connected to any hubs!');
                    } else {
                        my.showConnectionStatus(false, connectedHubs);
                    }
                }
            },

            errorConnectedHubs: function (data) {
                my.connectedHubs = [];
                my.showConnectionStatus(true, 'Connection to EiskaltDC++ daemon failed on ' + 'http://' + config.jsonrpc.host + ':' + config.jsonrpc.port);
            },

            requestConnectedHubs: function () {
                $.jsonRPC.request('hub.list', {
                    params : {'separator': ';'},
                    success : my.updateConnectedHubs,
                    error : my.errorConnectedHubs
                });
            },

            updateStatisticalData: function (data) {
                var statisticalData = data.result;
                if (my.statisticalData !== statisticalData) {
                    my.statisticalData = statisticalData;
                    $('#statisticaldata').attr('class', '');
                    $('#statisticaldata').html('Downloaded: ' + statisticalData.down + '<br>Uploaded: ' + statisticalData.up + '<br>Ratio: ' + statisticalData.ratio);
                }
            },

            requestStatisticalData: function () {
                $.jsonRPC.request('show.ratio', {
                    success : my.updateStatisticalData
                });
            },

            getURLParameter: function (name) {
                return decodeURIComponent(
                    (new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.search) || ['', ''])[1].replace(/\+/g, '%20')
                ) || null;
            },

            onLoad: function () {
                $('#tab-container').easytabs();

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

                $('#connectedhubs').timer({
                    callback: my.requestConnectedHubs,
                    delay: 1000,
                    repeat: true
                });
                $('#statisticaldata').timer({
                    callback: my.requestStatisticalData,
                    delay: 3000,
                    repeat: true
                });
            }
        };

        return my;
    }
);
