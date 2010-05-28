#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

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

signals:

public Q_SLOTS:
    void loadScripts();
    void loadScript(const QString&);
    void stopScripts();
    void stopScript(const QString&);
    void prepareThis(QScriptEngine &);

private Q_SLOTS:
    void slotWSKeyChanged(const QString &key, const QString &value);

private:
    ScriptEngine();
    virtual ~ScriptEngine();

    ScriptEngine(const ScriptEngine&) {}
    ScriptEngine &operator =(const ScriptEngine&){}

    void registerStaticMembers(QScriptEngine &);
    void registerDynamicMembers(QScriptEngine &);

    QMap<QString, ScriptObject*> scripts;
};

Q_DECLARE_METATYPE(ScriptEngine*)

#endif // SCRIPTENGINE_H
