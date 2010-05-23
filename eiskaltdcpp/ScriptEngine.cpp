#include "ScriptEngine.h"

#include "MainWindow.h"
#include "Antispam.h"
#include "DownloadQueue.h"
#include "FavoriteHubs.h"
#include "FavoriteUsers.h"
#include "Notification.h"
#include "HubFrame.h"
#include "SearchFrame.h"
#include "WulforSettings.h"

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QtDebug>

#ifndef CLIENT_SCRIPTS_DIR
#define CLIENT_SCRIPTS_DIR
#endif

static QScriptValue shellExec(QScriptContext*, QScriptEngine*);
static QScriptValue staticMemberConstructor(QScriptContext*, QScriptEngine*);
static QScriptValue dynamicMemberConstructor(QScriptContext*, QScriptEngine*);
static QScriptValue importExtension(QScriptContext*, QScriptEngine*);

ScriptEngine::ScriptEngine() :
        QObject(NULL)
{
    loadScripts();
}

ScriptEngine::~ScriptEngine(){
    stopScripts();
}

void ScriptEngine::loadScripts(){
    QStringList enabled = QString(QByteArray::fromBase64(WSGET(WS_APP_ENABLED_SCRIPTS).toAscii())).split("\n");

    foreach (QString s, enabled)
        loadScript(s);
}

void ScriptEngine::loadScript(const QString &path){
    QFile f(path);

    if (!(f.exists() && f.open(QIODevice::ReadOnly)))
        return;

    ScriptObject *obj = new ScriptObject;
    obj->path = path;

    QTextStream stream(&f);
    QString data = stream.readAll();

    prepareThis(obj->engine);

    scripts.insert(path, obj);

    qDebug() << QString("ScriptEngine> Starting %1 ...").arg(path).toAscii().constData();

    obj->engine.evaluate(data);

    if (obj->engine.hasUncaughtException()){
        foreach (QString s, obj->engine.uncaughtExceptionBacktrace())
            qDebug() << s;
    }
}

void ScriptEngine::stopScripts(){
    QMap<QString, ScriptObject*> s = scripts;
    QMap<QString, ScriptObject*>::iterator it = s.begin();

    for (; it != s.end(); ++it)
        stopScript(it.key());

    scripts.clear();
}

void ScriptEngine::stopScript(const QString &path){
    if (!scripts.contains(path))
        return;

    ScriptObject *obj = scripts.value(path);

    if (obj->engine.isEvaluating())
        obj->engine.abortEvaluation();

    scripts.remove(path);

    qDebug() << QString("ScriptEngine> Stopping %1 ...").arg(path).toAscii().constData();

    delete obj;
}

void ScriptEngine::prepareThis(QScriptEngine &engine){
    setObjectName("ScriptEngine");

    QScriptValue me = engine.newQObject(this);
    engine.globalObject().setProperty(objectName(), me);

    QScriptValue scriptsPath = QScriptValue(&engine, QString(CLIENT_SCRIPTS_DIR)+QDir::separator());
    engine.globalObject().setProperty("SCRIPTS_PATH", scriptsPath);

    QScriptValue shellEx = engine.newFunction(shellExec);
    engine.globalObject().setProperty("shellExec", shellEx);

    QScriptValue import = engine.newFunction(importExtension);
    engine.globalObject().setProperty("Import", import);

    QScriptValue MW = engine.newQObject(MainWindow::getInstance());//MainWindow already initialized
    engine.globalObject().setProperty("MainWindow", MW);

    registerStaticMembers(engine);
    registerDynamicMembers(engine);

    engine.setProcessEventsInterval(100);
}

void ScriptEngine::registerStaticMembers(QScriptEngine &engine){
    static QStringList staticMembers = QStringList() << "AntiSpam" << "DownloadQueue" << "FavoriteHubs" << "FavoriteUsers"
                                       << "Notification";

    foreach( QString cl, staticMembers ) {
        qDebug() << QString("ScriptEngine> Register static class %1...").arg(cl).toAscii().constData();

        QScriptValue ct = engine.newFunction(staticMemberConstructor);
        ct.setProperty("className", cl);
        engine.globalObject().setProperty(cl, ct);
    }
}

void ScriptEngine::registerDynamicMembers(QScriptEngine &engine){
    static QStringList dynamicMembers = QStringList() << "HubFrame" << "SearchFrame";

    foreach( QString cl, dynamicMembers ) {
        qDebug() << QString("ScriptEngine> Register dynamic class %1...").arg(cl).toAscii().constData();

        QScriptValue ct = engine.newFunction(dynamicMemberConstructor);
        ct.setProperty("className", cl);
        engine.globalObject().setProperty(cl, ct);
    }
}

static QScriptValue shellExec(QScriptContext *ctx, QScriptEngine *engine){
    Q_UNUSED(engine);

    if (ctx->argumentCount() < 1)
        return QScriptValue();

    QString cmd = ctx->argument(0).toString();
    QStringList args = QStringList();

    for (int i = 1; i < ctx->argumentCount(); i++)
        args.push_back(ctx->argument(i).toString());

    QProcess *process = new QProcess();
    process->connect(process, SIGNAL(finished(int)), process, SLOT(deleteLater()));
    process->start(cmd, args);

    return QScriptValue();
}


static QScriptValue staticMemberConstructor(QScriptContext *context, QScriptEngine *engine){
    QScriptValue self = context->callee();
    const QString className = self.property("className").toString();

    qDebug() << QString("ScriptEngine> Constructing %1...").arg(className).toAscii().constData();

    QObject *obj = NULL;

    if (className == "AntiSpam"){
        if (!AntiSpam::getInstance()){
            AntiSpam::newInstance();
            AntiSpam::getInstance()->loadSettings();
            AntiSpam::getInstance()->loadLists();
        }

        obj = qobject_cast<QObject*>(AntiSpam::getInstance());
    }
    else if (className == "DownloadQueue"){
        if (!DownloadQueue::getInstance())
            DownloadQueue::newInstance();

        obj = qobject_cast<QObject*>(DownloadQueue::getInstance());
    }
    else if (className == "FavoriteHubs"){
        if (!FavoriteHubs::getInstance())
            FavoriteHubs::newInstance();

        obj = qobject_cast<QObject*>(FavoriteHubs::getInstance());
    }
    else if (className == "FavoriteUsers"){
        if (!FavoriteUsers::getInstance())
            FavoriteUsers::newInstance();

        obj = qobject_cast<QObject*>(FavoriteUsers::getInstance());
    }

    return engine->newQObject(obj);
}

static QScriptValue dynamicMemberConstructor(QScriptContext *context, QScriptEngine *engine){
    QScriptValue self = context->callee();
    const QString className = self.property("className").toString();

    qDebug() << QString("ScriptEngine> Constructing %1...").arg(className).toAscii().constData();

    QObject *obj = NULL;

    if (className == "HubFrame"){
        if (context->argumentCount() == 2){
            QString hub = context->argument(0).toString();
            QString enc = context->argument(1).toString();

            HubFrame *fr = new HubFrame(MainWindow::getInstance(), hub, enc);
            fr->setAttribute(Qt::WA_DeleteOnClose);

            MainWindow::getInstance()->addArenaWidget(fr);
            MainWindow::getInstance()->mapWidgetOnArena(fr);

            MainWindow::getInstance()->addArenaWidgetOnToolbar(fr);

            obj = qobject_cast<QObject*>(fr);
        }
    }
    else if (className == "SearchFrame"){
        SearchFrame *fr = new SearchFrame();
        fr->setAttribute(Qt::WA_DeleteOnClose);

        obj = qobject_cast<QObject*>(fr);
    }

    return engine->newQObject(obj);
}

static QScriptValue importExtension(QScriptContext *context, QScriptEngine *engine){
    static QStringList allowedExtensions = QStringList() << "qt.core" << "qt.gui" << "qt.network" << "qt.xml" << "qt.dbus";

    if (context->argumentCount() != 1)
        return QScriptValue();

    const QString name = context->argument(0).toString();

    if (!allowedExtensions.contains(name))
        return QScriptValue();

    if (!engine->importExtension(name).isUndefined())
        qDebug() << QString("ScriptEngine> Warning! %1 not found!").arg(name);

    return QScriptValue();
}
