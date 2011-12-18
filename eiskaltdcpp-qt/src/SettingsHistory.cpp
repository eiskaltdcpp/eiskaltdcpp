/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "SettingsHistory.h"

#include "WulforSettings.h"
#include "WulforUtil.h"
#include "DownloadToHistory.h"

SettingsHistory::SettingsHistory(QWidget *parent): QWidget(parent) {
    setupUi(this);
    
    connect(pushButton_ClearSearchHistory, SIGNAL(clicked(bool)),
            this, SLOT(slotClearSearchHistory()));
    connect(pushButton_ClearDirectoriesHistory, SIGNAL(clicked(bool)),
            this, SLOT(slotClearDirectoriesHistory()));
    
    checkBox_TTHSearchHistory->setChecked(WBGET("memorize-tth-search-phrases", false));
    checkBox_SearchHistory->setChecked(WBGET("app/clear-search-history-on-exit", false));
    checkBox_DirectoriesHistory->setChecked(WBGET("app/clear-download-directories-history-on-exit", false));
    
    spinBox_SearchHistory->setValue(WIGET("search-history-items-number", 10));
    spinBox_DirectoriesHistory->setValue(WIGET("download-directory-history-items-number", 5));
}

SettingsHistory::~SettingsHistory() {

}

void SettingsHistory::ok(){
    WBSET("app/clear-search-history-on-exit", checkBox_SearchHistory->isChecked());
    WBSET("app/clear-download-directories-history-on-exit", checkBox_DirectoriesHistory->isChecked());
    
    { // memorize-tth-search-phrases
    WBSET("memorize-tth-search-phrases", checkBox_TTHSearchHistory->isChecked());
    
    if (!checkBox_TTHSearchHistory->isChecked()){
        QString     raw  = QByteArray::fromBase64(WSGET(WS_SEARCH_HISTORY).toAscii());
        QStringList searchHistory = raw.replace("\r","").split('\n', QString::SkipEmptyParts);
        
        QString text = "";
        for (int k = searchHistory.count()-1; k >= 0; k--){
            text = searchHistory.at(k);
            
            if (WulforUtil::isTTH(text))
                searchHistory.removeAt(k);
        }
        
        QString hist = searchHistory.join("\n");
        WSSET(WS_SEARCH_HISTORY, hist.toAscii().toBase64());
    }
    }
    
    { // search-history-items-number
    QString     raw  = QByteArray::fromBase64(WSGET(WS_SEARCH_HISTORY).toAscii());
    QStringList searchHistory = raw.replace("\r","").split('\n', QString::SkipEmptyParts);
    uint maxItemsNumber = WIGET("search-history-items-number", 10);
    
    if (spinBox_SearchHistory->value() != maxItemsNumber){
        maxItemsNumber = spinBox_SearchHistory->value();
        WISET("search-history-items-number", maxItemsNumber);
        
        if (!searchHistory.isEmpty()){
            while (searchHistory.count() > maxItemsNumber)
                searchHistory.removeLast();
            
            QString hist = searchHistory.join("\n");
            WSSET(WS_SEARCH_HISTORY, hist.toAscii().toBase64());
        }
    }
    }
    
    { // download-directory-history-items-number
        WISET("download-directory-history-items-number", spinBox_DirectoriesHistory->value());
        
        QStringList list = DownloadToDirHistory::get();
        DownloadToDirHistory::put(list);
    }
}

void SettingsHistory::slotClearSearchHistory() {
    WSSET(WS_SEARCH_HISTORY, "");
}

void SettingsHistory::slotClearDirectoriesHistory() {
    WSSET(WS_DOWNLOAD_DIR_HISTORY, "");
}
