/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser: true */
/*global define */

define(
    ['jquery', 'app/eiskalt', 'app/eiskalt.debug', 'app/eiskalt.status', 'app/eiskalt.search', 'jquery.tablesorter', 'jquery.timer'],

    function ($, eiskalt) {
        'use strict';

        var my = {

            downloadQueue: {},
            downloadQueueTTH: {},

            onRemoveClicked: function () {
                $.jsonRPC.request('queue.remove', {
                    params : {'target': $(this).attr('target')},
                    error : eiskalt.debug.onError
                });
                my.downloadQueue[$(this).attr('target')].row.remove();
            },

            addDownloadQueue: function (entry) {
                var table, row, headers, key, removeLink, removeImage;
                table = $('table#downloadqueue');
                headers = table.find('th');
                if (!my.downloadQueue.hasOwnProperty(entry.Target)) {
                    row = $('<tr>');
                    headers.each(function (i, header) {
                        row.append($('<td>'));
                    });
                    removeImage = $('<img src="images/remove.png">');
                    removeLink = $('<a target="' + entry.Target + '">').append(removeImage);
                    removeLink.on('click', my.onRemoveClicked);
                    $(row.children()[0]).append(removeLink);
                    table.find('tbody').append(row);
                    my.downloadQueue[entry.Target] = {'row': row, 'data': entry};
                    my.downloadQueueTTH[entry.TTH] = my.downloadQueue[entry.Target];
                } else {
                    row = my.downloadQueue[entry.Target].row;
                }
                headers.each(function (i, header) {
                    key = $(header).attr('key');
                    if (key !== undefined) {
                        $(row.find('td')[i]).text(entry[key]);
                    }
                });
            },

            updateDownloadQueue: function (data) {
                var tth;
                eiskalt.status.showConnectionStatus(false);
                if (data.result !== null) {
                    $.each(data.result, function (target, entry) {
                        my.addDownloadQueue(entry);
                    });
                }
                $.each(my.downloadQueue, function (target, entryInfo) {
                    tth = entryInfo.data.TTH;
                    if (data.result === null || !data.result.hasOwnProperty(target)) {
                        entryInfo.row.remove();
                        delete my.downloadQueue[target];
                        delete my.downloadQueueTTH[tth];
                    }
                    eiskalt.search.updateSearchResultIcon(tth);
                });
                $('table#downloadqueue').trigger('update');
            },

            onDownloadQueueError: function (data) {
                eiskalt.status.showConnectionStatus(true);
                $.each(my.downloadQueue, function (target, entry) {
                    entry.row.remove();
                });
                my.downloadQueue = {};
            },

            requestDownloadQueue: function () {
                $.jsonRPC.request('queue.list', {
                    success : my.updateDownloadQueue,
                    error : my.onDownloadQueueError
                });
            },

            onLoad: function () {
                // init table sorting and set initial sorting column by key
                $('table#downloadqueue').tablesorter();
                $('table#downloadqueue').find('th').each(function (i, header) {
                    if ($(header).attr('key') === 'Filename') {
                        $('table#downloadqueue').trigger('sorton', [[[i, 0]]]);
                        return;
                    }
                });

                $('table#downloadqueue').timer({
                    callback: my.requestDownloadQueue,
                    delay: 1000,
                    repeat: true
                });
            }
        };

        eiskalt.queue = my;
        return my;
    }
);
