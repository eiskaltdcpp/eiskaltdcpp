#include "ScriptEngine.h"

#include "MainWindow.h"
#include "Antispam.h"
#include "DownloadQueue.h"
#include "FavoriteHubs.h"
#include "FavoriteUsers.h"
#include "Notification.h"
#include "HubFrame.h"
#include "SearchFrame.h"

#include <QProcess>
#include <QDir>
#include <QtDebug>

#ifndef CLIENT_SCRIPTS_DIR
#define CLIENT_SCRIPTS_DIR
#endif

static QScriptValue shellExec(QScriptContext*, QScriptEngine*);
static QScriptValue staticMemberConstructor(QScriptContext*, QScriptEngine*);
static QScriptValue dynamicMemberConstructor(QScriptContext*, QScriptEngine*);

ScriptEngine::ScriptEngine() :
        QObject(NULL)
{
    prepareThis();
}

ScriptEngine::~ScriptEngine(){

}

void ScriptEngine::importExtension(const QString &name){
    static QStringList allowedExtensions = QStringList() << "qt.core" << "qt.gui" << "qt.network" << "qt.xml";

    if (!allowedExtensions.contains(name) || importedExtensions.contains(name))
        return;

    if (engine.importExtension(name).isUndefined())
        importedExtensions << name;
    else
        qDebug() << QString("ScriptEngine> Warning! %1 not found!").arg(name);
}

void ScriptEngine::prepareThis(){
    setObjectName("ScriptEngine");

    QScriptValue me = engine.newQObject(this);
    engine.globalObject().setProperty(objectName(), me);

    QScriptValue scriptsPath = QScriptValue(&engine, QString(CLIENT_SCRIPTS_DIR)+QDir::separator());
    engine.globalObject().setProperty("SCRIPTS_PATH", scriptsPath);

    QScriptValue shellEx = engine.newFunction(shellExec);
    engine.globalObject().setProperty("shellExec", shellEx);

    QScriptValue MW = engine.newQObject(MainWindow::getInstance());//MainWindow already initialized
    engine.globalObject().setProperty("MainWindow", MW);

    registerStaticMembers();
    registerDynamicMembers();
}

void ScriptEngine::registerStaticMembers(){
    static QStringList staticMembers = QStringList() << "AntiSpam" << "DownloadQueue" << "FavoriteHubs" << "FavoriteUsers"
                                       << "Notification";

    foreach( QString cl, staticMembers ) {
        qDebug() << QString("ScriptEngine> Register static class %1...").arg(cl).toAscii().constData();

        QScriptValue ct = engine.newFunction(staticMemberConstructor);
        ct.setProperty("className", cl);
        engine.globalObject().setProperty(cl, ct);
    }
}

void ScriptEngine::registerDynamicMembers(){
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
