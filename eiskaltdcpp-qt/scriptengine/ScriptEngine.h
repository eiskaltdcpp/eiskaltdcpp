/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QFileSystemWatcher>

#include <QMetaType>

#include <QList>

#include <QStringList>
#include <QMap>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

struct ScriptObject {
    QScriptEngine engine;
    QString path;
};

class ScriptEngine :
        public QObject,
        public dcpp::Singleton<ScriptEngine>
{
Q_OBJECT
friend class dcpp::Singleton<ScriptEngine>;

public:

Q_SIGNALS:
    void scriptChanged(const QString &script);
    
public Q_SLOTS:
    void loadScripts();
    void loadScript(const QString&);
    void stopScripts();
    void stopScript(const QString&);
    void prepareThis(QScriptEngine &);

private Q_SLOTS:
    void slotWSKeyChanged(const QString &key, const QString &value);
    void slotScriptChanged(const QString &script);

private:
    ScriptEngine();
    virtual ~ScriptEngine();

    void loadJSScript(const QString&);
#ifdef USE_QML
    void loadQMLScript(const QString&);
#endif

    ScriptEngine(const ScriptEngine&) {}
    ScriptEngine &operator =(const ScriptEngine&){ return *this; }

    void registerStaticMembers(QScriptEngine &);
    void registerDynamicMembers(QScriptEngine &);

    QMap<QString, ScriptObject*> scripts;
    
    QFileSystemWatcher watcher;
};

Q_DECLARE_METATYPE(ScriptEngine*)
#ifndef USE_QML
Q_DECLARE_METATYPE(QList<QObject*>)
#endif

#endif // SCRIPTENGINE_H
