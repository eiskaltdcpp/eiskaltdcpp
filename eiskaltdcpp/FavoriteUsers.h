#ifndef FAVORITEUSERS_H
#define FAVORITEUSERS_H

#include <QObject>
#include <QWidget>
#include <QCloseEvent>
#include <QHash>
#include <QTreeWidgetItem>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"

#include "ArenaWidget.h"
#include "Func.h"
#include "WulforUtil.h"

#include "ui_UIFavoriteUsers.h"

class FavoriteUsers :
        public QWidget,
        public dcpp::Singleton<FavoriteUsers>,
        public dcpp::FavoriteManagerListener,
        public ArenaWidget,
        private Ui::UIFavoriteUsers
{
Q_OBJECT
friend class dcpp::Singleton<FavoriteUsers>;
typedef QMap<QString, QVariant> VarMap;

class FavUserEvent: public QEvent{
public:
    static const QEvent::Type EventAddUser  = static_cast<QEvent::Type>(1211);
    static const QEvent::Type EventRemUser  = static_cast<QEvent::Type>(1212);
    static const QEvent::Type EventUpdUser  = static_cast<QEvent::Type>(1213);

    FavUserEvent(): QEvent(EventAddUser) {}
    FavUserEvent(const QString &stat): QEvent(EventUpdUser), stat(stat) {}
    FavUserEvent(const dcpp::CID &cid):QEvent(EventRemUser), cid(cid) {}
    virtual ~FavUserEvent() { }

    VarMap &getMap() { return map; }
    QString &getStat() {return stat; }
    dcpp::CID &getCID() {return cid; }
private:
    dcpp::CID cid;
    QString stat;
    VarMap map;
};

public:

    virtual QWidget *getWidget() { return this; }
    virtual QString getArenaTitle() { return tr("Favorite Users"); }
    virtual QMenu *getMenu() { return NULL; }
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiUSERS); }

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void customEvent(QEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

    virtual void on(UserAdded, const dcpp::FavoriteUser& aUser) throw();
    virtual void on(UserRemoved, const dcpp::FavoriteUser& aUser) throw();
    virtual void on(StatusChanged, const dcpp::UserPtr& aUser) throw();

private slots:
    void slotContextMenu();

private:
    FavoriteUsers(QWidget *parent = 0);
    virtual ~FavoriteUsers();

    void handleRemove(QTreeWidgetItem*);
    void handleDesc(QTreeWidgetItem*);

    QString cidForItem(QTreeWidgetItem *);

    void getParams(VarMap &map, const dcpp::FavoriteUser &);
    void updItem(const QString&, QTreeWidgetItem *);

    void addUser(const VarMap &);
    void updateUser(const QString &, const QString &);
    void remUser(const QString &);

    QHash<QString, QTreeWidgetItem*> hash;//CID -> Item
};

#endif // FAVORITEUSERS_H
