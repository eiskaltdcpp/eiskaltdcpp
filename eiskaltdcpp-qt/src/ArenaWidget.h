/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef ARENAWIDGET_H
#define ARENAWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QPixmap>
#include <QMetaType>

#ifdef USE_QML
#include <QCloseEvent>
#include <QtDeclarative>
#endif

class ArenaWidget
{
public:
    enum Role{
        Hub=0,
        HubManager,
        PrivateMessage,
        ShareBrowser,
        FavoriteHubs,
        FavoriteUsers,
        Search,
        ADLS,
        PublicHubs,
        Downloads,
        FinishedUploads,
        FinishedDownloads,
        Spy,
        CustomWidget,
        QueuedUsers,
        NoRole  //Not valid for widgets
    };
    
    enum Flags {
        NoFlags         =0,
        Singleton       =1,
        Hidden          =2,
        RaiseOnStart    =4
    };
    
    friend Flags operator|(const Flags &a, const Flags &b);
    friend Flags operator&(const Flags &a, const Flags &b);
    friend Flags operator^(const Flags &a, const Flags &b);
    friend Flags operator~(const Flags &a);

    ArenaWidget();

    virtual QWidget *getWidget() = 0;
    virtual QString getArenaTitle() = 0;
    virtual QString getArenaShortTitle() = 0;
    virtual QMenu *getMenu() = 0;
    virtual QAction *toolButton() { return toolBtn; }
    virtual void  setToolButton(QAction *btn) { if (btn) toolBtn = btn; }
    virtual const QPixmap &getPixmap(){ return _pxmap; }
    
    Flags state() const { return flags; }
    Flags setState(Flags f) { flags = f; }

    virtual void requestFilter() {}
    virtual void requestFocus() {}

    virtual void setUnload(bool b){ _arenaUnload = b; }
    virtual bool isUnload() const { return _arenaUnload; }

    virtual Role role() const = 0;

protected:
    virtual ~ArenaWidget();

private:
    bool _arenaUnload;
    QAction *toolBtn;
    QPixmap _pxmap;
    Flags flags;
};

Q_DECLARE_INTERFACE (ArenaWidget, "com.NegatiV.EiskaltDCPP.ArenaWidget/1.0")
Q_DECLARE_METATYPE(ArenaWidget*)

class ScriptWidget :
    public QObject,
    public ArenaWidget
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)

public:
    ScriptWidget();
    virtual ~ScriptWidget();

    Q_PROPERTY(QString  title READ getArenaTitle WRITE setArenaTitle)
    Q_PROPERTY(QString  shortTitle READ getArenaShortTitle WRITE setArenaShortTitle)
    Q_PROPERTY(QWidget* widget READ getWidget WRITE setWidget)
    Q_PROPERTY(QMenu*   menu READ getMenu WRITE setMenu)
    Q_PROPERTY(QPixmap  pixmap READ getPixmap WRITE setPixmap)

public Q_SLOTS:
    virtual QWidget *getWidget();
    virtual QString getArenaTitle();
    virtual QString getArenaShortTitle();
    virtual QMenu *getMenu();
    virtual const QPixmap &getPixmap();

    virtual void  setWidget(QWidget*);
    virtual void  setArenaTitle(QString);
    virtual void  setArenaShortTitle(QString);
    virtual void  setMenu(QMenu*);
    virtual void  setPixmap(const QPixmap&);

    virtual Role role() const { return ArenaWidget::CustomWidget; }
private:
    QWidget *_wgt;
    QString _arenaTitle;
    QString _arenaShortTitle;
    QPixmap pxm;
    QMenu *_menu;
};

Q_DECLARE_METATYPE(ScriptWidget*)

#ifdef USE_QML
class DeclarativeWidget:
    public QWidget,
    public ArenaWidget
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)

public:
    DeclarativeWidget(const QString &file);
    virtual ~DeclarativeWidget();

    virtual QWidget *getWidget();
    virtual QString getArenaTitle();
    virtual QString getArenaShortTitle();
    virtual QMenu *getMenu();
    virtual const QPixmap &getPixmap();

    virtual Role role() const { return ArenaWidget::CustomWidget; }
protected:
    virtual void closeEvent(QCloseEvent *e);
private:
    QDeclarativeView *view;
};

Q_DECLARE_METATYPE(DeclarativeWidget*)

#endif

#endif // ARENAWIDGET_H
