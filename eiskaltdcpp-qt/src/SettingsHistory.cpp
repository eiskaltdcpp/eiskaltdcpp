#include "SettingsHistory.h"

#include "WulforSettings.h"

SettingsHistory::SettingsHistory(QWidget *parent): QWidget(parent) {
    setupUi(this);
    
    connect(pushButton_DEL, SIGNAL(clicked(bool)), this, SLOT(slotDeleteHistory()));
}

SettingsHistory::~SettingsHistory() {

}

void SettingsHistory::ok(){
    // Do nothing
}

void SettingsHistory::slotDeleteHistory() {
    if (checkBox_DH->isChecked()){
        WSSET(WS_DOWNLOAD_DIR_HISTORY, "");
    }
    
    if (checkBox_SH->isChecked()){
        WSSET(WS_SEARCH_HISTORY, "");
    }
}




