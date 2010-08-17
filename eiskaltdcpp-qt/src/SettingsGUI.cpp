#include "SettingsGUI.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "MainWindow.h"
#include "Notification.h"
#include "EmoticonFactory.h"
#include "CustomFontModel.h"

#include <QListWidgetItem>
#include <QPixmap>
#include <QColor>
#include <QColorDialog>
#include <QStyleFactory>
#include <QFontDialog>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QSystemTrayIcon>
#include <QHeaderView>

#ifndef CLIENT_ICONS_DIR
#define CLIENT_ICONS_DIR ""
#endif

SettingsGUI::SettingsGUI(QWidget *parent) :
    QWidget(parent),
    custom_style(false)
{
    setupUi(this);

    init();
}

SettingsGUI::~SettingsGUI(){

}

void SettingsGUI::init(){
    {//Basic tab
        WulforUtil *WU = WulforUtil::getInstance();
        QStringList styles = QStyleFactory::keys();

        comboBox_THEMES->addItem(tr("Default (need to restart)"));

        foreach (QString s, styles)
            comboBox_THEMES->addItem(s);

        comboBox_THEMES->setCurrentIndex(styles.indexOf(WSGET(WS_APP_THEME)) >= 0? (styles.indexOf(WSGET(WS_APP_THEME))+1) : 0);


        if (WSGET(WS_APP_FONT).isEmpty()){
            lineEdit_APPFONT->setText(qApp->font().toString());
            WSSET(WS_APP_FONT, qApp->font().toString());
        }
        else
            lineEdit_APPFONT->setText(WSGET(WS_APP_FONT));

        int i = 0;
        int k = -1;
#ifndef WIN32
        QDir translationsDir(CLIENT_TRANSLATIONS_DIR);
#else
        QDir translationsDir(qApp->applicationDirPath()+QDir::separator()+CLIENT_TRANSLATIONS_DIR);
#endif//WIN32
        foreach (QString f, translationsDir.entryList(QDir::Files | QDir::NoSymLinks)){
            QString full_path = QDir::toNativeSeparators( translationsDir.filePath(f) );
            QString lang = "";

            if (f == "en.qm")
                lang = tr("English");
            else if (f == "ru.qm")
                lang = tr("Russian");
            else if (f == "be.qm")
                lang = tr("Belarusian");
            else if (f == "hu.qm")
                lang = tr("Hungarian");
            else if (f == "fr.qm")
                lang = tr("French");
            else if (f == "pl.qm")
                lang = tr("Polish");
            else if (f == "sr.qm")
                lang = tr("Serbian");
            else if (f == "uk.qm")
                lang = tr("Ukrainian");
            else if (f == "es.qm")
                lang = tr("Spanish");
            else if (f == "bg.qm")
                lang = tr("Bulgarian");

            if (!lang.isEmpty()){
                comboBox_LANGS->addItem(lang, full_path);

                if (WSGET(WS_TRANSLATION_FILE).endsWith(f))
                    k = i;

                i++;
            }
        }
        comboBox_LANGS->setCurrentIndex(k);

#ifndef WIN32
        QString users = CLIENT_ICONS_DIR "/user/";
#else
        QString users = qApp->applicationDirPath()+QDir::separator()+CLIENT_ICONS_DIR "/user/";
#endif//WIN32
        i = 0;
        k = -1;
        foreach (QString f, QDir(users).entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot)){
            if (!f.isEmpty()){
                comboBox_USERS->addItem(f);

                if (f == WSGET(WS_APP_USERTHEME))
                    k = i;

                i++;
            }
        }
        comboBox_USERS->setCurrentIndex(k);

#ifndef WIN32
        QString icons = CLIENT_ICONS_DIR "/appl/";
#else
        QString icons = qApp->applicationDirPath()+QDir::separator()+CLIENT_ICONS_DIR "/appl/";
#endif//WIN32
        i = 0;
        k = -1;
        foreach (QString f, QDir(icons).entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot)){
            if (!f.isEmpty()){
                comboBox_ICONS->addItem(f);

                if (f == WSGET(WS_APP_ICONTHEME))
                    k = i;

                i++;
            }
        }
        comboBox_ICONS->setCurrentIndex(k);

#ifndef WIN32
        QString emot = CLIENT_ICONS_DIR "/emot/";
#else
        QString emot = qApp->applicationDirPath()+QDir::separator()+CLIENT_ICONS_DIR "/emot/";
#endif//WIN32
        comboBox_EMOT->addItem("");
        comboBox_EMOT->setCurrentIndex(0);
        i = 1;
        foreach (QString f, QDir(emot).entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot)){
            if (!f.isEmpty()){
                comboBox_EMOT->addItem(f);

                if (f == WSGET(WS_APP_EMOTICON_THEME))
                    comboBox_EMOT->setCurrentIndex(i);

                i++;
            }
        }

        lineEdit_LANGFILE->setText(WSGET(WS_TRANSLATION_FILE));

        toolButton_LANGBROWSE->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));

        if (WBGET(WB_MAINWINDOW_REMEMBER))
            radioButton_REMEMBER->setChecked(true);
        else if (WBGET(WB_MAINWINDOW_HIDE))
            radioButton_HIDE->setChecked(true);
        else
            radioButton_SHOW->setChecked(true);

        groupBox_TRAY->setChecked(WBGET(WB_TRAY_ENABLED));
        groupBox_TRAY->setEnabled(QSystemTrayIcon::isSystemTrayAvailable());

        if (WBGET(WB_MAINWINDOW_USE_SIDEBAR))
            comboBox_TABBAR->setCurrentIndex(2);
        else if (WBGET(WB_MAINWINDOW_USE_M_TABBAR))
            comboBox_TABBAR->setCurrentIndex(1);
        else
            comboBox_TABBAR->setCurrentIndex(0);

    }
    {//Chat tab
        checkBox_CHATJOINS->setChecked(WBGET(WB_CHAT_SHOW_JOINS));
        checkBox_JOINSFAV->setChecked(WBGET(WB_CHAT_SHOW_JOINS_FAV));
        checkBox_CHATHIDDEN->setChecked(WBGET(WB_SHOW_HIDDEN_USERS));
        checkBox_CHAT_SHOW_IP->setChecked(WBGET(WB_SHOW_IP_IN_CHAT));
        checkBox_IGNOREPMHUB->setChecked(BOOLSETTING(IGNORE_HUB_PMS));
        checkBox_IGNOREPMBOT->setChecked(BOOLSETTING(IGNORE_BOT_PMS));
        checkBox_REDIRECTPMBOT->setChecked(WBGET(WB_CHAT_REDIRECT_BOT_PMS));
        checkBox_KEEPFOCUS->setChecked(WBGET(WB_CHAT_KEEPFOCUS));
        checkBox_EMOT->setChecked(WBGET(WB_APP_ENABLE_EMOTICON));
        checkBox_EMOTFORCE->setChecked(WBGET(WB_APP_FORCE_EMOTICONS));
        checkBox_USE_CTRL_ENTER->setChecked(WBGET(WB_USE_CTRL_ENTER));
        checkBox_HIGHLIGHTFAVS->setChecked(WBGET(WB_CHAT_HIGHLIGHT_FAVS));
    }
    {//Chat (extended) tab
        comboBox_DBL_CLICK->setCurrentIndex(WIGET(WI_CHAT_DBLCLICK_ACT));
        comboBox_MDL_CLICK->setCurrentIndex(WIGET(WI_CHAT_MDLCLICK_ACT));
        comboBox_DEF_MAGNET_ACTION->setCurrentIndex(WIGET(WI_DEF_MAGNET_ACTION));
        comboBox_APP_UNIT_BASE->setCurrentIndex(comboBox_APP_UNIT_BASE->findText(QString::number(WIGET(WI_APP_UNIT_BASE))));

        lineEdit_TIMESTAMP->setText(WSGET(WS_CHAT_TIMESTAMP));

        spinBox_OUT_IN_HIST->setValue(WIGET(WI_OUT_IN_HIST));
        spinBox_PARAGRAPHS->setValue(WIGET(WI_CHAT_MAXPARAGRAPHS));

        comboBox_CHAT_SEPARATOR->setCurrentIndex(comboBox_CHAT_SEPARATOR->findText(WSGET(WS_CHAT_SEPARATOR)));
    }
    {//Color tab
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

        c.setNamedColor(WSGET(WS_CHAT_FAVUSER_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Favorite User"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_TIME_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Time stamp"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_MSG_COLOR));
        p.fill(c);
        new QListWidgetItem(p, tr("Message"), listWidget_CHATCOLOR);

        c.setNamedColor(WSGET(WS_CHAT_FIND_COLOR));
        h_color = c;

        c.setAlpha(WIGET(WI_CHAT_FIND_COLOR_ALPHA));
        p.fill(c);
        toolButton_H_COLOR->setIcon(p);

        horizontalSlider_H_COLOR->setValue(WIGET(WI_CHAT_FIND_COLOR_ALPHA));
    }
    {// Fonts tab
        CustomFontModel *model = new CustomFontModel(this);
        tableView->setModel(model);

        tableView->horizontalHeader()->restoreState(QByteArray::fromBase64(WSGET(WS_SETTINGS_GUI_FONTS_STATE).toAscii()));

        connect(tableView, SIGNAL(doubleClicked(QModelIndex)), model, SLOT(itemDoubleClicked(QModelIndex)));
        connect(this, SIGNAL(saveFonts()), model, SLOT(ok()));
    }

    connect(checkBox_EMOT, SIGNAL(toggled(bool)), checkBox_EMOTFORCE, SLOT(setEnabled(bool)));
    connect(pushButton_TEST, SIGNAL(clicked()), this, SLOT(slotTestAppTheme()));
    connect(comboBox_THEMES, SIGNAL(activated(int)), this, SLOT(slotThemeChanged()));
    connect(listWidget_CHATCOLOR, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(slotChatColorItemClicked(QListWidgetItem*)));
    connect(toolButton_APPFONTBROWSE, SIGNAL(clicked()), this, SLOT(slotBrowseFont()));
    connect(toolButton_LANGBROWSE, SIGNAL(clicked()), this, SLOT(slotBrowseLng()));
    connect(comboBox_LANGS, SIGNAL(activated(int)), this, SLOT(slotLngIndexChanged(int)));
    connect(comboBox_USERS, SIGNAL(activated(int)), this, SLOT(slotUsersChanged()));
	connect(comboBox_ICONS, SIGNAL(activated(int)), this, SLOT(slotIconsChanged()));
    connect(toolButton_H_COLOR, SIGNAL(clicked()), this, SLOT(slotGetColor()));
    connect(horizontalSlider_H_COLOR, SIGNAL(valueChanged(int)), this, SLOT(slotSetTransparency(int)));
}

void SettingsGUI::ok(){
    SettingsManager *SM = SettingsManager::getInstance();
    {//Basic tab
        if (custom_style && comboBox_THEMES->currentIndex() > 0)
            WSSET(WS_APP_THEME, comboBox_THEMES->currentText());
        else if (comboBox_THEMES->currentIndex() == 0)
            WSSET(WS_APP_THEME, "");

        if (!lineEdit_APPFONT->text().isEmpty())
            WSSET(WS_APP_FONT, lineEdit_APPFONT->text());

        if (!lineEdit_LANGFILE->text().isEmpty())
            WSSET(WS_TRANSLATION_FILE, lineEdit_LANGFILE->text());

        WBSET(WB_MAINWINDOW_REMEMBER, radioButton_REMEMBER->isChecked());
        WBSET(WB_MAINWINDOW_HIDE, radioButton_HIDE->isChecked());

        if (WBGET(WB_TRAY_ENABLED) != groupBox_TRAY->isChecked()){
            WBSET(WB_TRAY_ENABLED, groupBox_TRAY->isChecked());

            Notify->enableTray(WBGET(WB_TRAY_ENABLED));
        }

        if (WSGET(WS_APP_EMOTICON_THEME) != comboBox_EMOT->currentText()){
            WSSET(WS_APP_EMOTICON_THEME, comboBox_EMOT->currentText());

            EmoticonFactory::getInstance()->load();
        }

        if (comboBox_TABBAR->currentIndex() == 2){
            WBSET(WB_MAINWINDOW_USE_SIDEBAR, true);
            WBSET(WB_MAINWINDOW_USE_M_TABBAR, false);
        }
        else if (comboBox_TABBAR->currentIndex() == 1){
            WBSET(WB_MAINWINDOW_USE_SIDEBAR, false);
            WBSET(WB_MAINWINDOW_USE_M_TABBAR, true);
        }
        else{
            WBSET(WB_MAINWINDOW_USE_SIDEBAR, false);
            WBSET(WB_MAINWINDOW_USE_M_TABBAR, false);
        }
    }
    {//Chat tab
        WBSET(WB_SHOW_HIDDEN_USERS, checkBox_CHATHIDDEN->isChecked());
        WBSET(WB_SHOW_IP_IN_CHAT, checkBox_CHAT_SHOW_IP->isChecked());
        WBSET(WB_CHAT_SHOW_JOINS, checkBox_CHATJOINS->isChecked());
        WBSET(WB_CHAT_SHOW_JOINS_FAV, checkBox_JOINSFAV->isChecked());
        WBSET(WB_CHAT_REDIRECT_BOT_PMS, checkBox_REDIRECTPMBOT->isChecked());
        WBSET(WB_CHAT_KEEPFOCUS, checkBox_KEEPFOCUS->isChecked());
        WBSET(WB_APP_ENABLE_EMOTICON, checkBox_EMOT->isChecked());
        WBSET(WB_APP_FORCE_EMOTICONS, checkBox_EMOTFORCE->isChecked());
        WBSET(WB_USE_CTRL_ENTER, checkBox_USE_CTRL_ENTER->isChecked());
        WBSET(WB_CHAT_HIGHLIGHT_FAVS, checkBox_HIGHLIGHTFAVS->isChecked());
    }
    {//Chat (extended) tab
        WISET(WI_CHAT_DBLCLICK_ACT, comboBox_DBL_CLICK->currentIndex());
        WISET(WI_CHAT_MDLCLICK_ACT, comboBox_MDL_CLICK->currentIndex());
        WISET(WI_DEF_MAGNET_ACTION, comboBox_DEF_MAGNET_ACTION->currentIndex());
        WISET(WI_APP_UNIT_BASE, comboBox_APP_UNIT_BASE->currentText().toInt());

        WSSET(WS_CHAT_TIMESTAMP, lineEdit_TIMESTAMP->text());

        WISET(WI_OUT_IN_HIST, spinBox_OUT_IN_HIST->value());
        WISET(WI_CHAT_MAXPARAGRAPHS, spinBox_PARAGRAPHS->value());

        SM->set(SettingsManager::IGNORE_BOT_PMS, checkBox_IGNOREPMBOT->isChecked());
        SM->set(SettingsManager::IGNORE_HUB_PMS, checkBox_IGNOREPMHUB->isChecked());

        WSSET(WS_CHAT_SEPARATOR, comboBox_CHAT_SEPARATOR->currentText());
    }
    {//Color tab
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
        WSSET(WS_CHAT_FAVUSER_COLOR,    QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_TIME_COLOR,       QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());
        WSSET(WS_CHAT_MSG_COLOR,        QColor(listWidget_CHATCOLOR->item(i++)->icon().pixmap(10, 10).toImage().pixel(0, 0)).name());

        WSSET(WS_CHAT_FIND_COLOR,       h_color.name());
        WISET(WI_CHAT_FIND_COLOR_ALPHA, horizontalSlider_H_COLOR->value());
    }

    WSSET(WS_SETTINGS_GUI_FONTS_STATE, tableView->horizontalHeader()->saveState().toBase64());

    emit saveFonts();
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

void SettingsGUI::slotGetColor(){
    QPixmap p(10, 10);
    QColor color(toolButton_H_COLOR->icon().pixmap(10, 10).toImage().pixel(0, 0));
    color = QColorDialog::getColor(color);

    if (color.isValid()) {
        h_color = color;

        color.setAlpha(horizontalSlider_H_COLOR->value());
        p.fill(color);
        toolButton_H_COLOR->setIcon(p);
    }
}

void SettingsGUI::slotSetTransparency(int value){
    QPixmap p(10, 10);
    QColor color = h_color;
    color.setAlpha(value);

    if (color.isValid()) {
        p.fill(color);
        toolButton_H_COLOR->setIcon(p);
    }
}

void SettingsGUI::slotTestAppTheme(){
    if (comboBox_THEMES->currentIndex() == 0){//Default
        WSSET(WS_APP_THEME, "");

        return;
    }

    custom_style = true;

    QString s = comboBox_THEMES->currentText();

    if (s.isEmpty())
        return;

    qApp->setStyle(s);

    WSSET(WS_APP_THEME, s);
}

void SettingsGUI::slotThemeChanged(){
    custom_style = true;
}

void SettingsGUI::slotBrowseFont(){
    bool ok = false;

    QFont f = QFontDialog::getFont(&ok, this);

    if (ok){
        qApp->setFont(f);
        lineEdit_APPFONT->setText(f.toString());

        WSSET(WS_APP_FONT, f.toString());
    }
}

void SettingsGUI::slotBrowseLng(){
    QString file = QFileDialog::getOpenFileName(this, tr("Select translation"), QString(CLIENT_TRANSLATIONS_DIR), tr("Translation (*.qm)"));

    if (!file.isEmpty()){
        file = QDir::toNativeSeparators(file);

        WSSET(WS_TRANSLATION_FILE, file);

        WulforSettings::getInstance()->loadTranslation();
        MainWindow::getInstance()->retranslateUi();

        lineEdit_LANGFILE->setText(WSGET(WS_TRANSLATION_FILE));
    }
}

void SettingsGUI::slotLngIndexChanged(int index){
    QString file = comboBox_LANGS->itemData(index).toString();

    WSSET(WS_TRANSLATION_FILE, file);

    WulforSettings::getInstance()->loadTranslation();
    MainWindow::getInstance()->retranslateUi();

    lineEdit_LANGFILE->setText(WSGET(WS_TRANSLATION_FILE));
}

void SettingsGUI::slotIconsChanged(){
    WSSET(WS_APP_ICONTHEME, comboBox_ICONS->currentText());

    WulforUtil::getInstance()->loadIcons();
}

void SettingsGUI::slotUsersChanged(){
    WSSET(WS_APP_USERTHEME, comboBox_USERS->currentText());
}
