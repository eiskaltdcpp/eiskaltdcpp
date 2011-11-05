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
}

SettingsHistory::~SettingsHistory() {

}

void SettingsHistory::ok(){
    WBSET("app/clear-search-history-on-exit", checkBox_SearchHistory->isChecked());
    WBSET("app/clear-download-directories-history-on-exit", checkBox_DirectoriesHistory->isChecked());
}

void SettingsHistory::slotClearSearchHistory() {
    WSSET(WS_SEARCH_HISTORY, "");
}

void SettingsHistory::slotClearDirectoriesHistory() {
    WSSET(WS_DOWNLOAD_DIR_HISTORY, "");
}
