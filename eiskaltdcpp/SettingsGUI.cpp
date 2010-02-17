#include "SettingsGUI.h"
#include "WulforSettings.h"

#include <QListWidgetItem>
#include <QPixmap>
#include <QColor>
#include <QColorDialog>

SettingsGUI::SettingsGUI(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    init();
}

SettingsGUI::~SettingsGUI(){

}

void SettingsGUI::init(){
    {//Chat tab
        spinBox_PARAGRAPHS->setValue(WIGET(WI_CHAT_MAXPARAGRAPHS));

        checkBox_CHATJOINS->setChecked(WBGET(WB_CHAT_SHOW_JOINS));
        checkBox_CHATHIDDEN->setChecked(WBGET(WB_SHOW_HIDDEN_USERS));

        QColor c;
        QPixmap p(10, 10);

        c.setNamedColor(WSGET(WS_CHAT_LOCAL_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Local user"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_OP_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Operator"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_BOT_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Bot"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_PRIV_LOCAL_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Private: local user"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_PRIV_USER_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Private: user"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_SAY_NICK));
        p.fill(c);
        new QListWidgetItem(p, tr("Chat: Say nick"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_CORE_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Core"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_STAT_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Status"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_USER_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("User"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_TIME_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Time stamp"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_MSG_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Message"), listWidget_CHATCOLOR);

    }

    connect(listWidget_CHATCOLOR, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotChatColorItemClicked(QListWidgetItem*)));
}

void SettingsGUI::ok(){
    {//Chat tab
        WISET(WI_CHAT_MAXPARAGRAPHS, spinBox_PARAGRAPHS->value());

        WBSET(WB_SHOW_HIDDEN_USERS, checkBox_CHATHIDDEN->isChecked());
        WBSET(WB_CHAT_SHOW_JOINS, checkBox_CHATJOINS->isChecked());

        int i = 0;

        WSSET(WS_CHAT_LOCAL_COLOR,      QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_OP_COLOR,         QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_BOT_COLOR,        QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_PRIV_LOCAL_COLOR, QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_PRIV_USER_COLOR,  QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_SAY_NICK,         QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_CORE_COLOR,       QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_STAT_COLOR,       QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_USER_COLOR,       QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_TIME_COLOR,       QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_MSG_COLOR,        QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
    }

    WulforSettings::getInstance()->save();
}

void SettingsGUI::slotChatColorItemClicked(QListWidgetItem *item){
    QPixmap p(10, 10);
    QColor color(item->icon().pixmap(10, 10).toImage().pixel(0, 0));
    color = QColorDialog::getColor(color);

    if (color.isValid()) {
        p.fill(color);
        item->setIcon(p);
    }
}
