/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser: true */
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
        'Any': 0,
        'Audio': 1,
        'Compressed': 2,
        'Document': 3,
        'Executable': 4,
        'Picture': 5,
        'Video': 6,
        'Directory': 7,
        'TTH': 8,
        'CD image': 9
    }, eiskalt = {

        searchResults: {},
        groupedResults: {},
        downloadQueue: {},
        downloadQueueTTH: {},
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

        updateSearchResultIcon: function (tth) {
            var downloadLink, removeLink;
            downloadLink = $('#download_' + tth);
            removeLink = $('#remove_' + tth);
            if (eiskalt.downloadQueueTTH.hasOwnProperty(tth)) {
                downloadLink.hide();
                removeLink.show();
                removeLink.attr('target', eiskalt.downloadQueueTTH[tth].data.Target);
            } else {
                downloadLink.show();
                removeLink.hide();
            }
        },

        getUserLinks: function (result) {
            var userlinks = '', re = new RegExp(/%\w+%/gi), matches;
            if (config.hasOwnProperty('userlinks')) {
                $.each(config.userlinks, function (key, data) {
                    userlinks += '<a target="_blank" href="' + data.url + '"><img src="' + data.icon + '">';
                    if (!data.filter) {
                        data.filter = function (text) { return text; };
                    }
                    matches = userlinks.match(re);
                    if (matches !== null) {
                        $.each(matches, function (index, match) {
                            userlinks = userlinks.replace(match, data.filter(result[match.replace(/%/g, '')]));
                        });
                    }
                });
            }
            return userlinks;
        },

        addSearchResult: function (result) {
            var table, row, tth, headers, downloadLink, downloadImage, removeLink, removeImage;
            table = $('table#searchresults');
            tth = result.TTH;
            if (!eiskalt.groupedResults.hasOwnProperty(tth)) {
                row = $('<tr>');

                downloadImage = $('<img src="images/download.png">');
                downloadLink = $('<a id="download_' + tth + '" filename="' + result.Filename + '" tth="' + tth + '" size="' + result['Real Size'] + '">').append(downloadImage);
                downloadLink.on('click', eiskalt.onDownloadClicked);

                removeImage = $('<img src="images/remove.png">');
                removeLink = $('<a id="remove_' + tth + '" target="">').append(removeImage);
                removeLink.on('click', eiskalt.onRemoveClicked);
                removeLink.hide();

                result.UserLinks = eiskalt.getUserLinks(result);
                result.DownloadLink = downloadLink.add(removeLink);

                headers = table.find('th');
                headers.each(function (i, header) {
                    var key = $(header).attr('key');
                    if (key !== undefined) {
                        row.append($('<td key="' + key + '">').append(result[key]));
                    }
                });
                table.find('tbody').append(row);
                eiskalt.groupedResults[tth] = {'row': row, 'results': []};
                eiskalt.updateSearchResultIcon(tth);
            }
            eiskalt.groupedResults[tth].results.push(result);
            eiskalt.groupedResults[tth].row.find('td[key=UserCount]').text(eiskalt.groupedResults[tth].results.length);
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
                params : {
                    'searchstring': $('input#searchstring').val(),
                    'searchtype': $('#searchtype option:selected').val()
                },
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
                params : {'searchstring': $(this).attr('tth'), 'searchtype': searchTypes.TTH},
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
                eiskalt.downloadQueue[entry.Target] = {'row': row, 'data': entry};
                eiskalt.downloadQueueTTH[entry.TTH] = eiskalt.downloadQueue[entry.Target];
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
            var tth;
            eiskalt.showConnectionStatus(false);
            if (data.result !== null) {
                $.each(data.result, function (target, entry) {
                    eiskalt.addDownloadQueue(entry);
                });
            }
            $.each(eiskalt.downloadQueue, function (target, entryInfo) {
                tth = entryInfo.data.TTH;
                if (data.result === null || !data.result.hasOwnProperty(target)) {
                    entryInfo.row.remove();
                    delete eiskalt.downloadQueue[target];
                    delete eiskalt.downloadQueueTTH[tth];
                }
                eiskalt.updateSearchResultIcon(tth);
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

        addConnectedHub: function (target, entry) {
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

            if (entry.connected === '1') {
                image = 'images/ball_green.png';
            } else {
                image = 'images/ball_red.png';
            }

            $(row.find('td')[0]).html($('<img src="' + image + '">'));
            $(row.find('td')[1]).text(target);
            $(row.find('td')[2]).html($('<div title="' + entry.description +'">')
                                        .append(entry.hubname));
            $(row.find('td')[3]).text(entry.users);
            $(row.find('td')[4]).text(entry['totalshare preformatted']);
        },

        updateConnectedHubs: function (data) {
            if (data.result !== null) {
                $.each(data.result, function (target, entry) {
                    eiskalt.addConnectedHub(target, entry);
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

            $.each(searchTypes, function (typename, typeval) {
                $('#searchtype').append(
                    $('<option></option>').val(typeval).html(typename)
                );
            });

            if (!config.hasOwnProperty('userlinks') || config.userlinks.length === 0) {
                $('#userlinks').remove();
            }

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

            // init table sorting and set initial sorting column by key
            $('table#downloadqueue').tablesorter();
            $('table#downloadqueue').find('th').each(function (i, header) {
                if ($(header).attr('key') === 'Filename') {
                    $('table#downloadqueue').trigger('sorton', [[[i, 0]]]);
                    return;
                }
            });

            $('table#searchresults').tablesorter();
            $('table#searchresults').find('th').each(function (i, header) {
                if ($(header).attr('key') === 'UserCount') {
                    $('table#searchresults').trigger('sorton', [[[i, 1]]]);
                    return;
                }
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
            $('table#connectedhubs').timer({
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
