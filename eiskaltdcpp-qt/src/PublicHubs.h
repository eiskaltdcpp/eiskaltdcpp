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
#include <QMenu>
#include <QPixmap>
#include <QSortFilterProxyModel>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"

#include "ArenaWidget.h"
#include "WulforUtil.h"
#include "PublicHubModel.h"

#include "ui_UIPublicHubs.h"

class PublicHubs :
        public  QWidget,
        public  dcpp::Singleton<PublicHubs>,
        public  ArenaWidget,
        public  dcpp::FavoriteManagerListener,
        private Ui::UIPublicHubs
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)
friend class dcpp::Singleton<PublicHubs>;

public:
    QString  getArenaTitle(){ return tr("Public Hubs"); }
    QString  getArenaShortTitle(){ return getArenaTitle(); }
    QWidget *getWidget(){ return this; }
    QMenu   *getMenu(){ return NULL; }
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiSERVER); }
    void requestFilter() { slotFilter(); }
    ArenaWidget::Role role() const { return ArenaWidget::PublicHubs; }

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void on(DownloadStarting, const std::string& l) noexcept;
    virtual void on(DownloadFailed, const std::string& l) noexcept;
    virtual void on(DownloadFinished, const std::string& l, bool fromCoral) noexcept;
    virtual void on(LoadedFromCache, const std::string& l, const std::string& d) noexcept;
    virtual void on(Corrupted, const std::string& l) noexcept;

private Q_SLOTS:
    void slotFilter();
    void slotContextMenu();
    void slotHeaderMenu();
    void slotHubChanged(int);
    void slotFilterColumnChanged();
    void slotDoubleClicked(const QModelIndex&);
    void slotSettingsChanged(const QString&, const QString&);

    void setStatus(const QString&);
    void onFinished(const QString&);

Q_SIGNALS:
    void coreDownloadStarted(const QString&);
    void coreDownloadFailed (const QString&);
    void coreDownloadFinished(const QString&);
    void coreCacheLoaded(const QString&);

private:
    PublicHubs(QWidget *parent = NULL);
    ~PublicHubs();

    void updateList();

    dcpp::HubEntryList entries;
    PublicHubModel *model;
    PublicHubProxyModel *proxy;
};
