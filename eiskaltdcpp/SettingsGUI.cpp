#include "SettingsGUI.h"
#include "WulforSettings.h"

SettingsGUI::SettingsGUI(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);
}

SettingsGUI::~SettingsGUI(){

}

void SettingsGUI::init(){
    {//Chat tab
        spinBox_PARAGRAPHS->setValue(WIGET(WI_CHAT_MAXPARAGRAPHS));

    }
}

void SettingsGUI::ok(){

}
