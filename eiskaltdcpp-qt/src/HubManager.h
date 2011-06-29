/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef HUBMANAGER_H
#define HUBMANAGER_H

#include <QObject>
#include <QHash>
#include <QMap>
#include <QMetaType>
#include <QMenu>
#include <QCloseEvent>

#include "ui_UIHubManager.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"

class HubFrame;

class HubManager :
        public QWidget,
        public ArenaWidget,
        public dcpp::Singleton<HubManager>,
        private Ui::UIHubManager
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)

friend class dcpp::Singleton<HubManager>;
friend class HubFrame;
typedef QHash<QString, HubFrame*> HubHash;

public:
    QWidget *getWidget() { return this; }
    QString getArenaTitle() { return tr("Hub Manager"); }
    QString getArenaShortTitle() { return getArenaTitle(); }
    QMenu *getMenu() { return NULL; }
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiSERVER); }

    ArenaWidget::Role role() const { return ArenaWidget::HubManager; }

Q_SIGNALS:
    void newMessage(HubFrame*, const QString &hubUrl, const QString &cid, const QString &nick, const QString &msg);

protected:
    virtual void closeEvent(QCloseEvent *);

public Q_SLOTS:
    QObject *getHubObject();
    QList<HubFrame*> getHubs() const;
    HubFrame *getHub(const QString &);
    HubFrame *activeHub() const;

private Q_SLOTS:
    void slotHubUpdated();
    void slotContextMenu();
    void slotHubClosed();

private:
    explicit HubManager();
    virtual ~HubManager();

    void registerHubUrl(const QString &, HubFrame *);
    void unregisterHubUrl(const QString &);
    void setActiveHub(HubFrame*);

    HubHash hubs;
    HubFrame *active;

    QMap<HubFrame*,QTreeWidgetItem*> items;
};

Q_DECLARE_METATYPE(HubManager*)

#endif // HUBMANAGER_H
