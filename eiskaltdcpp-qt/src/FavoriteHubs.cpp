/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "FavoriteHubs.h"
#include "FavoriteHubModel.h"
#include "MainWindow.h"
#include "WulforUtil.h"
#include "WulforSettings.h"

#include "dcpp/stdinc.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/Util.h"

#include <QTreeView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QList>

using namespace dcpp;

FavoriteHubs::FavoriteHubs(QWidget *parent):
        QWidget(parent),
        model(NULL)
{
    setupUi(this);

    init();

    FavoriteManager::getInstance()->addListener(this);
    
    registerThis();
}

FavoriteHubs::~FavoriteHubs(){
    FavoriteManager::getInstance()->removeListener(this);

    delete model;
}

void FavoriteHubs::closeEvent(QCloseEvent *e){
    if (isUnload()){
        save();

        e->accept();
    }
    else {
        e->ignore();
    }
}

QWidget *FavoriteHubs::getWidget(){
    return this;
}

QString FavoriteHubs::getArenaTitle(){
    return tr("Favorite hubs");
}

QString FavoriteHubs::getArenaShortTitle(){
    return getArenaTitle();
}

QMenu *FavoriteHubs::getMenu(){
    return NULL;
}

void FavoriteHubs::load(){
    treeView->header()->restoreState(QByteArray::fromBase64(WSGET(WS_FAV_HUBS_STATE).toAscii()));
}

void FavoriteHubs::save(){
    WSSET(WS_FAV_HUBS_STATE, treeView->header()->saveState().toBase64());
}

void FavoriteHubs::init(){
    model = new FavoriteHubModel();
    setUnload(false);

    treeView->setModel(model);

    fakeNMDCTags = QStringList();
    fakeADCTags = QStringList();

    fakeNMDCTags << QString("%1").arg(fullVersionString)
            << QString("EiskaltDC++ V:%1").arg(VERSIONSTRING)
            << "++ V:0.75"
            << "++ V:0.777"
            << "StrgDC++ V:2.42"
            << "ApexDC++ V:1.3.6"
            << "FlylinkDC++ V:(r400)"
            << "HomeDC++ V:2.22"
            << "FakeDC V:1.0";

    fakeADCTags << QString("%1").arg(fullADCVersionString)
            << QString("EiskaltDC++ %1").arg(VERSIONSTRING)
            << "++ 0.75"
            << "++ 0.777"
            << "StrgDC++ 2.42"
            << "ApexDC++ 1.3.6"
            << "FlylinkDC++ V:(r400)"
            << "HomeDC++ 2.22"
            << "FakeDC 1.0";

    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;

        QList<QVariant> data;

        data << entry->getConnect()
             << _q(entry->getName())
             << _q(entry->getDescription())
             << _q(entry->getServer())
             << _q(entry->getNick())
             << _q(entry->getPassword())
             << _q(entry->getUserDescription())
             << WulforUtil::getInstance()->dcEnc2QtEnc(_q(entry->getEncoding()));

        model->addResult(data);
    }

    treeView->setRootIsDecorated(false);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->viewport()->setAcceptDrops(false); // temporary
    treeView->setDragEnabled(false); // temporary
    treeView->setAcceptDrops(false); // temporary

    WulforUtil *WU = WulforUtil::getInstance();

    add_newButton->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));
    changeButton->setIcon(WU->getPixmap(WulforUtil::eiEDIT));
    removeButton->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
    connectButton->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));

    load();

    int row_num = model->rowCount();
    if (row_num == 0){
        changeButton->setEnabled(false);
        removeButton->setEnabled(false);
        connectButton->setEnabled(false);
    }

    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotContexMenu(const QPoint&)));
    connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)));
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDblClicked()));

    connect(add_newButton, SIGNAL(clicked()), this, SLOT(slotAdd_newButtonClicked()));
    connect(changeButton,  SIGNAL(clicked()), this, SLOT(slotChangeButtonClicked()));
    connect(removeButton,  SIGNAL(clicked()), this, SLOT(slotRemoveButtonClicked()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(slotConnectButtonClicked()));

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));
    
    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

void FavoriteHubs::initHubEditor(FavoriteHubEditor &editor){
    editor.comboBox_ENC->addItem(tr("System default"));
    editor.comboBox_ENC->addItems(WulforUtil::getInstance()->encodings());
    editor.spinBox_MINSEARCH_INTERVAL->setValue(SETTING(MINIMUM_SEARCH_INTERVAL));
    connect(editor.checkBox_CID, SIGNAL(clicked()), this, SLOT(slotUpdateComboBox_CID()));
    connect(editor.lineEdit_ADDRESS, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateComboBox_CID()));
}

static bool isValidIP(const QString &ip){
    if (ip.isEmpty())
        return false;

    QStringList l = ip.split(".", QString::SkipEmptyParts);
    QIntValidator v(0, 255, NULL);

    bool valid = true;
    int pos = 0;

    foreach (QString s, l)
        valid = valid && (v.validate(s, pos) == QValidator::Acceptable);

    return valid;
}

void FavoriteHubs::initHubEditor(FavoriteHubEditor &editor, StrMap &map){
    initHubEditor(editor);

    editor.checkBox_AUTOCONNECT->setChecked(map["AUTO"].toBool());
    editor.checkBox_NICK->setChecked(map["NICK"].toString() != "" && map["NICK"].toString() != _q(SETTING(NICK)));
    editor.checkBox_USERDESC->setChecked(map["UDESC"].toString() != "");

    if (map["ENC"].toString() == tr("System default"))
        editor.comboBox_ENC->setCurrentIndex(0);
    else
        editor.comboBox_ENC->setCurrentIndex(WulforUtil::getInstance()->encodings().indexOf(map["ENC"].toString())+1);

    editor.lineEdit_ADDRESS->setText(map["ADDR"].toString());
    editor.lineEdit_DESC->setText(map["DESC"].toString());
    editor.lineEdit_NAME->setText(map["NAME"].toString());
    editor.lineEdit_NICK->setText(map["NICK"].toString());
    editor.lineEdit_PASSWORD->setText(map["PASS"].toString());
    editor.lineEdit_USERDESC->setText(map["UDESC"].toString());
    editor.lineEdit_IP->setText(map["IP"].toString());

    editor.checkBox_IP->setChecked(isValidIP(map["IP"].toString()));
    editor.checkBox_USEINTERNET->setChecked(map["IIP"].toBool());
    editor.checkBox_DISABLECHAT->setChecked(map["DCHAT"].toBool());
    editor.checkBox_CID->setChecked(map["OVERTAG"].toBool());
    editor.comboBox_MODE->setCurrentIndex(map["MODE"].toInt());
    editor.spinBox_MINSEARCH_INTERVAL->setValue(map["SINT"].toInt());

    QStringList tags;
    QString tag = map["TAG"].toString();

    editor.comboBox_CID->clear();
    if (map["ADDR"].toString().startsWith("adc://",Qt::CaseInsensitive) ||
        map["ADDR"].toString().startsWith("adcs://",Qt::CaseInsensitive))
        tags = fakeADCTags;
    else
        tags = fakeNMDCTags;
    editor.comboBox_CID->addItems(tags);

    if (tags.indexOf(tag) > 0){
        editor.comboBox_CID->setCurrentIndex(tags.indexOf(tag));
    }
    else {
        editor.comboBox_CID->setCurrentIndex(0);
    }
}

void FavoriteHubs::slotUpdateComboBox_CID(){
    FavoriteHubEditor *editor = qobject_cast<FavoriteHubEditor *>(sender()->parent()->parent());

    if(!editor)
        return;

    QStringList tags;
    QString ADDR = editor->lineEdit_ADDRESS->text();

    if (ADDR.startsWith("adc://",Qt::CaseInsensitive) ||
        ADDR.startsWith("adcs://",Qt::CaseInsensitive))
        tags = fakeADCTags;
    else
        tags = fakeNMDCTags;

    QStringList old_tags;
    for (int i = 0; i < editor->comboBox_CID->count(); i++)
        old_tags.append(editor->comboBox_CID->itemText(i));

    if (tags != old_tags){
        editor->comboBox_CID->clear();
        editor->comboBox_CID->addItems(tags);
        editor->comboBox_CID->setCurrentIndex(0);
    }
}

void FavoriteHubs::getParams(const FavoriteHubEntry *entry, StrMap &map){
    if (!entry)
        return;

    map["NAME"]     = _q(entry->getName());
    map["ADDR"]     = _q(entry->getServer());
    map["DESC"]     = _q(entry->getDescription());
    map["AUTO"]     = entry->getConnect();
    map["NICK"]     = _q(entry->getNick());
    map["PASS"]     = _q(entry->getPassword());
    map["ENC"]      = WulforUtil::getInstance()->dcEnc2QtEnc(_q(entry->getEncoding()));
    map["UDESC"]    = _q(entry->getUserDescription());
    map["TAG"]      = _q(entry->getClientId());
    map["OVERTAG"]  = entry->getOverrideId();
    map["IP"]       = _q(entry->getExternalIP());
    map["IIP"]      = entry->getUseInternetIP();
    map["DCHAT"]    = entry->getDisableChat();
    map["MODE"]     = entry->getMode();
    map["SINT"]     = entry->getSearchInterval();
}

void FavoriteHubs::getParams(const FavoriteHubEditor &editor, StrMap &map){
    WulforUtil *WU = WulforUtil::getInstance();

    map["NAME"]     = editor.lineEdit_NAME->text();
    map["ADDR"]     = editor.lineEdit_ADDRESS->text();
    map["DESC"]     = editor.lineEdit_DESC->text();
    map["AUTO"]     = editor.checkBox_AUTOCONNECT->isChecked();
    map["PASS"]     = editor.lineEdit_PASSWORD->text();
    map["IIP"]      = editor.checkBox_USEINTERNET->isChecked();
    map["DCHAT"]    = editor.checkBox_DISABLECHAT->isChecked();

    if (isValidIP(editor.lineEdit_IP->text()) && editor.checkBox_IP->isChecked())
        map["IP"] = editor.lineEdit_IP->text();
    else
        map["IP"] = "";
    map["OVERTAG"] = editor.checkBox_CID->isChecked();
    if (editor.comboBox_CID->currentIndex() != 0 && editor.checkBox_CID->isChecked())
        map["TAG"] = editor.comboBox_CID->currentText();
    else
        map["TAG"] = editor.comboBox_CID->itemText(0);

    if (editor.checkBox_NICK->isChecked() && !editor.lineEdit_NICK->text().isEmpty())
        map["NICK"] = editor.lineEdit_NICK->text();
    else
        map["NICK"] = "";

    if (editor.comboBox_ENC->currentText() != tr("System default")){
        QString enc = WU->qtEnc2DcEnc(editor.comboBox_ENC->currentText());
        enc = enc.left(enc.indexOf(" "));

        map["ENC"] = enc;
    }
    else
        map["ENC"] = WU->qtEnc2DcEnc(WSGET(WS_DEFAULT_LOCALE));

    map["UDESC"]    = editor.lineEdit_USERDESC->text();
    map["MODE"]     = editor.comboBox_MODE->currentIndex();
    map["SINT"]     = editor.spinBox_MINSEARCH_INTERVAL->value();
}

void FavoriteHubs::updateEntry(FavoriteHubEntry &entry, StrMap &map){
    entry.setConnect((int)map["AUTO"].toBool());
    entry.setName(map["NAME"].toString().toStdString());
    entry.setServer(map["ADDR"].toString().toStdString());
    entry.setEncoding(map["ENC"].toString().toStdString());
    entry.setNick(map["NICK"].toString().toStdString());
    entry.setPassword(map["PASS"].toString().toStdString());
    entry.setUserDescription(map["UDESC"].toString().toStdString());
    entry.setDescription(map["DESC"].toString().toStdString());
    entry.setExternalIP(map["IP"].toString().toStdString());
    entry.setClientId(map["TAG"].toString().toStdString());
    entry.setOverrideId(map["OVERTAG"].toBool());
    entry.setUseInternetIP(map["IIP"].toBool());
    entry.setDisableChat(map["DCHAT"].toBool());
    entry.setMode(map["MODE"].toInt());
    entry.setSearchInterval(map["SINT"].toInt());
}

void FavoriteHubs::updateItem(FavoriteHubItem *item, StrMap &map){
    if (!item)
        return;

    WulforUtil *WU = WulforUtil::getInstance();

    item->updateColumn(COLUMN_HUB_ADDRESS, map["ADDR"]);
    item->updateColumn(COLUMN_HUB_AUTOCONNECT, map["AUTO"]);
    item->updateColumn(COLUMN_HUB_DESC, map["DESC"]);

    if (WU->encodings().contains(map["ENC"].toString()))
        item->updateColumn(COLUMN_HUB_ENCODING, map["ENC"]);
    else
        item->updateColumn(COLUMN_HUB_ENCODING, WulforUtil::getInstance()->dcEnc2QtEnc(map["ENC"].toString()));

    item->updateColumn(COLUMN_HUB_NAME, map["NAME"]);
    item->updateColumn(COLUMN_HUB_NICK, map["NICK"]);
    item->updateColumn(COLUMN_HUB_USERDESC, map["UDESC"]);
}

void FavoriteHubs::slotContexMenu(const QPoint &){
    QItemSelectionModel *s_model = treeView->selectionModel();
    QModelIndexList list = s_model->selectedRows(0);
    WulforUtil *WU = WulforUtil::getInstance();
    bool empty = list.empty();
    QMenu *menu = new QMenu(this);

    if (empty){
        QAction *add_new = new QAction(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD), tr("Add new"), menu);
        menu->addAction(add_new);

        QAction *res = menu->exec(QCursor::pos());

        if (res){
            FavoriteHubEditor editor;

            initHubEditor(editor);

            if (editor.exec() == QDialog::Accepted){
                StrMap map;
                FavoriteHubEntry entry;

                getParams(editor, map);
                updateEntry(entry, map);

                FavoriteManager::getInstance()->addFavorite(entry);
            }
        }
    }
    else {
        FavoriteHubItem *item = static_cast<FavoriteHubItem*>(list.at(0).internalPointer());

        if (!item){
            delete menu;

            return;
        }

        QAction *add_new = new QAction(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD), tr("Add new"), menu);
        QAction *change  = new QAction(WU->getPixmap(WulforUtil::eiEDIT), tr("Change"), menu);
        QAction *remove  = new QAction(WU->getPixmap(WulforUtil::eiEDITDELETE), tr("Delete"), menu);
        QAction *conn    = new QAction(WU->getPixmap(WulforUtil::eiCONNECT), tr("Connect"), menu);
        QAction *sep1    = new QAction(menu);
        QAction *sep2    = new QAction(menu);
        sep1->setSeparator(true);
        sep2->setSeparator(true);

        menu->addActions(QList<QAction*>() << change
                                           << remove
                                           << sep1
                                           << conn
                                           << sep2
                                           << add_new);

        QAction *res = menu->exec(QCursor::pos());

        if (res == change)
            slotChangeButtonClicked();
        else if (res == remove)
            slotRemoveButtonClicked();
        else if (res == conn)
            slotDblClicked();
        else if (res == add_new)
            slotAdd_newButtonClicked();
    }

    delete menu;
}

void FavoriteHubs::slotDblClicked(){
    FavoriteHubItem *item = getItem();

    if (!item)
        return;

    QString address = item->data(COLUMN_HUB_ADDRESS).toString();
    FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address.toStdString());
    QString encoding = WulforUtil::getInstance()->dcEnc2QtEnc(_q(entry->getEncoding()));

    MainWindow::getInstance()->newHubFrame(address, encoding);
}

void FavoriteHubs::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void FavoriteHubs::slotClicked(const QModelIndex &index){
    if (!index.isValid() || index.column() != COLUMN_HUB_AUTOCONNECT || !index.internalPointer())
        return;

    FavoriteHubItem *item = reinterpret_cast<FavoriteHubItem*>(index.internalPointer());
    QString address = item->data(COLUMN_HUB_ADDRESS).toString();
    FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address.toStdString());

    if (entry){
        bool autoconnect = !item->data(COLUMN_HUB_AUTOCONNECT).toBool();

        entry->setConnect(autoconnect);
        item->updateColumn(COLUMN_HUB_AUTOCONNECT, autoconnect);

        model->repaint();

        FavoriteManager::getInstance()->save();
    }
}

void FavoriteHubs::slotSettingsChanged(const QString &key, const QString &value){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void FavoriteHubs::on(FavoriteAdded, const FavoriteHubEntryPtr entry) noexcept{
    QList<QVariant> data;
    WulforUtil *WU = WulforUtil::getInstance();

    data << entry->getConnect()
         << _q(entry->getName())
         << _q(entry->getDescription())
         << _q(entry->getServer())
         << _q(entry->getNick())
         << _q(entry->getPassword())
         << _q(entry->getUserDescription())
         << WU->dcEnc2QtEnc(_q(entry->getEncoding()));

   model->addResult(data);
}

void FavoriteHubs::on(FavoriteRemoved, const FavoriteHubEntryPtr entry) noexcept{
    QString server = _q(entry->getServer());
    FavoriteHubItem *item = NULL;

    QList<FavoriteHubItem*> list = model->getItems();

    foreach (FavoriteHubItem *i, list){
        if (i->data(COLUMN_HUB_ADDRESS).toString() == server){
            item = i;

            break;
        }
    }

    if (!item)
        return;

    model->removeItem(item);
}

void FavoriteHubs::slotAdd_newButtonClicked(){
    FavoriteHubEditor editor;

    initHubEditor(editor);

    if (editor.exec() == QDialog::Accepted){
        StrMap map;
        FavoriteHubEntry entry;

        getParams(editor, map);
        updateEntry(entry, map);

        FavoriteManager::getInstance()->addFavorite(entry);
    }
}

void FavoriteHubs::slotChangeButtonClicked(){
    FavoriteHubItem *item = getItem();

    if (!item)
        return;

    QString address = item->data(COLUMN_HUB_ADDRESS).toString();
    FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address.toStdString());

    FavoriteHubEditor editor;

    if (entry){
        StrMap map;

        getParams(entry, map);
        initHubEditor(editor, map);

        if (editor.exec() == QDialog::Accepted){
            getParams(editor, map);
            updateItem(item, map);
            updateEntry(*entry, map);

            FavoriteManager::getInstance()->save();
        }
    }
}

void FavoriteHubs::slotRemoveButtonClicked(){
    FavoriteHubItem *item = getItem();

    if (!item)
        return;

    QString address = item->data(COLUMN_HUB_ADDRESS).toString();
    FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address.toStdString());

    if (entry)
        FavoriteManager::getInstance()->removeFavorite(entry);
}

void FavoriteHubs::slotConnectButtonClicked(){
    slotDblClicked();
}

FavoriteHubItem *FavoriteHubs::getItem(){
    QItemSelectionModel *s_model = treeView->selectionModel();
    QModelIndexList list = s_model->selectedRows(0);

    FavoriteHubItem *item = NULL;
    if (!list.isEmpty())
        item = static_cast<FavoriteHubItem*>(list.first().internalPointer());

    return item;
}
