/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "MainWindowScript.h"
#include "QtContextAware.h"
#include "QtContext.h"
#include "MainWindow.h"

#include <QtDebug>

MainWindowScript::MainWindowScript(QJSEngine *engine, QObject *parent)
    : QObject(parent)
    , engine(engine)
{
    Q_ASSERT_X(engine != nullptr, Q_FUNC_INFO, "engine == NULL");
}

MainWindowScript::~MainWindowScript(){
}

bool MainWindowScript::addToolButton(const QString &name, const QString &title, const QIcon &icon) {
    QJSValue mwToolBar = engine->globalObject().property("MainWindow").property("ToolBar");

    if (mwToolBar.isUndefined()) {
        qDebug() << "MainWindowScript::addToolButton: MainWindow.ToolBar is undefined";
        return false;
    }

    if (name.isEmpty())
        return false;

    QAction *act = new QAction(icon, title, qtCtx()->mainWindow());
    act->setObjectName("scriptToolbarButton" + name);
    actions.insert(name, act);

    QJSValue act_val = engine->newQObject(act);
    mwToolBar.setProperty(name, act_val);

    qtCtx()->mainWindow()->addActionOnToolBar(act);

    return true;
}

bool MainWindowScript::remToolButton(const QString &name) {
    QJSValue mwToolBar = engine->globalObject().property("MainWindow").property("ToolBar");

    if (mwToolBar.isUndefined()) {
        qDebug() << "MainWindowScript::remToolButton: MainWindow.ToolBar is undefined";
        return false;
    }

    if (mwToolBar.property(name).isUndefined() || !actions.contains(name))
        return false;

    QAction *act = actions.value(name);

    mwToolBar.setProperty(name, QJSValue());

    qtCtx()->mainWindow()->remActionFromToolBar(act);
    actions.remove(name);

    act->deleteLater();

    return true;
}

bool MainWindowScript::addMenu(QMenu *menu) {
    if (!menu || menus.contains(menu) || menu->title().isEmpty())
        return false;

    QMenuBar *menuBar = qtCtx()->mainWindow()->menuBar();
    QAction *act = menuBar->addMenu(menu);

    qtCtx()->mainWindow()->toggleMainMenu(menuBar->isVisible());

    menus.insert(menu, act);

    QJSValue menu_val = engine->newQObject(menu);
    engine->globalObject().property("MainWindow").property("MenuBar").setProperty(menu->title(), menu_val);

    return true;
}

bool MainWindowScript::remMenu(QMenu *menu) {
    if (!(menu && menus.contains(menu)))
        return false;

    QMenuBar *menuBar = qtCtx()->mainWindow()->menuBar();
    menuBar->removeAction(menus[menu]);

    menus.remove(menu);

    menu->deleteLater();

    engine->globalObject().property("MainWindow").property("MenuBar").setProperty(menu->title(), QJSValue());

    qtCtx()->mainWindow()->toggleMainMenu(menuBar->isVisible());

    return true;
}
