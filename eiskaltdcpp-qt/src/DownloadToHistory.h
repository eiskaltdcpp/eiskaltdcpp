/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/ 

#ifndef DOWNLOADTOHISTORY_H
#define DOWNLOADTOHISTORY_H

#include <QStringList>
#include <QByteArray>
#include <QDir>

#include "WulforSettings.h"

struct DownloadToDirHistory {
    static QStringList get() {
        QString paths = QByteArray::fromBase64(WSGET(WS_DOWNLOAD_DIR_HISTORY).toAscii());
        QStringList result;
        
        foreach (QString path, paths.replace("\r","").split("\n", QString::SkipEmptyParts)) {
            if (path.endsWith(QDir::separator()))
                path = path.left(path.length()-1);
            
            if (!result.contains(path))
                result.push_back(path);
        }
        
        return result;
    }
    
    static void put(QStringList &list) {
        uint maxItemsNumber = WIGET ( "download-directory-history-items-number", 5 );
        
        while ( list.count() > maxItemsNumber )
            list.removeLast();

        QString raw = list.join ( "\n" );
        WSSET ( WS_DOWNLOAD_DIR_HISTORY, raw.toAscii().toBase64() );
    }
};

#endif // DOWNLOADTOHISTORY_H
