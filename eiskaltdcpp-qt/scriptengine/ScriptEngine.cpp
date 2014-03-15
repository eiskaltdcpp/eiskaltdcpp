/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ScriptEngine.h"

#include "ArenaWidget.h"
#include "ArenaWidgetFactory.h"
#include "MainWindow.h"
#include "Antispam.h"
#include "DownloadQueue.h"
#include "FavoriteHubs.h"
#include "FavoriteUsers.h"
#include "Notification.h"
#include "HubFrame.h"
#include "HubManager.h"
#include "SearchFrame.h"
#include "ShellCommandRunner.h"
#include "WulforSettings.h"
#include "DebugHelper.h"
#include "GlobalTimer.h"

#include "scriptengine/ClientManagerScript.h"
#include "scriptengine/HashManagerScript.h"
#include "scriptengine/LogManagerScript.h"
#include "scriptengine/MainWindowScript.h"

#include <QProcess>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QScriptValueIterator>

#ifndef CLIENT_SCRIPTS_DIR
#define CLIENT_SCRIPTS_DIR
#endif

static QScriptValue shellExec(QScriptContext*, QScriptEngine*);
static QScriptValue getMagnets(QScriptContext*, QScriptEngine*);
static QScriptValue staticMemberConstructor(QScriptContext*, QScriptEngine*);
static QScriptValue dynamicMemberConstructor(QScriptContext*, QScriptEngine*);
static QScriptValue importExtension(QScriptContext*, QScriptEngine*);
static QScriptValue parseChatLinks(QScriptContext*, QScriptEngine*);
static QScriptValue parseMagnetAlias(QScriptContext*, QScriptEngine*);
static QScriptValue eval(QScriptContext*, QScriptEngine*);
static QScriptValue includeFile(QScriptContext*, QScriptEngine*);
static QScriptValue printErr(QScriptContext*, QScriptEngine*);
QScriptValue ScriptVarMapToScriptValue(QScriptEngine* eng, const VarMap& map);
void ScriptVarMapFromScriptValue( const QScriptValue& value, VarMap& map);

ScriptEngine::ScriptEngine() :
        QObject(NULL)
{
    DEBUG_BLOCK

    setObjectName("ScriptEngine");

    qRegisterMetaType<ArenaWidget::Flags>("Flags");
    qRegisterMetaType<ArenaWidget::Flags>("ArenaWidget::Flags");

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotWSKeyChanged(QString,QString)));
    connect(&watcher, SIGNAL(fileChanged(QString)), this, SLOT(slotScriptChanged(QString)));
    connect(GlobalTimer::getInstance(), SIGNAL(second()), this ,SLOT(slotProcessChangedFiles()));

    loadScripts();
}

ScriptEngine::~ScriptEngine(){
    DEBUG_BLOCK

    stopScripts();

    ClientManagerScript::deleteInstance();
    HashManagerScript::deleteInstance();
}

void ScriptEngine::loadScripts(){
    DEBUG_BLOCK

    QStringList enabled = QString(QByteArray::fromBase64(WSGET(WS_APP_ENABLED_SCRIPTS).toLatin1())).split("\n");

    for (const auto &s : enabled)
        loadScript(s);
}

void ScriptEngine::loadScript(const QString &path){
    DEBUG_BLOCK

    QFile f(path);

    if (!f.exists())
        return;

    stopScript(path);

    if (path.endsWith(".js", Qt::CaseInsensitive))
        loadJSScript(path);
#ifdef USE_QML
    else if (path.endsWith(".qml", Qt::CaseInsensitive))
        loadQMLScript(path);
#endif
}

void ScriptEngine::loadJSScript(const QString &file){
    DEBUG_BLOCK

    QFile f(file);

    if (!f.open(QIODevice::ReadOnly))
        return;

    ScriptObject *obj = new ScriptObject;
    obj->path = file;

    QTextStream stream(&f);
    QString data = stream.readAll();

    prepareThis(obj->engine);

    QScriptValue scriptPath = QScriptValue(&obj->engine, file.left(file.lastIndexOf(QDir::separator())) + QDir::separator());
    obj->engine.globalObject().setProperty("SCRIPT_PATH", scriptPath);

    scripts.insert(file, obj);

    watcher.addPath(file);

    obj->engine.evaluate(data, file);

    if (obj->engine.hasUncaughtException()){
        for (const auto &s : obj->engine.uncaughtExceptionBacktrace())
            qDebug() << s;
    }
}

#ifdef USE_QML
void ScriptEngine::loadQMLScript(const QString &file){
    DEBUG_BLOCK

    DeclarativeWidget *wgt = ArenaWidgetFactory().create<DeclarativeWidget, QString>(file);
}
#endif

void ScriptEngine::stopScripts(){
    DEBUG_BLOCK

    QMap<QString, ScriptObject*> s = scripts;
    auto it = s.begin();

    for (; it != s.end(); ++it)
        stopScript(it.key());

    scripts.clear();
}

void ScriptEngine::stopScript(const QString &path){
    DEBUG_BLOCK

    if (!scripts.contains(path))
        return;

    watcher.removePath(path);

    ScriptObject *obj = scripts.value(path);

    obj->engine.globalObject().property("deinit").call();

    if (obj->engine.isEvaluating())
        obj->engine.abortEvaluation();

    scripts.remove(path);

    if (obj->engine.hasUncaughtException())
        qDebug() << obj->engine.uncaughtExceptionBacktrace();

    delete obj;
}

void ScriptEngine::slotProcessChangedFiles() {
    DEBUG_BLOCK

    for (const QString &file : changedFiles)
        emit scriptChanged(file);

    changedFiles.clear();
}


void ScriptEngine::prepareThis(QScriptEngine &engine){
    DEBUG_BLOCK

    QScriptValue me = engine.newQObject(&engine);
    engine.globalObject().setProperty(objectName(), me);

#ifndef WIN32
    QScriptValue scriptsPath = QScriptValue(&engine, QString(CLIENT_SCRIPTS_DIR)+QDir::separator());
#else
    QScriptValue scriptsPath = QScriptValue(&engine,
        qApp->applicationDirPath()+QDir::separator()+CLIENT_SCRIPTS_DIR+QDir::separator() );
#endif//WIN32
    engine.globalObject().setProperty("SCRIPTS_PATH", scriptsPath, QScriptValue::ReadOnly);

    QScriptValue shellEx = engine.newFunction(shellExec);
    engine.globalObject().setProperty("shellExec", shellEx, QScriptValue::ReadOnly);

    QScriptValue getMagnet = engine.newFunction(getMagnets);
    engine.globalObject().setProperty("getMagnets", getMagnet, QScriptValue::ReadOnly);

    QScriptValue printE = engine.newFunction(printErr);
    engine.globalObject().setProperty("printErr", printE, QScriptValue::ReadOnly);

    QScriptValue import = engine.newFunction(importExtension);
    engine.globalObject().setProperty("Import", import, QScriptValue::ReadOnly);

    QScriptValue include = engine.newFunction(includeFile);
    engine.globalObject().setProperty("Include", include, QScriptValue::ReadOnly);

    QScriptValue evalStr = engine.newFunction(eval);
    engine.globalObject().setProperty("Eval", evalStr, QScriptValue::ReadOnly);

    QScriptValue MW = engine.newQObject(MainWindow::getInstance());//MainWindow already initialized
    engine.globalObject().setProperty("MainWindow", MW, QScriptValue::ReadOnly);

    QScriptValue WU = engine.newQObject(WulforUtil::getInstance());//WulforUtil already initialized
    engine.globalObject().setProperty("WulforUtil", WU, QScriptValue::ReadOnly);

    QScriptValue WS = engine.newQObject(WulforSettings::getInstance());//WulforSettings already initialized
    engine.globalObject().setProperty("WulforSettings", WS, QScriptValue::ReadOnly);

    QScriptValue linkParser = engine.newObject();
    engine.globalObject().setProperty("LinkParser", linkParser, QScriptValue::ReadOnly);
    QScriptValue linkParser_parse = engine.newFunction(parseChatLinks);
    QScriptValue linkParser_parseMagnet = engine.newFunction(parseMagnetAlias);
    linkParser.setProperty("parse", linkParser_parse, QScriptValue::ReadOnly);
    linkParser.setProperty("parseMagnetAlias", linkParser_parseMagnet, QScriptValue::ReadOnly);

    QScriptValue widgetManager = engine.newQObject(ArenaWidgetManager::getInstance());
    engine.globalObject().setProperty("WidgetManager", widgetManager, QScriptValue::ReadOnly);

    qScriptRegisterSequenceMetaType< QList<QObject*> >(&engine);
    qScriptRegisterMetaType<VarMap>(&engine, ScriptVarMapToScriptValue, ScriptVarMapFromScriptValue);
    qScriptRegisterMetaType<VarMap>(&engine, ScriptVarMapToScriptValue, ScriptVarMapFromScriptValue);

    engine.globalObject().setProperty("LinkParser", linkParser, QScriptValue::ReadOnly);

    registerStaticMembers(engine);
    registerDynamicMembers(engine);

    engine.setProcessEventsInterval(100);
}

void ScriptEngine::registerStaticMembers(QScriptEngine &engine){
    DEBUG_BLOCK

    static QStringList staticMembers = QStringList() << "AntiSpam"          << "DownloadQueue"  << "FavoriteHubs"
                                                     << "Notification"      << "HubManager"     << "ClientManagerScript"
                                                     << "LogManagerScript"  << "FavoriteUsers"  << "HashManagerScript"
                                                     << "WulforUtil"        << "WulforSettings";

    for (const auto &cl : staticMembers) {
        QScriptValue ct = engine.newFunction(staticMemberConstructor);
        ct.setProperty("className", cl, QScriptValue::ReadOnly);
        engine.globalObject().setProperty(cl, ct, QScriptValue::ReadOnly);
    }
}

void ScriptEngine::registerDynamicMembers(QScriptEngine &engine){
    DEBUG_BLOCK

    static QStringList dynamicMembers = QStringList() << "HubFrame" << "SearchFrame" << "ShellCommandRunner" << "MainWindowScript"
                                                      << "ScriptWidget";

    for (const auto &cl : dynamicMembers) {
        QScriptValue ct = engine.newFunction(dynamicMemberConstructor);
        ct.setProperty("className", cl, QScriptValue::ReadOnly);
        engine.globalObject().setProperty(cl, ct, QScriptValue::ReadOnly);
    }
}

void ScriptEngine::slotWSKeyChanged(const QString &key, const QString &value){
    DEBUG_BLOCK

    if (key == WS_APP_ENABLED_SCRIPTS){
        QStringList enabled = QString(QByteArray::fromBase64(value.toLatin1())).split("\n", QString::SkipEmptyParts);
        QMap<QString, ScriptObject*>::iterator it;

        for (const auto &script : enabled){
            it = scripts.find(script);

            if (it == scripts.end())
                loadScript(script);
        }

        QMap<QString, ScriptObject*> copy = scripts;
        for (it = copy.begin(); it != copy.end(); ++it){
            if (!enabled.contains(it.key()))
                stopScript(it.key());
        }
    }
}

void ScriptEngine::slotScriptChanged(const QString &script){
    DEBUG_BLOCK

    if (!QFile::exists(script))
        stopScript(script);
    else if (!changedFiles.contains(script)){
        changedFiles.push_back(script);
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

static QScriptValue getMagnets(QScriptContext *ctx, QScriptEngine *engine){
    Q_UNUSED(engine);

    if (ctx->argumentCount() < 1)
        return QScriptValue();

    QStringList files;

    for (int i = 0; i < ctx->argumentCount(); i++)
        files.push_back(ctx->argument(i).toString());

    QStringList magnets;

    for (const auto &f : files){
        QFile file(f);

        if (!file.exists())
            continue;

        const dcpp::TTHValue *tth = dcpp::HashManager::getInstance()->getFileTTHif(_tq(f));

        if (tth)
            magnets.push_back(WulforUtil::getInstance()->makeMagnet(f.split(QDir::separator(), QString::SkipEmptyParts).last(),
                                                                    file.size(),
                                                                    _q(tth->toBase32())
                                                                    ));
    }

    QScriptValue array = engine->newArray(magnets.size());

    for (int i = 0; i < magnets.length(); i++)
        array.setProperty(i, QScriptValue(magnets.at(i)));

    return array;
}

QScriptValue wulforUtilQObjectConstructor(QScriptContext *context, QScriptEngine *engine){
    Q_UNUSED(context);
    return engine->newQObject(WulforUtil::getInstance());
}

static QScriptValue staticMemberConstructor(QScriptContext *context, QScriptEngine *engine){
    QScriptValue self = context->callee();
    const QString className = self.property("className").toString();

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
        obj = qobject_cast<QObject*>(ArenaWidgetFactory().create<dcpp::Singleton, DownloadQueue>());
    }
    else if (className == "FavoriteHubs"){
        obj = qobject_cast<QObject*>(ArenaWidgetFactory().create<dcpp::Singleton, FavoriteHubs>());
    }
    else if (className == "FavoriteUsers"){
        obj = qobject_cast<QObject*>(ArenaWidgetFactory().create<dcpp::Singleton, FavoriteUsers>());
    }
    else if (className == "Notification"){
        if (Notification::getInstance()){
            engine->globalObject().setProperty("NOTIFY_ANY", (int)Notification::ANY);

            obj = qobject_cast<QObject*>(Notification::getInstance());
        }
    }
    else if (className == "HubManager"){
        obj = qobject_cast<QObject*>(HubManager::getInstance());
    }
    else if (className == "ClientManagerScript"){
        if (!ClientManagerScript::getInstance()){
            ClientManagerScript::newInstance();

            ClientManagerScript::getInstance()->moveToThread(MainWindow::getInstance()->thread());
        }

        obj = qobject_cast<QObject*>(ClientManagerScript::getInstance());
    }
    else if (className == "HashManagerScript"){
        if (!HashManagerScript::getInstance()){
            HashManagerScript::newInstance();

            HashManagerScript::getInstance()->moveToThread(MainWindow::getInstance()->thread());
        }

        obj = qobject_cast<QObject*>(HashManagerScript::getInstance());
    }
    else if (className == "LogManagerScript"){
        if (!LogManagerScript::getInstance()){
            LogManagerScript::newInstance();

            LogManagerScript::getInstance()->moveToThread(MainWindow::getInstance()->thread());
        }

        obj = qobject_cast<QObject*>(LogManagerScript::getInstance());
    }
    else if (className == "WulforUtil"){
        QScriptValue ctor = engine->newFunction(wulforUtilQObjectConstructor);
        QScriptValue metaObject = engine->newQMetaObject(&WulforUtil::staticMetaObject, ctor);

        return metaObject;
    }
    else if (className == "WulforSettings")
        obj = qobject_cast<QObject*>(WulforSettings::getInstance());

    return engine->newQObject(obj);
}

static QScriptValue dynamicMemberConstructor(QScriptContext *context, QScriptEngine *engine){
    QScriptValue self = context->callee();
    const QString className = self.property("className").toString();

    QObject *obj = NULL;

    if (className == "HubFrame"){
        if (context->argumentCount() == 2){
            QString hub = context->argument(0).toString();
            QString enc = context->argument(1).toString();

            HubFrame *fr = ArenaWidgetFactory().create<HubFrame, MainWindow*, QString, QString>(MainWindow::getInstance(), hub, enc);

            obj = qobject_cast<QObject*>(fr);
        }
    }
    else if (className == "SearchFrame"){
        SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame>();

        obj = qobject_cast<QObject*>(fr);
    }
    else if (className == "ShellCommandRunner"){
        if (context->argumentCount() >= 1){
            QString cmd = context->argument(0).toString();
            QStringList args = QStringList();

            for (int i = 1; i < context->argumentCount(); i++)
                args.push_back(context->argument(i).toString());

            ShellCommandRunner *runner = new ShellCommandRunner(cmd, args, MainWindow::getInstance());
            runner->connect(runner, SIGNAL(finished(bool,QString)), runner, SLOT(deleteLater()));

            obj = qobject_cast<QObject*>(runner);
        }
    }
    else if (className == "MainWindowScript"){
        obj = qobject_cast<QObject*>(new MainWindowScript(engine, MainWindow::getInstance()));
    }
    else if (className == "ScriptWidget"){
        obj = qobject_cast<QObject*>(ArenaWidgetFactory().create<ScriptWidget>());
    }

    return engine->newQObject(obj, QScriptEngine::AutoOwnership);
}

static QScriptValue importExtension(QScriptContext *context, QScriptEngine *engine){
    if (context->argumentCount() != 1)
        return QScriptValue();

    const QString name = context->argument(0).toString();

    if (!engine->importExtension(name).isUndefined())
        qDebug() << QString("ScriptEngine> Warning! %1 not found!").arg(name).toLatin1().constData();

    return QScriptValue();
}

static QScriptValue parseChatLinks(QScriptContext *ctx, QScriptEngine *engine){
    if (ctx->argumentCount() != 1)
        return engine->undefinedValue();

    return QScriptValue(HubFrame::LinkParser::parseForLinks(ctx->argument(0).toString(), 0));
}

static QScriptValue parseMagnetAlias(QScriptContext *ctx, QScriptEngine *engine) {
    if (ctx->argumentCount() != 1)
        return engine->undefinedValue();

    QString output = ctx->argument(0).toString();

    HubFrame::LinkParser::parseForMagnetAlias(output);

    return output;
}

static QScriptValue eval(QScriptContext *ctx, QScriptEngine *engine){
    if (ctx->argumentCount() < 1)
        return engine->undefinedValue();

    QString content = ctx->argument(0).toString();
    QScriptValue ret = engine->evaluate(content);

    if (engine->hasUncaughtException())
        qDebug() << engine->uncaughtExceptionBacktrace();

    return ret;
}

static QScriptValue includeFile(QScriptContext *ctx, QScriptEngine *engine){
    if (ctx->argumentCount() < 1)
        return engine->undefinedValue();

    QString path = ctx->argument(0).toString();

    QFile f(path);

    if (!(f.exists() && f.open(QIODevice::ReadOnly)))
        return engine->undefinedValue();

    QTextStream stream(&f);
    QString data = stream.readAll();

    QScriptValue ret = engine->evaluate(data);

    if (engine->hasUncaughtException())
        qDebug() << engine->uncaughtExceptionBacktrace();

    return ret;
}

QScriptValue printErr(QScriptContext *ctx, QScriptEngine *engine){
        if (ctx->argumentCount() < 1)
        return engine->undefinedValue();

    QString out = ctx->argument(0).toString();

    for (int i = 1; i < ctx->argumentCount(); i++)
        out = out.arg(ctx->argument(i).toString());

    qWarning() << qPrintable(out);

    return engine->undefinedValue();
}

QScriptValue ScriptVarMapToScriptValue(QScriptEngine* eng, const VarMap& map)
{
    QScriptValue a = eng->newObject();
    auto it = map.begin();

    for(; it != map.end(); ++it) {
        QString prop = it.key();

        a.setProperty(prop, qScriptValueFromValue(eng, it.value()));
    }

    return a;
}

void ScriptVarMapFromScriptValue( const QScriptValue& value, VarMap& map){
    QScriptValueIterator itr(value);

    while(itr.hasNext()){
       itr.next();
       map[itr.name()] = qscriptvalue_cast<VarMap::mapped_type>(itr.value());
    }
}
