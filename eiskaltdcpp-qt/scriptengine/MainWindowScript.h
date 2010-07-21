/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MAINWINDOWSCRIPT_H
#define MAINWINDOWSCRIPT_H

#include <QObject>
#include <QAction>
#include <QIcon>
#include <QMap>
#include <QtScript/QScriptEngine>

class MainWindowScript : public QObject
{
Q_OBJECT
public:
    explicit MainWindowScript(QScriptEngine *engine, QObject *parent = 0);
    virtual ~MainWindowScript();

public slots:
    bool addToolButton(const QString &name, const QString &title, const QIcon &icon);
    bool remToolButton(const QString &name);
    bool addMenu(QMenu *menu);
    bool remMenu(QMenu *menu);

private:
    QScriptEngine *engine;
    QMap<QString, QAction*> actions;
    QMap<QMenu*, QAction*> menus;
};

#endif // MAINWINDOWSCRIPT_H
