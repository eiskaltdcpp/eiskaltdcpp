/* Copyright (c) 2013 Dorian Scholz */
/* License: GPLv3 or later */
/*jslint browser:true */
/*global define */

define(
    ['jquery', 'app/eiskalt', 'config.js', 'jquery.tablesorter', 'jquery.timer'],

    function ($, eiskalt, config) {
        'use strict';

        var my = {
            searchTypes: {
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
            },

            searchResults: {},
            groupedResults: {},

            clearSearchResults: function () {
                $.jsonRPC.request('search.clear', {
                    success : eiskalt.onSuccess,
                    error : eiskalt.onError
                });
                $('table#searchresults tbody > tr').remove();
                my.groupedResults = {};
                my.searchResults = {};
            },

            updateSearchResultIcon: function (tth) {
                var downloadLink, removeLink;
                downloadLink = $('#download_' + tth);
                removeLink = $('#remove_' + tth);
                if (eiskalt.queue.downloadQueueTTH.hasOwnProperty(tth)) {
                    downloadLink.hide();
                    removeLink.show();
                    removeLink.attr('target', eiskalt.queue.downloadQueueTTH[tth].data.Target);
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
                if (!my.groupedResults.hasOwnProperty(tth)) {
                    row = $('<tr>');

                    downloadImage = $('<img src="images/download.png">');
                    downloadLink = $('<a id="download_' + tth + '" filename="' + result.Filename + '" tth="' + tth + '" size="' + result['Real Size'] + '">').append(downloadImage);
                    downloadLink.on('click', my.onDownloadClicked);

                    removeImage = $('<img src="images/remove.png">');
                    removeLink = $('<a id="remove_' + tth + '" target="">').append(removeImage);
                    removeLink.on('click', eiskalt.queue.onRemoveClicked);
                    removeLink.hide();

                    result.UserLinks = my.getUserLinks(result);
                    result.DownloadLink = downloadLink.add(removeLink);

                    headers = table.find('th');
                    headers.each(function (i, header) {
                        var key = $(header).attr('key');
                        if (key !== undefined) {
                            row.append($('<td key="' + key + '">').append(result[key]));
                        }
                    });
                    table.find('tbody').append(row);
                    my.groupedResults[tth] = {'row': row, 'results': []};
                    my.updateSearchResultIcon(tth);
                }
                my.groupedResults[tth].results.push(result);
                my.groupedResults[tth].row.find('td[key=UserCount]').text(my.groupedResults[tth].results.length);
            },

            updateSearchResults: function (data) {
                if (data.result === null) {
                    eiskalt.debugOut(eiskalt.debugLevels.INFO, 'search results: ' + data.result);
                } else {
                    data.result.forEach(function (result) {
                        var resultId = result.CID + result.TTH;
                        if (!my.searchResults.hasOwnProperty(resultId)) {
                            my.searchResults[resultId] = result;
                            my.addSearchResult(result);
                        }
                    });
                    $('table#searchresults').trigger('update');
                }
            },

            requestSearchResults: function () {
                $.jsonRPC.request('search.getresults', {
                    success : my.updateSearchResults,
                    error : eiskalt.onError
                });
            },

            onSearchSendSuccess: function (data) {
                var searchIsValid = (data.result === 0);
                eiskalt.debugOut(eiskalt.debugLevels.DEBUG, 'searchIsValid: ' + searchIsValid);
                if (searchIsValid) {
                    $('input#searchstring').timer('start');
                }
            },

            onSearchClicked: function () {
                $('input#searchstring').timer('stop');
                my.clearSearchResults();
                $.jsonRPC.request('search.send', {
                    params : {
                        'searchstring': $('input#searchstring').val(),
                        'searchtype': $('#searchtype option:selected').val()
                    },
                    success : my.onSearchSendSuccess,
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
                    params : {'searchstring': $(this).attr('tth'), 'searchtype': my.searchTypes.TTH},
                    success : eiskalt.onSuccess,
                    error : eiskalt.onError
                });
            },

            onLoad: function () {
                var searchString, searchType;

                $.each(my.searchTypes, function (typename, typeval) {
                    $('#searchtype').append(
                        $('<option></option>').val(typeval).html(typename)
                    );
                });

                if (!config.hasOwnProperty('userlinks') || config.userlinks.length === 0) {
                    $('#userlinks').remove();
                }

                // init table sorting and set initial sorting column by key
                $('table#searchresults').tablesorter();
                $('table#searchresults').find('th').each(function (i, header) {
                    if ($(header).attr('key') === 'UserCount') {
                        $('table#searchresults').trigger('sorton', [[[i, 1]]]);
                        return;
                    }
                });

                $('input#search').on('click', my.onSearchClicked);
                $('input#searchstring').keypress(function (event) {
                    if (event.which === 13) {
                        event.preventDefault();
                        my.onSearchClicked();
                        return false;
                    }
                });

                $('input#searchstring').timer({
                    callback: my.requestSearchResults,
                    delay: 500,
                    repeat: 50,
                    autostart: false
                });

                searchType = eiskalt.getURLParameter('searchtype');
                if (searchType !== null) {
                    $.each(my.searchTypes, function (typename, typeval) {
                        if (typename.toLowerCase() === String(searchType).toLowerCase()) {
                            $('#searchtype option[value="' + typeval + '"]').attr('selected', true);
                            return false;
                        }
                    });
                }

                searchString = eiskalt.getURLParameter('searchstring');
                if (searchString !== null) {
                    $('#tab-container').easytabs('select', '#tab-search');
                    $('input#searchstring').val(searchString);
                    my.onSearchClicked();
                }
            }
        };

        eiskalt.search = my;
        return my;
    }
);
