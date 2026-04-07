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

#include "SettingsHistory.h"
#include "QtContextAware.h"
#include "QtContext.h"

#include <QPushButton>

#include "WulforSettings.h"
#include "WulforUtil.h"
#include "DownloadToHistory.h"

SettingsHistory::SettingsHistory(QWidget *parent): QWidget(parent) {
    setupUi(this);
    
    connect(pushButton_ClearSearchHistory, &QPushButton::clicked,
            this, &SettingsHistory::slotClearSearchHistory);
    connect(pushButton_ClearDirectoriesHistory, &QPushButton::clicked,
            this, &SettingsHistory::slotClearDirectoriesHistory);
    
    checkBox_TTHSearchHistory->setChecked(qtCtx()->settings()->getBool("memorize-tth-search-phrases", false));
    checkBox_SearchHistory->setChecked(qtCtx()->settings()->getBool("app/clear-search-history-on-exit", false));
    checkBox_DirectoriesHistory->setChecked(qtCtx()->settings()->getBool("app/clear-download-directories-history-on-exit", false));
    
    spinBox_SearchHistory->setValue(qtCtx()->settings()->getInt("search-history-items-number", 10));
    spinBox_DirectoriesHistory->setValue(qtCtx()->settings()->getInt("download-directory-history-items-number", 5));
}

SettingsHistory::~SettingsHistory() {

}

void SettingsHistory::ok(){
    qtCtx()->settings()->setBool("app/clear-search-history-on-exit", checkBox_SearchHistory->isChecked());
    qtCtx()->settings()->setBool("app/clear-download-directories-history-on-exit", checkBox_DirectoriesHistory->isChecked());
    
    { // memorize-tth-search-phrases
    qtCtx()->settings()->setBool("memorize-tth-search-phrases", checkBox_TTHSearchHistory->isChecked());
    
    if (!checkBox_TTHSearchHistory->isChecked()){
        QString     raw  = QByteArray::fromBase64(qtCtx()->settings()->getStr(WS_SEARCH_HISTORY).toUtf8());
        QStringList searchHistory = raw.replace("\r","").split('\n', Qt::SkipEmptyParts);
        
        QString text = "";
        for (int k = searchHistory.count()-1; k >= 0; k--){
            text = searchHistory.at(k);
            
            if (WulforUtil::isTTH(text))
                searchHistory.removeAt(k);
        }
        
        QString hist = searchHistory.join("\n");
        qtCtx()->settings()->setStr(WS_SEARCH_HISTORY, hist.toUtf8().toBase64());
    }
    }
    
    { // search-history-items-number
    QString     raw  = QByteArray::fromBase64(qtCtx()->settings()->getStr(WS_SEARCH_HISTORY).toUtf8());
    QStringList searchHistory = raw.replace("\r","").split('\n', Qt::SkipEmptyParts);
    int maxItemsNumber = qtCtx()->settings()->getInt("search-history-items-number", 10);
    
    if (spinBox_SearchHistory->value() != maxItemsNumber){
        maxItemsNumber = spinBox_SearchHistory->value();
        qtCtx()->settings()->setInt("search-history-items-number", maxItemsNumber);
        
        if (!searchHistory.isEmpty()){
            while (searchHistory.count() > maxItemsNumber)
                searchHistory.removeLast();
            
            QString hist = searchHistory.join("\n");
            qtCtx()->settings()->setStr(WS_SEARCH_HISTORY, hist.toUtf8().toBase64());
        }
    }
    }
    
    { // download-directory-history-items-number
        qtCtx()->settings()->setInt("download-directory-history-items-number", spinBox_DirectoriesHistory->value());
        
        QStringList list = DownloadToDirHistory::get();
        DownloadToDirHistory::put(list);
    }
}

void SettingsHistory::slotClearSearchHistory() {
    qtCtx()->settings()->setStr(WS_SEARCH_HISTORY, "");
}

void SettingsHistory::slotClearDirectoriesHistory() {
    qtCtx()->settings()->setStr(WS_DOWNLOAD_DIR_HISTORY, "");
}
