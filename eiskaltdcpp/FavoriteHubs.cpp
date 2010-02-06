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
        model(NULL),
        unload(false)
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
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
    MainWindow::getInstance()->remWidgetFromArena(this);

    if (!unload)
        e->ignore();
    else
        e->accept();
}

QWidget *FavoriteHubs::getWidget(){
    return this;
}

QString FavoriteHubs::getArenaTitle(){
    return tr("Favorite hubs");
}

QMenu *FavoriteHubs::getMenu(){
    return NULL;
}

void FavoriteHubs::init(){
    model = new FavoriteHubModel();

    treeView->setModel(model);

    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;

        QList<QVariant> data;

        data << entry->getConnect()
             << QString::fromStdString(entry->getName())
             << QString::fromStdString(entry->getDescription())
             << QString::fromStdString(entry->getServer())
             << QString::fromStdString(entry->getNick())
             << QString::fromStdString(entry->getPassword())
             << QString::fromStdString(entry->getUserDescription())
             << WulforUtil::getInstance()->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));

        model->addResult(data);
    }

    treeView->setRootIsDecorated(false);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    MainWindow::getInstance()->addArenaWidget(this);

    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotContexMenu(const QPoint&)));
    connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)));
}

void FavoriteHubs::initHubEditor(FavoriteHubEditor &editor){
    editor.comboBox_ENC->addItem(tr("System default"));
    editor.comboBox_ENC->addItems(WulforUtil::getInstance()->encodings());
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
}

void FavoriteHubs::getParams(const FavoriteHubEntry *entry, StrMap &map){
    if (!entry)
        return;

    map["NAME"]     = QString::fromStdString(entry->getName());
    map["ADDR"]     = QString::fromStdString(entry->getServer());
    map["DESC"]     = QString::fromStdString(entry->getDescription());
    map["AUTO"]     = entry->getConnect();
    map["NICK"]     = QString::fromStdString(entry->getNick());
    map["PASS"]     = QString::fromStdString(entry->getPassword());
    map["ENC"]      = WulforUtil::getInstance()->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));
    map["UDESC"]    = QString::fromStdString(entry->getUserDescription());
}

void FavoriteHubs::getParams(const FavoriteHubEditor &editor, StrMap &map){
    WulforUtil *WU = WulforUtil::getInstance();

    map["NAME"]     = editor.lineEdit_NAME->text();
    map["ADDR"]     = editor.lineEdit_ADDRESS->text();
    map["DESC"]     = editor.lineEdit_DESC->text();
    map["AUTO"]     = editor.checkBox_AUTOCONNECT->isChecked();
    map["NICK"]     = editor.lineEdit_NICK->text();
    map["PASS"]     = editor.lineEdit_PASSWORD->text();

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
           MainWindow::getInstance()->newHubFrame(address, QString::fromStdString(entry->getEncoding()));
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
         << QString::fromStdString(entry->getName())
         << QString::fromStdString(entry->getDescription())
         << QString::fromStdString(entry->getServer())
         << QString::fromStdString(entry->getNick())
         << QString::fromStdString(entry->getPassword())
         << QString::fromStdString(entry->getUserDescription())
         << WU->dcEnc2QtEnc(QString::fromStdString(entry->getEncoding()));

   model->addResult(data);
}

void FavoriteHubs::on(FavoriteRemoved, const FavoriteHubEntryPtr entry) throw(){
    QString server = QString::fromStdString(entry->getServer());
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

