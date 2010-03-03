#ifndef FINISHEDTRANSFERS_H
#define FINISHEDTRANSFERS_H

#include <QWidget>
#include <QEvent>
#include <QCloseEvent>
#include <QDir>
#include <QComboBox>
#include <QItemSelectionModel>
#include <QDesktopServices>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/FinishedManager.h"
#include "dcpp/FinishedManagerListener.h"
#include "dcpp/Util.h"
#include "dcpp/FinishedItem.h"
#include "dcpp/User.h"
#include "dcpp/Singleton.h"

#include "ui_UIFinishedTransfers.h"
#include "Func.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "FinishedTransfersModel.h"
#include "MainWindow.h"

using namespace dcpp;

class FinishedTransfersCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1205);

    FinishedTransfersCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~FinishedTransfersCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class FinishedTransferProxy: public QWidget{
Q_OBJECT
public:
    FinishedTransferProxy(QWidget *parent):QWidget(parent){}
    ~FinishedTransferProxy(){}

    QString uploadTitle();
    QString downloadTitle();
public slots:
    virtual void slotTypeChanged(int) = 0;
    virtual void slotClear() = 0;
    virtual void slotContextMenu() = 0;
};

template <bool isUpload>
class FinishedTransfers :
        public dcpp::FinishedManagerListener,
        private Ui::UIFinishedTransfers,
        public dcpp::Singleton< FinishedTransfers<isUpload> >,
        public ArenaWidget,
        public FinishedTransferProxy
{

typedef QMap<QString, QVariant> VarMap;
friend class dcpp::Singleton< FinishedTransfers<isUpload> >;

public:
    QWidget *getWidget() { return this;}
    QString getArenaTitle(){ return (isUpload? uploadTitle() : downloadTitle()); }
    QString getArenaShortTitle(){ return getArenaTitle(); }
    QMenu *getMenu() { return NULL; }
    const QPixmap &getPixmap(){
        if (isUpload)
            return WulforUtil::getInstance()->getPixmap(WulforUtil::eiUP);
        else
            return WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWN);
    }

protected:
    virtual void customEvent(QEvent *e){
        if (e->type() == FinishedTransfersCustomEvent::Event){
            FinishedTransfersCustomEvent *c_e = reinterpret_cast<FinishedTransfersCustomEvent*>(e);

            c_e->func()->call();
        }

        e->accept();
    }

    virtual void closeEvent(QCloseEvent *e){
        if (isUnload()){
            MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
            MainWindow::getInstance()->remWidgetFromArena(this);
            MainWindow::getInstance()->remArenaWidget(this);

            setAttribute(Qt::WA_DeleteOnClose);

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
        FinishedTransferProxy(parent)
    {
        setupUi(this);

        model = new FinishedTransfersModel();

        treeView->setModel(model);

        loadList();

        MainWindow::getInstance()->addArenaWidget(this);
        FinishedManager::getInstance()->addListener(this);

        setUnload(false);

        treeView->setContextMenuPolicy(Qt::CustomContextMenu);

        QObject::connect(comboBox, SIGNAL(activated(int)), this, SLOT(slotTypeChanged(int)));
        QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(slotClear()));
        QObject::connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));

        slotTypeChanged(0);
    }

    ~FinishedTransfers(){
        FinishedManager::getInstance()->removeListener(this);

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
    }

    void getParams(const FinishedFileItemPtr& item, const string& file, FinishedTransfers::VarMap &params){
        QString nicks = "";

        params["FNAME"] = _q(file).split(QDir::separator()).last();
        params["TIME"]  = _q(Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime()));
        params["PATH"]  = _q(Util::getFilePath(file));

        for (UserList::const_iterator it = item->getUsers().begin(); it != item->getUsers().end(); ++it)
                nicks += WulforUtil::getInstance()->getNicks(it->get()->getCID()) + " ";

        params["USERS"] = nicks;
        params["TR"]    = (qlonglong)item->getTransferred();
        params["SPEED"] = (qlonglong)item->getAverageSpeed();
        params["CRC32"] = item->getCrc32Checked();
        params["TARGET"]= _q(file);
        params["ELAP"]  = (qlonglong)item->getMilliSeconds();
    }

    void getParams(const FinishedUserItemPtr& item, const UserPtr& user, FinishedTransfers::VarMap &params){
        QString files = "";

        params["TIME"]  = _q(Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime()));
        params["NICK"]  = WulforUtil::getInstance()->getNicks(user->getCID());

        for (StringList::const_iterator it = item->getFiles().begin(); it != item->getFiles().end(); ++it)
                files += _q(*it) + "; ";

        params["FILES"] = files;
        params["TR"]    = (qlonglong)item->getTransferred();
        params["SPEED"] = (qlonglong)item->getAverageSpeed();
        params["CID"]   = _q(user->getCID().toBase32());
        params["ELAP"]  = (qlonglong)item->getMilliSeconds();
    }

    void slotTypeChanged(int index){
        QString from_key = (index == 0)? WS_FTRANSFERS_USERS_STATE : WS_FTRANSFERS_FILES_STATE;
        QString to_key = (index == 0)? WS_FTRANSFERS_FILES_STATE : WS_FTRANSFERS_USERS_STATE;
        QString old_state = treeView->header()->saveState().toBase64();

        WSSET(from_key, old_state);

        model->switchViewType(static_cast<FinishedTransfersModel::ViewType>(index));

        treeView->header()->restoreState(QByteArray::fromBase64(WSGET(to_key).toAscii()));

        treeView->setSortingEnabled(true);
    }

    void slotClear(){
        model->clearModel();
        FinishedManager::getInstance()->removeAll(isUpload);
    }

    void slotContextMenu(){
        static WulforUtil *WU = WulforUtil::getInstance();

        QItemSelectionModel *s_model = treeView->selectionModel();
        QModelIndexList indexes = s_model->selectedRows(0);

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

                if (!file_list.isEmpty())
                    files.append(file_list.split("; ", QString::SkipEmptyParts));
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

    void on(FinishedManagerListener::AddedFile, bool upload, const std::string &file, const FinishedFileItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, file, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addFile, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::AddedUser, bool upload, const dcpp::UserPtr &user, const FinishedUserItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, user, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addUser, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::UpdatedFile, bool upload, const std::string &file, const FinishedFileItemPtr &item) throw(){
        if (isUpload == upload){
            VarMap params;

            getParams(item, file, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addFile, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::RemovedFile, bool upload, const std::string &file) throw(){
        if (isUpload == upload){
            typedef Func1<FinishedTransfersModel, QString> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::remFile, _q(file));

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::UpdatedUser, bool upload, const UserPtr &user) throw(){
        if (isUpload == upload){
            const FinishedManager::MapByUser &umap = FinishedManager::getInstance()->getMapByUser(isUpload);
            FinishedManager::MapByUser::const_iterator userit = umap.find(user);
            if (userit == umap.end())
                return;

            const FinishedUserItemPtr &item = userit->second;

            VarMap params;

            getParams(item, user, params);

            typedef Func1<FinishedTransfersModel, VarMap> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::addUser, params);

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    void on(FinishedManagerListener::RemovedUser, bool upload, const UserPtr &user) throw(){
        if (isUpload == upload){
            typedef Func1<FinishedTransfersModel, QString> FUNC;
            FUNC *f = new FUNC(model, &FinishedTransfersModel::remUser, _q(user->getCID().toBase32()));

            QApplication::postEvent(this, new FinishedTransfersCustomEvent(f));
        }
    }

    FinishedTransfersModel *model;
};

typedef FinishedTransfers<true>  FinishedUploads;
typedef FinishedTransfers<false> FinishedDownloads;

#else

typedef FinishedTransfers<true>  FinishedUploads;
typedef FinishedTransfers<false> FinishedDownloads;

#endif // FINISHEDTRANSFERS_H
