/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#pragma once

#include <QStringList>
#include <QByteArray>
#include <QDir>

#include "WulforSettings.h"
#include "QtContextAware.h"
#include "QtContext.h"

struct DownloadToDirHistory {
    static QStringList get() {
        QString paths = QByteArray::fromBase64(qtCtx()->settings()->getStr(WS_DOWNLOAD_DIR_HISTORY).toUtf8());
        QStringList result;

        for (auto path : paths.replace("\r","").split("\n", Qt::SkipEmptyParts)) {
            if (path.endsWith(QDir::separator()))
                path = path.left(path.length()-1);

            if (!result.contains(path))
                result.push_back(path);
        }

        return result;
    }

    static void put(QStringList &list) {
        const int maxItemsNumber = qtCtx()->settings()->getInt("download-directory-history-items-number", 5);

        while ( list.count() > maxItemsNumber )
            list.removeLast();

        QString raw = list.join ( "\n" );
        qtCtx()->settings()->setStr( WS_DOWNLOAD_DIR_HISTORY, raw.toUtf8().toBase64() );
    }
};
