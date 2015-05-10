/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser: true */
/*global define */

define(
    ['jquery', 'app/eiskalt', 'config.js', 'jquery.tablesorter', 'jquery.timer', 'jquery.easytabs', 'jquery.jsonrpc'],

    function ($, eiskalt, config) {
        'use strict';

        var my = {

            connectedHubs: [],
            statisticalData: [],

            showConnectionStatus: function (error) {
                if (error) {
                    $('#connectionerror').attr('class', 'error');
                    $('#connectionerror').text('Connection to EiskaltDC++ daemon failed on ' + 'http://' + config.jsonrpc.host + ':' + config.jsonrpc.port);
                } else {
                    $('#connectionerror').attr('class', 'hidden');
                    $('#connectionerror').text('');
                }
            },

            addConnectedHub: function (hubAddress, entry) {
                var table, row, headers, image;
                table = $('table#connectedhubs');
                headers = table.find('th');
                if (!my.connectedHubs.hasOwnProperty(hubAddress)) {
                    row = $('<tr>');
                    row.append($('<td align="center">'));
                    row.append($('<td align="left">'));
                    row.append($('<td align="left">'));
                    row.append($('<td align="right">'));
                    row.append($('<td align="right">'));
                    table.find('tbody').append(row);
                    my.connectedHubs[hubAddress] = {'row': row};
                } else {
                    row = my.connectedHubs[hubAddress].row;
                }

                if (entry.connected === '1') {
                    image = 'images/ball_green.png';
                } else {
                    image = 'images/ball_red.png';
                }

                $(row.find('td')[0]).html($('<img src="' + image + '">'));
                $(row.find('td')[1]).text(hubAddress);
                $(row.find('td')[2]).html($('<div title="' + entry.description +'">')
                                            .append(entry.hubname));
                $(row.find('td')[3]).text(entry.users);
                $(row.find('td')[4]).text(entry['totalshare preformatted']);
            },

            updateConnectedHubs: function (data) {
                if (data.result !== null) {
                    $.each(data.result, function (hubAddress, entry) {
                        my.addConnectedHub(hubAddress, entry);
                    });
                }
                $.each(my.connectedHubs, function (hubAddress, entry) {
                    if (data.result === null || !data.result.hasOwnProperty(hubAddress)) {
                        entry.row.remove();
                    }
                });
                $('table#connectedhubs').trigger('update');
            },

            onConnectedHubsError: function (data) {
                $.each(my.connectedHubs, function (hubAddress, entry) {
                    entry.row.remove();
                });
                my.connectedHubs = {};
            },

            requestConnectedHubs: function () {
                $.jsonRPC.request('hub.listfulldesc', {
                    success : my.updateConnectedHubs,
                    error : my.onConnectedHubsError
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

            onStatisticalDataError: function (data) {
                $('#statisticaldata').text('Error retrieving statistical data...');
            },

            requestStatisticalData: function () {
                $.jsonRPC.request('show.ratio', {
                    success : my.updateStatisticalData,
                    error: my.onStatisticalDataError
                });
            },

            onLoad: function () {
                $('table#connectedhubs').tablesorter({
                    sortList: [[3, 1]]
                });

                $('#connectedhubs').timer({
                    callback: my.requestConnectedHubs,
                    delay: 3000,
                    repeat: true
                });

                $('#statisticaldata').timer({
                    callback: my.requestStatisticalData,
                    delay: 10000,
                    repeat: true
                });
            }
        };

        eiskalt.status = my;
        return my;
    }
);
