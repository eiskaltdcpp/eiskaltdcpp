/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "MainWindowScript.h"
#include "MainWindow.h"

#include <QtDebug>

MainWindowScript::MainWindowScript(QScriptEngine *engine, QObject *parent) :
    engine(engine),
    QObject(parent)
{
    Q_ASSERT_X(engine != nullptr, Q_FUNC_INFO, "engine == nullptr");
}

MainWindowScript::~MainWindowScript(){

}

bool MainWindowScript::addToolButton(const QString &name, const QString &title, const QIcon &icon) {
    QScriptValue mwToolBar = engine->globalObject().property("MainWindow").property("ToolBar");

    if(mwToolBar.isUndefined()){
        qDebug() << engine->currentContext()->backtrace();

        engine->abortEvaluation();

        return false;
    }

    if (name.isEmpty())
        return false;

    QAction *act = new QAction(icon, title, MainWindow::getInstance());
    act->setObjectName("scriptToolbarButton"+name);
    actions.insert(name, act);

    QScriptValue act_val = engine->newQObject(act);
    mwToolBar.setProperty(name, act_val);

    MainWindow::getInstance()->addActionOnToolBar(act);

    return true;
}

bool MainWindowScript::remToolButton(const QString &name) {
    QScriptValue mwToolBar = engine->globalObject().property("MainWindow").property("ToolBar");

    if(mwToolBar.isUndefined()){
        qDebug() << engine->currentContext()->backtrace();

        engine->abortEvaluation();

        return false;
    }

    if (mwToolBar.property(name).isUndefined() || !actions.contains(name))
        return false;

    QAction *act = actions.value(name);

    QScriptValue act_val = engine->undefinedValue();
    mwToolBar.setProperty(name, act_val);

    MainWindow::getInstance()->remActionFromToolBar(act);
    actions.remove(name);

    act->deleteLater();

    return true;
}

bool MainWindowScript::addMenu(QMenu *menu) {
    if (!menu || menus.contains(menu) || menu->title().isEmpty())
        return false;

    QMenuBar *menuBar = MainWindow::getInstance()->menuBar();
    QAction *act = menuBar->addMenu(menu);

    MainWindow::getInstance()->toggleMainMenu(menuBar->isVisible());//update menus

    menus.insert(menu, act);

    QScriptValue menu_val = engine->newQObject(menu);
    engine->globalObject().property("MainWindow").property("MenuBar").setProperty(menu->title(), menu_val);

    return true;
}

bool MainWindowScript::remMenu(QMenu *menu) {
    if (!(menu && menus.contains(menu)))
        return false;

    QMenuBar *menuBar = MainWindow::getInstance()->menuBar();
    menuBar->removeAction(menus[menu]);

    menus.remove(menu);

    menu->deleteLater();

    engine->globalObject().property("MainWindow").property("MenuBar").setProperty(menu->title(), engine->undefinedValue());

    MainWindow::getInstance()->toggleMainMenu(menuBar->isVisible());//update menus

    return true;
}
