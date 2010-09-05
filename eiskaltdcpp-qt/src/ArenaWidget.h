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

class ArenaWidget
{
public:
    enum Role{
        Hub=0,
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
        NoRole  //Not valid for widgets
    };

    ArenaWidget();

    virtual QWidget *getWidget() = 0;
    virtual QString getArenaTitle() = 0;
    virtual QString getArenaShortTitle() = 0;
    virtual QMenu *getMenu() = 0;
    virtual QAction *toolButton() { return toolBtn; }
    virtual void  setToolButton(QAction *btn) { if (btn) toolBtn = btn; }
    virtual const QPixmap &getPixmap(){ return _pxmap; }

    virtual void DEL_pressed() {}
    virtual void CTRL_F_pressed() {}

    virtual void setUnload(bool b){ _arenaUnload = b; }
    virtual bool isUnload() const { return _arenaUnload; }

    virtual Role role() const = 0;

protected:
    ~ArenaWidget();

private:
    bool _arenaUnload;
    QAction *toolBtn;
    QPixmap _pxmap;
};

Q_DECLARE_INTERFACE (ArenaWidget, "com.NegatiV.EiskaltDCPP.ArenaWidget/1.0")

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

public Q_SLOTS:
    virtual QWidget *getWidget();
    virtual QString getArenaTitle();
    virtual QString getArenaShortTitle();
    virtual QMenu *getMenu();

    virtual void  setWidget(QWidget*);
    virtual void  setArenaTitle(QString);
    virtual void  setArenaShortTitle(QString);
    virtual void  setMenu(QMenu*);

    virtual Role role() const { return ArenaWidget::CustomWidget; }
private:
    QWidget *_wgt;
    QString _arenaTitle;
    QString _arenaShortTitle;
    QMenu *_menu;
};

Q_DECLARE_METATYPE(ScriptWidget*)

#endif // ARENAWIDGET_H
