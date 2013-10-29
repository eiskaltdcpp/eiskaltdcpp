/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser:true */
/*global $, jQuery, config */
var eiskalt = (function () {
    'use strict';

    var debugLevels = {
        FATAL : 1,
        ERROR : 2,
        WARN : 3,
        INFO : 4,
        DEBUG : 5
    }, searchTypes = {
        TYPE_ANY: 0,
        TYPE_AUDIO: 1,
        TYPE_COMPRESSED: 2,
        TYPE_DOCUMENT: 3,
        TYPE_EXECUTABLE: 4,
        TYPE_PICTURE: 5,
        TYPE_VIDEO: 6,
        TYPE_DIRECTORY: 7,
        TYPE_TTH: 8,
        TYPE_CD_IMAGE: 9,
        TYPE_LAST: 10
    }, eiskalt = {

        searchResults: {},
        groupedResults: {},
        downloadQueue: {},
        config: null,
        connectedHubs: {},
        statisticalData: [],

        debugOut: function (debugLevel, text) {
            if (debugLevel <= config.debugLevel) {
                $('#debugdiv').show();
                $('#debugout').prepend((new Date()).toLocaleTimeString() + ': ' + text + '\n');
            }
        },

        onError: function (data) {
            eiskalt.debugOut(debugLevels.ERROR, 'Error: ' + data.error.message);
        },

        onSuccess: function (data) {
            eiskalt.debugOut(debugLevels.INFO, 'Success: ' + (data.result === 0));
        },

        clearSearchResults: function () {
            $.jsonRPC.request('search.clear', {
                success : eiskalt.onSuccess,
                error : eiskalt.onError
            });
            $('table#searchresults tbody > tr').remove();
            eiskalt.groupedResults = {};
            eiskalt.searchResults = {};
        },

        addSearchResult: function (result) {
            var table, row, tth, headers, key, downloadLink, downloadImage;
            table = $('table#searchresults');
            tth = result.TTH;
            if (!eiskalt.groupedResults.hasOwnProperty(tth)) {
                row = $('<tr>');
                downloadImage = $('<img src="images/download.png">');
                downloadLink = $('<a filename="' + result.Filename + '" tth="' + result.TTH + '" size="' + result['Real Size'] + '">').append(downloadImage);
                downloadLink.on('click', eiskalt.onDownloadClicked);
                row.append($('<td>').append(downloadLink));
                row.append($('<td>').text('0'));
                headers = table.find('th');
                headers.each(function (i, header) {
                    key = $(header).attr('key');
                    if (key !== undefined) {
                        row.append($('<td>').text(result[key]));
                    }
                });
                table.find('tbody').append(row);
                eiskalt.groupedResults[tth] = {'row': row, 'results': []};
            }
            eiskalt.groupedResults[tth].results.push(result);
            $(eiskalt.groupedResults[tth].row.children()[1]).text(eiskalt.groupedResults[tth].results.length);
        },

        updateSearchResults: function (data) {
            if (data.result === null) {
                eiskalt.debugOut(debugLevels.INFO, 'search results: ' + data.result);
            } else {
                data.result.forEach(function (result) {
                    var resultId = result.CID + result.TTH;
                    if (!eiskalt.searchResults.hasOwnProperty(resultId)) {
                        eiskalt.searchResults[resultId] = result;
                        eiskalt.addSearchResult(result);
                    }
                });
                $('table#searchresults').trigger('update');
            }
        },

        requestSearchResults: function () {
            $.jsonRPC.request('search.getresults', {
                success : eiskalt.updateSearchResults,
                error : eiskalt.onError
            });
        },

        onSearchSendSuccess: function (data) {
            var searchIsValid = (data.result === 0);
            eiskalt.debugOut(debugLevels.DEBUG, 'searchIsValid: ' + searchIsValid);
            if (searchIsValid) {
                $('input#searchstring').timer('start');
            }
        },

        onSearchClicked: function () {
            $('input#searchstring').timer('stop');
            eiskalt.clearSearchResults();
            $.jsonRPC.request('search.send', {
                params : {'searchstring': $('input#searchstring').val()},
                success : eiskalt.onSearchSendSuccess,
                error : eiskalt.onError
            });
        },

        onDownloadClicked: function () {
            // add file to the download queue
            $.jsonRPC.request('queue.add', {
                params : {'filename': $(this).attr('filename'), 'size': $(this).attr('size'), 'tth': $(this).attr('tth'), 'directory': ''},
                success : eiskalt.onSuccess,
                error : eiskalt.onError
            });
            // start TTH search for the newly added file to get download sources
            $('input#searchstring').timer('stop');
            $.jsonRPC.request('search.send', {
                params : {'searchstring': $(this).attr('tth'), 'searchtype': searchTypes.TYPE_TTH},
                success : eiskalt.onSuccess,
                error : eiskalt.onError
            });
        },

        onRemoveClicked: function () {
            $.jsonRPC.request('queue.remove', {
                params : {'target': $(this).attr('target')},
                success : eiskalt.onSuccess,
                error : eiskalt.onError
            });
            eiskalt.downloadQueue[$(this).attr('target')].row.remove();
        },

        addDownloadQueue: function (entry) {
            var table, row, headers, key, removeLink, removeImage;
            table = $('table#downloadqueue');
            headers = table.find('th');
            if (!eiskalt.downloadQueue.hasOwnProperty(entry.Target)) {
                row = $('<tr>');
                headers.each(function (i, header) {
                    row.append($('<td>'));
                });
                removeImage = $('<img src="images/remove.png">');
                removeLink = $('<a target="' + entry.Target + '">').append(removeImage);
                removeLink.on('click', eiskalt.onRemoveClicked);
                $(row.children()[0]).append(removeLink);
                table.find('tbody').append(row);
                eiskalt.downloadQueue[entry.Target] = {'row': row};
            } else {
                row = eiskalt.downloadQueue[entry.Target].row;
            }
            headers.each(function (i, header) {
                key = $(header).attr('key');
                if (key !== undefined) {
                    $(row.find('td')[i]).text(entry[key]);
                }
            });
        },

        updateDownloadQueue: function (data) {
            eiskalt.showConnectionStatus(false);
            if (data.result !== null) {
                $.each(data.result, function (target, entry) {
                    eiskalt.addDownloadQueue(entry);
                });
            }
            $.each(eiskalt.downloadQueue, function (target, entry) {
                if (data.result === null || !data.result.hasOwnProperty(target)) {
                    entry.row.remove();
                }
            });
            $('table#downloadqueue').trigger('update');
        },

        errorDownloadQueue: function (data) {
            eiskalt.showConnectionStatus(true);
            $.each(eiskalt.downloadQueue, function (target, entry) {
                entry.row.remove();
            });
            eiskalt.downloadQueue = {};
        },

        requestDownloadQueue: function () {
            $.jsonRPC.request('queue.list', {
                success : eiskalt.updateDownloadQueue,
                error : eiskalt.errorDownloadQueue
            });
        },

        showConnectionStatus: function (error) {
            if (error) {
                $('#connectionerror').attr('class', 'error');
                $('#connectionerror').text('Connection to EiskaltDC++ daemon failed on ' + 'http://' + config.jsonrpc.host + ':' + config.jsonrpc.port);
            } else {
                $('#connectionerror').attr('class', 'hidden');
                $('#connectionerror').text('');
            }
        },

        addConnectedHubs: function (target, entry) {
            var table, row, headers, image;
            table = $('table#connectedhubs');
            headers = table.find('th');
            if (!eiskalt.connectedHubs.hasOwnProperty(target)) {
                row = $('<tr>');
                row.append($('<td align="center">'));
                row.append($('<td align="left">'));
                row.append($('<td align="left">'));
                row.append($('<td align="right">'));
                row.append($('<td align="right">'));
                table.find('tbody').append(row);
                eiskalt.connectedHubs[target] = {'row': row};
            } else {
                row = eiskalt.connectedHubs[target].row;
            }

            if (entry["connected"] == 1) {
                image = "images/ball_green.png";
            } else {
                image = "images/ball_red.png";
            }

            $(row.find('td')[0]).html($('<img src="' + image + '">'));
            $(row.find('td')[1]).text(target);
            $(row.find('td')[2]).html($('<div title="' + entry["description"] +'">')
                                        .append(entry["hubname"]));
            $(row.find('td')[3]).text(entry["users"]);
            $(row.find('td')[4]).text(entry["totalshare preformatted"]);
        },

        updateConnectedHubs: function (data) {
            if (data.result !== null) {
                $.each(data.result, function (target, entry) {
                    eiskalt.addConnectedHubs(target, entry);
                });
            }
            $.each(eiskalt.connectedHubs, function (target, entry) {
                if (data.result === null || !data.result.hasOwnProperty(target)) {
                    entry.row.remove();
                }
            });
            $('table#connectedhubs').trigger('update');
        },

        errorConnectedHubs: function (data) {
            $.each(eiskalt.connectedHubs, function (target, entry) {
                entry.row.remove();
            });
            eiskalt.connectedHubs = {};
        },

        requestConnectedHubs: function () {
            $.jsonRPC.request('hub.listfulldesc', {
                success : eiskalt.updateConnectedHubs,
                error : eiskalt.errorConnectedHubs
            });
        },

        updateStatisticalData: function (data) {
            var statisticalData = data.result;
            if (eiskalt.statisticalData !== statisticalData) {
                eiskalt.statisticalData = statisticalData;
                $('#statisticaldata').attr('class', '');
                $('#statisticaldata').html('Downloaded: ' + statisticalData.down + '<br>Uploaded: ' + statisticalData.up + '<br>Ratio: ' + statisticalData.ratio);
            }
        },

        errorStatisticalData: function (data) {
            $('#statisticaldata').text('Statistical data are unknown.');
        },

        requestStatisticalData: function () {
            $.jsonRPC.request('show.ratio', {
                success : eiskalt.updateStatisticalData,
                error: eiskalt.errorStatisticalData
            });
        },

        getURLParameter: function (name) {
            return decodeURIComponent(
                (new RegExp('[?|&]' + name + '=' + '([^&;]+?)(&|#|;|$)').exec(location.search) || ['', ''])[1].replace(/\+/g, '%20')
            ) || null;
        },

        onLoad: function () {
            var searchString;

            $('#tab-container').easytabs();

            $.jsonRPC.setup({
                endPoint : 'http://' + config.jsonrpc.host + ':' + config.jsonrpc.port,
                namespace : ''
            });

            jQuery.tablesorter.addParser({
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

            $('table#downloadqueue').tablesorter({
                sortList: [[1, 0]]
            });
            $('table#searchresults').tablesorter({
                sortList: [[1, 1]]
            });
            $('table#connectedhubs').tablesorter({
                sortList: [[3, 1]]
            });

            $('input#search').on('click', eiskalt.onSearchClicked);
            $('input#searchstring').keypress(function (event) {
                if (event.which === 13) {
                    event.preventDefault();
                    eiskalt.onSearchClicked();
                    return false;
                }
            });

            $('input#searchstring').timer({
                callback: eiskalt.requestSearchResults,
                delay: 500,
                repeat: 50,
                autostart: false
            });
            $('table#downloadqueue').timer({
                callback: eiskalt.requestDownloadQueue,
                delay: 1000,
                repeat: true
            });
            $('table##connectedhubs').timer({
                callback: eiskalt.requestConnectedHubs,
                delay: 2000,
                repeat: true
            });
            $('#statisticaldata').timer({
                callback: eiskalt.requestStatisticalData,
                delay: 3000,
                repeat: true
            });

            searchString = eiskalt.getURLParameter('search');
            if (searchString !== null) {
                $('#tab-container').easytabs('select', '#tab-search');
                $('input#searchstring').val(searchString);
                eiskalt.onSearchClicked();
            }
        }
    };
    return eiskalt;
}());
