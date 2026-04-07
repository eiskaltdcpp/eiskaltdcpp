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
#include <QJSEngine>
#include <QJSValue>
#include <QFileSystemWatcher>
#include <QMetaType>

#include <QList>

#include <QStringList>
#include <QMap>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"

#include "ScriptBridge.h"

struct ScriptObject {
    QJSEngine engine;
    ScriptBridge *bridge = nullptr;
    QString path;
};

#include "QtContextAware.h"

class ScriptEngine :
        public QObject,
        public QtContextAware
{
Q_OBJECT
friend class QtContext;

public:

Q_SIGNALS:
    void scriptChanged(const QString &script);

public Q_SLOTS:
    void loadScripts();
    void loadScript(const QString&);
    void stopScripts();
    void stopScript(const QString&);

public:
    void prepareThis(QJSEngine &);

private Q_SLOTS:
    void slotWSKeyChanged(const QString &key, const QString &value);
    void slotScriptChanged(const QString &script);
    void slotProcessChangedFiles();

public:
    explicit ScriptEngine(dcpp::DCContext& ctx);
    virtual ~ScriptEngine();

private:

    void loadJSScript(const QString&);
#ifdef USE_QML
    void loadQMLScript(const QString&);
#endif

    ScriptEngine(const ScriptEngine&) = delete;
    ScriptEngine &operator =(const ScriptEngine&) = delete;

    void registerStaticMembers(QJSEngine &);
    void registerDynamicMembers(QJSEngine &);

    QMap<QString, ScriptObject*> scripts;

    QFileSystemWatcher watcher;
    QStringList changedFiles;
};

Q_DECLARE_METATYPE(ScriptEngine*)
#ifndef USE_QML
Q_DECLARE_METATYPE(QList<QObject*>)
#endif
