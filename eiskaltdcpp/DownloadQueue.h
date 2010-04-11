#ifndef DOWNLOADQUEUE_H
#define DOWNLOADQUEUE_H

#include <QWidget>
#include <QCloseEvent>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QMap>

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/QueueManager.h>
#include <dcpp/Singleton.h>

#include "ArenaWidget.h"
#include "Func.h"
#include "WulforUtil.h"

#include "ui_UIDownloadQueue.h"

class DownloadQueueModel;
class DownloadQueueItem;
class DownloadQueueDelegate;

class DownloadQueueCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1205);

    DownloadQueueCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~DownloadQueueCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class DownloadQueue :
        public QWidget,
        public ArenaWidget,
        private Ui::UIDownloadQueue,
        private dcpp::QueueManagerListener,
        public dcpp::Singleton<DownloadQueue>
{
    Q_OBJECT

typedef QMap<QString, QVariant> VarMap;
typedef QMap<QString, QMap<QString, QString> > SourceMap;
typedef Func1<DownloadQueue, VarMap> AddFileFunc;
typedef Func1<DownloadQueue, VarMap> UpdateFileFunc;
typedef Func1<DownloadQueue, VarMap> RemFileFunc;

friend class dcpp::Singleton<DownloadQueue>;

class Menu{
public:
    enum Action{
        Alternates=0,
        Magnet,
        RenameMove,
        SetPriority,
        Browse,
        SendPM,
        RemoveSource,
        RemoveUser,
        Remove,
        None
    };

    Menu();
    virtual ~Menu();

    Action exec(const SourceMap&, const QString&, bool multiselect);
    QVariant getArg();

private:
    void clearMenu(QMenu*);
    QMap<QAction*, Action> map;

    QMenu *menu;
    QMenu *set_prio;
    QMenu *browse;
    QMenu *send_pm;
    QMenu *rem_src;
    QMenu *rem_usr;

    QVariant arg;
};

public:
    QString  getArenaTitle(){ return tr("Download Queue"); }
    QString  getArenaShortTitle(){ return getArenaTitle(); }
    QWidget *getWidget(){ return this; }
    QMenu   *getMenu(){ return NULL; }
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWNLOAD); }

protected:
    virtual void closeEvent(QCloseEvent*);
    virtual void customEvent(QEvent *);
    // Client callbacks
    virtual void on(dcpp::QueueManagerListener::Added, dcpp::QueueItem *item) throw();
    virtual void on(dcpp::QueueManagerListener::Moved, dcpp::QueueItem *item, const std::string &oldTarget) throw();
    virtual void on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem *item) throw();
    virtual void on(dcpp::QueueManagerListener::SourcesUpdated, dcpp::QueueItem *item) throw();
    virtual void on(dcpp::QueueManagerListener::StatusUpdated, dcpp::QueueItem *item) throw();

private slots:
    void slotContextMenu(const QPoint&);
    void slotCollapseRow(const QModelIndex &);
    void slotHeaderMenu(const QPoint&);

private:
    DownloadQueue(QWidget* = NULL);
    virtual ~DownloadQueue();

    void init();
    void load();
    void save();

    void getParams(VarMap&, const dcpp::QueueItem*);
    void loadList();

    void addFile(VarMap);
    void remFile(VarMap);
    void updateFile(VarMap);

    void getChilds(DownloadQueueItem *i, QList<DownloadQueueItem*>&);

    QString getCID(const VarMap&);

    DownloadQueueModel *queue_model;
    DownloadQueueModel *file_model;
    DownloadQueueDelegate *delegate;

    Menu *menu;

    SourceMap sources;
    SourceMap badSources;
};

#endif // DOWNLOADQUEUE_H
