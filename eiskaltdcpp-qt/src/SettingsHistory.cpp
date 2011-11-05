#include "SettingsHistory.h"

#include "WulforSettings.h"

SettingsHistory::SettingsHistory(QWidget *parent): QWidget(parent) {
    setupUi(this);
    
    connect(pushButton_ClearSearchHistory, SIGNAL(clicked(bool)),
            this, SLOT(slotClearSearchHistory()));
    connect(pushButton_ClearDirectoriesHistory, SIGNAL(clicked(bool)),
            this, SLOT(slotClearDirectoriesHistory()));
    
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
    QString raw = QByteArray::fromBase64(WSGET(WS_DOWNLOAD_DIR_HISTORY).toAscii());
    QStringList temp_pathes = raw.replace("\r","").split('\n', QString::SkipEmptyParts);
    uint maxItemsNumber = WIGET("download-directory-history-items-number", 5);
    
    if (spinBox_DirectoriesHistory->value() != maxItemsNumber){
        maxItemsNumber = spinBox_DirectoriesHistory->value();
        WISET("download-directory-history-items-number", maxItemsNumber);
        
        if (!temp_pathes.isEmpty()){
            while (temp_pathes.count() > maxItemsNumber)
                temp_pathes.removeLast();
            
            QString raw = temp_pathes.join("\n");
            WSSET(WS_DOWNLOAD_DIR_HISTORY, raw.toAscii().toBase64());
        }
    }
    }
}

void SettingsHistory::slotClearSearchHistory() {
    WSSET(WS_SEARCH_HISTORY, "");
}

void SettingsHistory::slotClearDirectoriesHistory() {
    WSSET(WS_DOWNLOAD_DIR_HISTORY, "");
}
