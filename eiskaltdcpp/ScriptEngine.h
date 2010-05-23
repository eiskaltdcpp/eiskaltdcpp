#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <QObject>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>

#include <QMetaType>

#include <QList>
#include <QStringList>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class ScriptEngine :
        public QObject,
        public dcpp::Singleton<ScriptEngine>
{
Q_OBJECT
friend class dcpp::Singleton<ScriptEngine>;

public:

signals:

public slots:
    void importExtension(const QString &name);

private:
    ScriptEngine();
    virtual ~ScriptEngine();

    ScriptEngine(const ScriptEngine&) {}
    ScriptEngine &operator =(const ScriptEngine&){}

    void prepareThis();
    void registerStaticMembers();
    void registerDynamicMembers();

    QScriptEngine engine;

    QStringList importedExtensions;
};

Q_DECLARE_METATYPE(ScriptEngine*)

#endif // SCRIPTENGINE_H
