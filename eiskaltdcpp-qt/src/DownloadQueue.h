/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QWidget>
#include <QCloseEvent>
#include <QModelIndex>
#include <QMenu>
#include <QAction>
#include <QMap>
#include <QMetaType>

#include <dcpp/stdinc.h>
#include <dcpp/QueueManager.h>
#include <dcpp/Singleton.h>
#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "ui_UIDownloadQueue.h"

class DownloadQueueModel;
class DownloadQueueItem;
class DownloadQueueDelegate;
class DownloadQueuePrivate;

class DownloadQueue :
        public QWidget,
        public ArenaWidget,
        private Ui::UIDownloadQueue,
        private dcpp::QueueManagerListener,
        public dcpp::Singleton<DownloadQueue>
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

typedef QMap<QString, QVariant> VarMap;
typedef QMap<QString, QMap<QString, QString> > SourceMap;

friend class dcpp::Singleton<DownloadQueue>;

class Menu{
public:
    enum Action{
        Alternates=0,
        Magnet,
        MagnetWeb,
        MagnetInfo,
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
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiDOWNLOAD); }

    ArenaWidget::Role role() const { return ArenaWidget::Downloads; }

protected:
    virtual void closeEvent(QCloseEvent*);
    // Client callbacks
    virtual void on(dcpp::QueueManagerListener::Added, dcpp::QueueItem *item) noexcept;
    virtual void on(dcpp::QueueManagerListener::Moved, dcpp::QueueItem *item, const std::string &oldTarget) noexcept;
    virtual void on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem *item) noexcept;
    virtual void on(dcpp::QueueManagerListener::SourcesUpdated, dcpp::QueueItem *item) noexcept;
    virtual void on(dcpp::QueueManagerListener::StatusUpdated, dcpp::QueueItem *item) noexcept;

public Q_SLOTS:
    QStringList getSources();
    void removeTarget(const QString&);
    void removeSource(const QString&, const QString&);

signals:
    void added  (const QString&);
    void moved  (const QString&, const QString&);
    void removed(const QString&);

private Q_SLOTS:
    void slotContextMenu(const QPoint&);
    void slotCollapseRow(const QModelIndex &);
    void slotHeaderMenu(const QPoint&);
    void slotUpdateStats(quint64 files, quint64 size);

    void slotSettingsChanged(const QString &key, const QString &value);

    void addFile(const VarMap&);
    void remFile(const VarMap&);
    void updateFile(const VarMap&);

    void requestDelete();

Q_SIGNALS:
    void coreAdded(VarMap);
    void coreMoved(VarMap);
    void coreRemoved(VarMap);
    void coreSourcesUpdated(VarMap);
    void coreStatusUpdated(VarMap);

private:
    DownloadQueue(QWidget* = NULL);
    virtual ~DownloadQueue();

    void init();
    void load();
    void save();

    void getParams(VarMap&, const dcpp::QueueItem*);
    void loadList();

    void getChilds(DownloadQueueItem *i, QList<DownloadQueueItem*>&);
    void getItems(const QModelIndexList &list, QList<DownloadQueueItem*> &items);

    QString getCID(const VarMap&);

    Q_DECLARE_PRIVATE(DownloadQueue);

    DownloadQueuePrivate *d_ptr;
};

Q_DECLARE_METATYPE(DownloadQueue*)
