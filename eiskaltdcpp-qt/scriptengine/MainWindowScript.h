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

#pragma once

#include <QObject>
#include <QAction>
#include <QIcon>
#include <QMap>
#include <QJSEngine>

class MainWindowScript : public QObject
{
Q_OBJECT
public:
    explicit MainWindowScript(QJSEngine *engine, QObject *parent = nullptr);
    virtual ~MainWindowScript();

public slots:
    bool addToolButton(const QString &name, const QString &title, const QIcon &icon);
    bool remToolButton(const QString &name);
    bool addMenu(QMenu *menu);
    bool remMenu(QMenu *menu);

private:
    QJSEngine *engine;
    QMap<QString, QAction*> actions;
    QMap<QMenu*, QAction*> menus;
};
