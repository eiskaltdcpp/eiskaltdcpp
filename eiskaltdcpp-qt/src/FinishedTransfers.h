/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef FINISHEDTRANSFERS_H
#define FINISHEDTRANSFERS_H

#include <QWidget>
#include <QEvent>
#include <QCloseEvent>
#include <QDir>
#include <QComboBox>
#include <QItemSelectionModel>
#include <QDesktopServices>
#include <QHeaderView>

#ifdef USE_QT_SQLITE
#include <QtSql>
#endif

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/FinishedManager.h"
//#include "dcpp/FinishedManagerListener.h"
#include "dcpp/Util.h"
#include "dcpp/FinishedItem.h"
#include "dcpp/User.h"
#include "dcpp/Singleton.h"

#include "ui_UIFinishedTransfers.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "FinishedTransfersModel.h"
#include "MainWindow.h"

using namespace dcpp;

class FinishedTransferProxy: public QWidget{
Q_OBJECT
typedef QMap<QString, QVariant> VarMap;
public:
    FinishedTransferProxy(QWidget *parent):QWidget(parent){}
    ~FinishedTransferProxy(){}

    QString uploadTitle();
    QString downloadTitle();

Q_SIGNALS:
    void coreAddedFile(const VarMap&);
    void coreAddedUser(const VarMap&);
    void coreUpdatedFile(const VarMap&);
    void coreUpdatedUser(const VarMap&);
    void coreRemovedFile(const QString&);
    void coreRemovedUser(const QString&);

public Q_SLOTS:
    virtual void slotTypeChanged(int) = 0;
    virtual void slotClear() = 0;
    virtual void slotContextMenu() = 0;
    virtual void slotHeaderMenu() = 0;
    virtual void slotSwitchOnlyFull(bool) = 0;
    virtual void slotSettingsChanged(const QString &key, const QString &) = 0;
};

template <bool isUpload>
class FinishedTransfers :
        public dcpp::FinishedManagerListener,
        private Ui::UIFinishedTransfers,
        public dcpp::Singleton< FinishedTransfers<isUpload> >,
        public ArenaWidget,
        public FinishedTransferProxy
{
Q_INTERFACES(ArenaWidget)

typedef QMap<QString, QVariant> VarMap;
friend class dcpp::Singleton< FinishedTransfers<isUpload> >;

public:
    QWidget *getWidget() { return this;}
    QString getArenaTitle(){ return (isUpload? uploadTitle() : downloadTitle()); }
    QString getArenaShortTitle(){ return getArenaTitle(); }
    QMenu *getMenu() { return NULL; }
    ArenaWidget::Role role() const;

    const QPixmap &getPixmap(){
        if (isUpload)
            return WICON(WulforUtil::eiUPLIST);
        else
            return WICON(WulforUtil::eiDOWNLIST);
    }

protected:
    virtual void closeEvent(QCloseEvent *e){
        if (isUnload()){
            MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
            MainWindow::getInstance()->remWidgetFromArena(this);
            MainWindow::getInstance()->remArenaWidget(this);

            //setAttribute(Qt::WA_DeleteOnClose);

            QString key = (comboBox->currentIndex() == 0)? WS_FTRANSFERS_FILES_STATE : WS_FTRANSFERS_USERS_STATE;
            QString state = treeView->header()->saveState().toBase64();

            WSSET(key, state);

            e->accept();
        }
        else {
            MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
            MainWindow::getInstance()->remWidgetFromArena(this);

            e->ignore();
        }
    }

private:
    FinishedTransfers(QWidget *parent = NULL) :
        FinishedTransferProxy(parent), db_opened(false)
    {
        setupUi(this);

        model = new FinishedTransfersModel();

        proxy = new FinishedTransferProxyModel();
        proxy->setDynamicSortFilter(true);
        proxy->setSourceModel(model);

        treeView->setModel(proxy);

#ifdef USE_QT_SQLITE
        db = QSqlDatabase::addDatabase("QSQLITE", (isUpload? "FinishedUploads" : "FinishedDownloads"));
        db_file = _q(Util::getPath(Util::PATH_USER_CONFIG)) + (isUpload? "FinishedUploads.sqlite" : "FinishedDownloads.sqlite");

        db.setDatabaseName(db_file);
        db_opened = db.open();

        if (db_opened){
            QSqlQuery q(db);
            q.exec("CREATE TABLE IF NOT EXISTS files (FNAME TEXT PRIMARY KEY, "
                   "TIME TEXT, PATH TEXT, USERS TEXT, TR TEXT, SPEED TEXT, CRC32 INTEGER, TARGET TEXT, ELAP TEXT, FULL INTEGER)");

            q.exec("CREATE TABLE IF NOT EXISTS users (NICK TEXT PRIMARY KEY, "
                   "TIME TEXT, FILES TEXT, TR TEXT, SPEED TEXT, CID TEXT, ELAP TEXT, FULL INTEGER)");
        }
#endif

        loadList();

        MainWindow::getInstance()->addArenaWidget(this);
        FinishedManager::getInstance()->addListener(this);

        setUnload(false);

        treeView->setContextMenuPolicy(Qt::CustomContextMenu);
        treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);

        QObject::connect(this, SIGNAL(coreAddedFile(VarMap)),   model, SLOT(addFile(VarMap)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(coreAddedUser(VarMap)),   model, SLOT(addUser(VarMap)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(coreUpdatedFile(VarMap)), model, SLOT(addFile(VarMap)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(coreUpdatedUser(VarMap)), model, SLOT(addUser(VarMap)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(coreRemovedFile(QString)), model, SLOT(remFile(QString)), Qt::QueuedConnection);
        QObject::connect(this, SIGNAL(coreRemovedUser(QString)), model, SLOT(remUser(QString)), Qt::QueuedConnection);

        QObject::connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));
        QObject::connect(comboBox, SIGNAL(activated(int)), this, SLOT(slotTypeChanged(int)));
        QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(slotClear()));
        QObject::connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
        QObject::connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
        QObject::connect(checkBox_FULL, SIGNAL(toggled(bool)), this, SLOT(slotSwitchOnlyFull(bool)));

        slotSwitchOnlyFull(false);
        slotTypeChanged(0);
    }

    ~FinishedTransfers(){
        FinishedManager::getInstance()->removeListener(this);

        model->clearModel();

#ifdef USE_QT_SQLITE
        db.close();
#endif

        delete proxy;
        delete model;
    }

    void loadList(){
        VarMap params;

        FinishedManager::getInstance()->lockLists();
        const FinishedManager::MapByFile &list = FinishedManager::getInstance()->getMapByFile(isUpload);
        const FinishedManager::MapByUser &user = FinishedManager::getInstance()->getMapByUser(isUpload);

        for (FinishedManager::MapByFile::const_iterator it = list.begin(); it != list.end(); ++it){
            params.clear();

            getParams(it->second, it->first, params);

            model->addFile(params);
        }

        for (FinishedManager::MapByUser::const_iterator uit = user.begin(); uit != user.end(); ++uit){
            params.clear();

            getParams(uit->second, uit->first, params);

            model->addUser(params);;
        }

        FinishedManager::getInstance()->unLockLists();

#ifdef USE_QT_SQLITE
        if (!db_opened)
            return;

        QSqlQuery q(db);

        q.exec("SELECT * FROM files");

        while (q.next()){
            int i = 0;

            params["FNAME"] = q.value(i++);
            params["TIME"]  = q.value(i++);
            params["PATH"]  = q.value(i++);
            params["USERS"] = q.value(i++);
            params["TR"]    = q.value(i++);
            params["SPEED"] = q.value(i++);
            params["CRC32"] = q.value(i++);
            params["TARGET"]= q.value(i++);
            params["ELAP"]  = q.value(i++);
            params["FULL"]  = q.value(i++);

            model->addFile(params);
        }

        q.exec("SELECT * FROM users");

        while (q.next()){
            int i = 0;

            params["NICK"] = q.value(i++);
            params["TIME"]  = q.value(i++);
            params["FILES"]  = q.value(i++);
            params["TR"]    = q.value(i++);
            params["SPEED"] = q.value(i++);
            params["CID"] = q.value(i++);
            params["ELAP"]  = q.value(i++);
            params["FULL"]  = q.value(i++);

            model->addUser(params);
        }
#endif
    }

    void getParams(const FinishedFileItemPtr& item, const string& file, FinishedTransfers::VarMap &params){
        QString nicks = "";

        params["FNAME"] = _q(file).split(QDir::separator()).last();
        params["TIME"]  = _q(Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime()));
        params["PATH"]  = _q(Util::getFilePath(file));

        for (HintedUserList::const_iterator it = item->getUsers().begin(); it != item->getUsers().end(); ++it)
                nicks += WulforUtil::getInstance()->getNicks(it->user->getCID()) + " ";

        params["USERS"] = nicks;
        params["TR"]    = (qlonglong)item->getTransferred();
        params["SPEED"] = (qlonglong)item->getAverageSpeed();
        params["CRC32"] = item->getCrc32Checked();
        params["TARGET"]= _q(file);
        params["ELAP"]  = (qlonglong)item->getMilliSeconds();
        params["FULL"]  = item->isFull();

#ifdef USE_QT_SQLITE
        if (!db_opened)
            return;

        QSqlQuery q(db);
        q.prepare("REPLACE INTO files "
                  "(FNAME, TIME, PATH, USERS, TR, SPEED, CRC32, TARGET, ELAP, FULL) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        q.bindValue(0, params["FNAME"]);
        q.bindValue(1, params["TIME"]);
        q.bindValue(2, params["PATH"]);
        q.bindValue(3, params["USERS"]);
        q.bindValue(4, params["TR"]);
        q.bindValue(5, params["SPEED"]);
        q.bindValue(6, params["CRC32"]);
        q.bindValue(7, params["TARGET"]);
        q.bindValue(8, params["ELAP"]);
        q.bindValue(9, params["FULL"]);

        q.exec();
#endif
    }

    void getParams(const FinishedUserItemPtr& item, const UserPtr& user, FinishedTransfers::VarMap &params){
        QString files = "";

        params["TIME"]  = _q(Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime()));
        params["NICK"]  = WulforUtil::getInstance()->getNicks(user->getCID());

        for (StringList::const_iterator it = item->getFiles().begin(); it != item->getFiles().end(); ++it)
                files += _q(*it) + " ";

        params["FILES"] = files;
        params["TR"]    = (qlonglong)item->getTransferred();
        params["SPEED"] = (qlonglong)item->getAverageSpeed();
        params["CID"]   = _q(user->getCID().toBase32());
        params["ELAP"]  = (qlonglong)item->getMilliSeconds();
        params["FULL"]  = true;

#ifdef USE_QT_SQLITE
        if (!db_opened)
            return;

        QSqlQuery q(db);
        q.prepare("REPLACE INTO users "
                  "(NICK, TIME, FILES, TR, SPEED, CID, ELAP, FULL)"
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
        q.bindValue(0, params["NICK"]);
        q.bindValue(1, params["TIME"]);
        q.bindValue(2, params["FILES"]);
        q.bindValue(3, params["TR"]);
        q.bindValue(4, params["SPEED"]);
        q.bindValue(5, params["CID"]);
        q.bindValue(6, params["ELAP"]);
        q.bindValue(7, params["FULL"]);

        q.exec();

#endif
    }

    void slotTypeChanged(int index){
        QString from_key = (index == 0)? WS_FTRANSFERS_USERS_STATE : WS_FTRANSFERS_FILES_STATE;
        QString to_key = (index == 0)? WS_FTRANSFERS_FILES_STATE : WS_FTRANSFERS_USERS_STATE;
        QString old_state = treeView->header()->saveState().toBase64();

        if (sender() == comboBox)
            WSSET(from_key, old_state);

        treeView->header()->restoreState(QByteArray::fromBase64(WSGET(to_key).toAscii()));
        treeView->setSortingEnabled(true);

        model->switchViewType(static_cast<FinishedTransfersModel::ViewType>(index));

        if (index == FinishedTransfersModel::FileView)
            proxy->setFilterKeyColumn(COLUMN_FINISHED_FULL);
        else
            proxy->setFilterKeyColumn(COLUMN_FINISHED_CRC32);
    }

    void slotClear(){
        model->clearModel();

        try {
            FinishedManager::getInstance()->removeAll(isUpload);
        }
        catch (const std::exception&){}

#ifdef USE_QT_SQLITE
        if (!db_opened)
            return;

        QSqlQuery q(db);
        q.exec("DROP TABLE files");
        q.exec("DROP TABLE users");
#endif
    }

    void slotContextMenu(){
        static WulforUtil *WU = WulforUtil::getInstance();

        QItemSelectionModel *s_model = treeView->selectionModel();
        QModelIndexList p_indexes = s_model->selectedRows(0);
        QModelIndexList indexes;

        foreach (QModelIndex i, p_indexes)
            indexes.push_back(proxy->mapToSource(i));

        if (indexes.size() < 1)
            return;

        QStringList files;

        if (comboBox->currentIndex() == 0){
            FinishedTransfersItem *item = NULL;
            QString file;

            foreach (QModelIndex i, indexes){
                item = reinterpret_cast<FinishedTransfersItem*>(i.internalPointer());
                file = item->data(COLUMN_FINISHED_TARGET).toString();

                if (!file.isEmpty())
                    files.push_back(file);
            }
        }
        else {
            FinishedTransfersItem *item = NULL;
            QString file_list;

            foreach (QModelIndex i, indexes){
                item = reinterpret_cast<FinishedTransfersItem*>(i.internalPointer());
                file_list = item->data(COLUMN_FINISHED_PATH).toString();

                if (!file_list.isEmpty()){
#if QT_VERSION >= 0x040500
                    files.append(file_list.split("; ", QString::SkipEmptyParts));
#else
                    QStringList s = file_list.split("; ", QString::SkipEmptyParts);
                    foreach (QString i, s)
                        files.push_back(i);
#endif
                }

            }
        }

        QMenu *m = new QMenu();
        QAction *open_f   = new QAction(tr("Open file"), m);
        QAction *open_dir = new QAction(WU->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Open directory"), m);

        m->addAction(open_f);
        m->addAction(open_dir);

        QAction *ret = m->exec(QCursor::pos());

        delete m;

        if (ret == open_f){
            foreach (QString f, files){
                if (f.startsWith("/"))
                    f = "file://" + f;
                else
                    f = "file:///" + f;

                QDesktopServices::openUrl(f);
            }
        }
        else if (ret == open_dir){
            foreach (QString f, files){
                f = f.left(f.lastIndexOf(QDir::separator()));

                if (f.startsWith("/"))
                    f = "file://" + f;
                else
                    f = "file:///" + f;

                QDesktopServices::openUrl(f);
            }
        }

    }

    void slotHeaderMenu(){
        WulforUtil::headerMenu(treeView);
    }

    void slotSwitchOnlyFull(bool checked){
        proxy->setFilterFixedString((checked? tr("Yes") : ""));
    }

    void slotSettingsChanged(const QString &key, const QString &){
        if (key == WS_TRANSLATION_FILE)
            retranslateUi(this);
    }

    void on(FinishedManagerListener::AddedFile, bool upload, const std::string &file, const FinishedFileItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, file, params);

            emit coreAddedFile(params);
        }
    }

    void on(FinishedManagerListener::AddedUser, bool upload, const dcpp::HintedUser &user, const FinishedUserItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, user, params);

            emit coreAddedUser(params);
        }
    }

    void on(FinishedManagerListener::UpdatedFile, bool upload, const std::string &file, const FinishedFileItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, file, params);

            emit coreUpdatedFile(params);
        }
    }

    void on(FinishedManagerListener::RemovedFile, bool upload, const std::string &file) throw(){
        if (isUpload == upload){
            emit coreRemovedFile(_q(file));
        }
    }

    void on(FinishedManagerListener::UpdatedUser, bool upload, const dcpp::HintedUser &user) throw(){
        if (isUpload == upload){
            const FinishedManager::MapByUser &umap = FinishedManager::getInstance()->getMapByUser(isUpload);
            FinishedManager::MapByUser::const_iterator userit = umap.find(user);
            if (userit == umap.end())
                return;

            const FinishedUserItemPtr &item = userit->second;

            VarMap params;

            getParams(item, user, params);

            emit coreUpdatedUser(params);
        }
    }

    void on(FinishedManagerListener::RemovedUser, bool upload, const dcpp::HintedUser &user) throw(){
        if (isUpload == upload){
            emit coreRemovedUser(_q(user.user->getCID().toBase32()));
        }
    }

    FinishedTransferProxyModel *proxy;
    FinishedTransfersModel *model;

#ifdef USE_QT_SQLITE
    QSqlDatabase db;
    QString db_file;
#endif
    bool db_opened;
};

template <>
inline ArenaWidget::Role FinishedTransfers<false>::role() const { return ArenaWidget::FinishedDownloads; }

template <>
inline ArenaWidget::Role FinishedTransfers<true>::role() const { return ArenaWidget::FinishedUploads; }

typedef FinishedTransfers<true>  FinishedUploads;
typedef FinishedTransfers<false> FinishedDownloads;

#else

typedef FinishedTransfers<true>  FinishedUploads;
typedef FinishedTransfers<false> FinishedDownloads;

#endif // FINISHEDTRANSFERS_H
