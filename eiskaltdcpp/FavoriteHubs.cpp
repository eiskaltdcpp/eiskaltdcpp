#include "FavoriteHubs.h"
#include "FavoriteHubModel.h"
#include "MainWindow.h"
#include "WulforUtil.h"
#include "WulforSettings.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/Util.h"

#include <QTreeView>
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
}

FavoriteHubs::~FavoriteHubs(){
    FavoriteManager::getInstance()->removeListener(this);

    MainWindow::getInstance()->remArenaWidget(this);

    delete model;
}

void FavoriteHubs::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

        save();

        setAttribute(Qt::WA_DeleteOnClose);

        e->accept();
    }
    else {
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);

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

    MainWindow::getInstance()->addArenaWidget(this);

    load();

    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotContexMenu(const QPoint&)));
    connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)));
}

void FavoriteHubs::initHubEditor(FavoriteHubEditor &editor){
    editor.comboBox_ENC->addItem(tr("System default"));
    editor.comboBox_ENC->addItems(WulforUtil::getInstance()->encodings());
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
    editor.checkBox_NICK->setChecked(map["NICK"].toString() != "");
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

    QString tag = map["TAG"].toString();
    QStringList tags;
    for (int i = 0; i < editor.comboBox_CID->count(); i++)
        tags.push_back(editor.comboBox_CID->itemText(i));

    if (!tag.isEmpty() || tags.indexOf(tag) > 0){
        editor.checkBox_CID->setChecked(true);
        editor.comboBox_CID->setCurrentIndex(tags.indexOf(tag));
    }
    else {
        editor.checkBox_CID->setChecked(false);
        editor.comboBox_CID->setCurrentIndex(0);
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
    map["IP"]       = _q(entry->getExternalIP());
}

void FavoriteHubs::getParams(const FavoriteHubEditor &editor, StrMap &map){
    WulforUtil *WU = WulforUtil::getInstance();

    map["NAME"]     = editor.lineEdit_NAME->text();
    map["ADDR"]     = editor.lineEdit_ADDRESS->text();
    map["DESC"]     = editor.lineEdit_DESC->text();
    map["AUTO"]     = editor.checkBox_AUTOCONNECT->isChecked();
    map["NICK"]     = editor.lineEdit_NICK->text();
    map["PASS"]     = editor.lineEdit_PASSWORD->text();

    if (isValidIP(editor.lineEdit_IP->text()) && editor.checkBox_IP->isChecked())
        map["IP"] = editor.lineEdit_IP->text();
    else
        map["IP"] = "";

    if (editor.comboBox_CID->currentIndex() != 0 && editor.checkBox_CID->isChecked())
        map["TAG"] = editor.comboBox_CID->currentText();
    else
        map["TAG"] = editor.comboBox_CID->itemText(0);

    if (editor.comboBox_ENC->currentText() != tr("System default")){
        QString enc = WU->qtEnc2DcEnc(editor.comboBox_ENC->currentText());
        enc = enc.left(enc.indexOf(" "));

        map["ENC"] = enc;
    }
    else
        map["ENC"] = WSGET(WS_DEFAULT_LOCALE);

    map["UDESC"]    = editor.lineEdit_USERDESC->text();
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
    entry.setOverrideId(map["TAG"].toString() != "EiskaltDC++ V:2.0");
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

        QString address = item->data(COLUMN_HUB_ADDRESS).toString();
        FavoriteHubEntry *entry = FavoriteManager::getInstance()->getFavoriteHubEntry(address.toStdString());

        if (res == change){
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
        else if (res == remove && entry){
            FavoriteManager::getInstance()->removeFavorite(entry);
        }
        else if (res == conn && entry){
            QString encoding = WulforUtil::getInstance()->dcEnc2QtEnc(_q(entry->getEncoding()));
           MainWindow::getInstance()->newHubFrame(address, encoding);
        }
        else if (res == add_new){
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

    delete menu;
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

void FavoriteHubs::on(FavoriteAdded, const FavoriteHubEntryPtr entry) throw(){
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

void FavoriteHubs::on(FavoriteRemoved, const FavoriteHubEntryPtr entry) throw(){
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

