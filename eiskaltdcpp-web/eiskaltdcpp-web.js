/*jslint browser:true */
/*global $, jQuery, config*/
var eiskalt = (function () {
    "use strict";

    var debugLevels = {
        FATAL : 1,
        ERROR : 2,
        WARN : 3,
        INFO : 4,
        DEBUG : 5
    }, eiskalt = {

        searchResults: {},
        groupedResults: {},
        downloadQueue: {},
        config: null,

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
                $('input#searchstring').timer('stop');
                $('input#searchstring').timer('start');
            }
        },

        onSearchClicked: function () {
            eiskalt.clearSearchResults();
            $.jsonRPC.request('search.send', {
                params : {'searchstring': $('input#searchstring').val()},
                success : eiskalt.onSearchSendSuccess,
                error : eiskalt.onError
            });
        },

        onDownloadClicked: function () {
            $.jsonRPC.request('queue.add', {
                params : {'filename': $(this).attr('filename'), 'size': $(this).attr('size'), 'tth': $(this).attr('tth'), 'directory': ''},
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
            //eiskalt.debugOut(debugLevels.DEBUG, 'updateDownloadQueue: ' + data.result);
            if (data.result !== null) {
                $.each(data.result, function (target, entry) {
                    eiskalt.addDownloadQueue(entry);
                });
            }
            $.each(eiskalt.downloadQueue, function (target, entry) {
                if (!data.result.hasOwnProperty(target)) {
                    entry.row.remove();
                }
            });
        },

        requestDownloadQueue: function () {
            $.jsonRPC.request('queue.list', {
                success : eiskalt.updateDownloadQueue,
                error : eiskalt.onError
            });
        },

        onLoad: function () {
            $.jsonRPC.setup({
                endPoint : 'http://' + config.jsonrpc.host + ':' + config.jsonrpc.port,
                namespace : ''
            });
            $('input#search').on('click', eiskalt.onSearchClicked);
            $('input#searchstring').timer({
                callback: eiskalt.requestSearchResults,
                delay: 1000,
                repeat: 5,
                autostart: false
            });
            $('table#downloadqueue').timer({
                callback: eiskalt.requestDownloadQueue,
                delay: 1000,
                repeat: true
            });
        }
    };
    return eiskalt;
}());
