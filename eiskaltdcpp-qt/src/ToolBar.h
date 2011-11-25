/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QTabBar>
#include <QEvent>
#include <QShowEvent>
#include <QShortcut>
#include <QList>

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"

class ArenaWidget;
class MainWindow;

class ToolBar :
    public QToolBar,
    public ArenaWidgetContainer
{
    Q_OBJECT

    typedef QMap<ArenaWidget*, unsigned> WidgetMap;

public:
    ToolBar(QWidget* = NULL);
    virtual ~ToolBar();

    virtual bool hasWidget(ArenaWidget*) const;
    void mapWidget(ArenaWidget*);

signals:
    void widgetInserted(ArenaWidget*);

protected:
    virtual bool eventFilter(QObject *, QEvent *);
    virtual void showEvent(QShowEvent *);

public Q_SLOTS:
    void nextTab();
    void prevTab();
    void initTabs();

private Q_SLOTS:
    void slotIndexChanged(int);
    void slotTabMoved(int, int);
    void slotClose(int);
    void slotContextMenu(const QPoint&);
    void slotShorcuts();
    void insertWidget(ArenaWidget *a);
    void removeWidget(ArenaWidget*);
    void updated(ArenaWidget*);
    void mapped(ArenaWidget*);
    void toggled(ArenaWidget*);
    void redraw();


private:
    ArenaWidget *findWidgetForIndex(int);
    void rebuildIndexes(int);

    QTabBar *tabbar;
    QList<QShortcut*> shortcuts;
    WidgetMap map;
};

#endif // TOOLBAR_H
