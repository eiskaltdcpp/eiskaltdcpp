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

#include "ScriptEngine.h"
#include "QtContext.h"
#include "QtContextAware.h"
#include "dcpp/DCPlusPlus.h"

#include "ArenaWidget.h"
#include "ArenaWidgetFactory.h"
#include "ArenaWidgetManager.h"
#include "QtContext.h"
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

#ifndef CLIENT_SCRIPTS_DIR
#define CLIENT_SCRIPTS_DIR
#endif

// ============================================================
// ScriptBridge implementation
// ============================================================

ScriptBridge::ScriptBridge(QJSEngine *engine, QObject *parent)
    : QObject(parent), m_engine(engine) {}

void ScriptBridge::shellExec(const QString &cmd, const QStringList &args) {
    QProcess *process = new QProcess();
    QObject::connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
                     process, &QProcess::deleteLater);
    process->start(cmd, args);
}

QJSValue ScriptBridge::getMagnets(const QStringList &files) {
    QStringList magnets;
    for (const auto &f : files) {
        QFile file(f);
        if (!file.exists())
            continue;
        const dcpp::TTHValue *tth = qtCtx()->dcCtx().getHashManager()->getFileTTHif(_tq(f));
        if (tth)
            magnets.push_back(qtCtx()->wulforUtil()->makeMagnet(
                f.split(QDir::separator(), Qt::SkipEmptyParts).last(),
                file.size(), _q(tth->toBase32())));
    }
    QJSValue array = m_engine->newArray(magnets.size());
    for (int i = 0; i < magnets.length(); i++)
        array.setProperty(i, QJSValue(magnets.at(i)));
    return array;
}

void ScriptBridge::printErr(const QString &msg) {
    qWarning() << qPrintable(msg);
}

void ScriptBridge::Import(const QString &name) {
    qWarning() << "ScriptBridge> Import() is not supported in Qt6. Ignoring:" << name;
}

QJSValue ScriptBridge::Include(const QString &path) {
    QFile f(path);
    if (!(f.exists() && f.open(QIODevice::ReadOnly)))
        return QJSValue();
    QTextStream stream(&f);
    QString data = stream.readAll();
    QJSValue ret = m_engine->evaluate(data, path);
    if (ret.isError())
        qDebug() << "ScriptBridge> Include error:" << ret.property("stack").toString();
    return ret;
}

QJSValue ScriptBridge::Eval(const QString &code) {
    QJSValue ret = m_engine->evaluate(code);
    if (ret.isError())
        qDebug() << "ScriptBridge> Eval error:" << ret.property("stack").toString();
    return ret;
}

QString ScriptBridge::parseChatLinks(const QString &text) {
    return HubFrame::LinkParser::parseForLinks(text, false);
}

QString ScriptBridge::parseMagnetAlias(const QString &text) {
    QString output = text;
    HubFrame::LinkParser::parseForMagnetAlias(output);
    return output;
}

QJSValue ScriptBridge::getStaticMember(const QString &className) {
    QObject *obj = nullptr;
    if (className == "AntiSpam") {
        if (!qtCtx()->antiSpam()) {
            qtCtx()->createAntiSpam();
            qtCtx()->antiSpam()->loadSettings();
            qtCtx()->antiSpam()->loadLists();
        }
        obj = qobject_cast<QObject*>(qtCtx()->antiSpam());
    }
    else if (className == "DownloadQueue") {
        if (!qtCtx()->downloadQueue()) {
            qtCtx()->createDownloadQueue();
            qtCtx()->arenaWidgetManager()->add(qtCtx()->downloadQueue());
        }
        obj = qobject_cast<QObject*>(qtCtx()->downloadQueue());
    }
    else if (className == "FavoriteHubs") {
        if (!qtCtx()->favoriteHubs()) {
            qtCtx()->createFavoriteHubs();
            qtCtx()->arenaWidgetManager()->add(qtCtx()->favoriteHubs());
        }
        obj = qobject_cast<QObject*>(qtCtx()->favoriteHubs());
    }
    else if (className == "FavoriteUsers") {
        if (!qtCtx()->favoriteUsers()) {
            qtCtx()->createFavoriteUsers();
            qtCtx()->arenaWidgetManager()->add(qtCtx()->favoriteUsers());
        }
        obj = qobject_cast<QObject*>(qtCtx()->favoriteUsers());
    }
    else if (className == "Notification") {
        if (qtCtx()->notification()) {
            m_engine->globalObject().setProperty("NOTIFY_ANY", (int)Notification::ANY);
            obj = qobject_cast<QObject*>(qtCtx()->notification());
        }
    }
    else if (className == "HubManager")
        obj = qobject_cast<QObject*>(qtCtx()->hubManager());
    else if (className == "ClientManagerScript") {
        if (!qtCtx()->clientManagerScript()) {
            qtCtx()->createClientManagerScript();
            qtCtx()->clientManagerScript()->moveToThread(qtCtx()->mainWindow()->thread());
        }
        obj = qobject_cast<QObject*>(qtCtx()->clientManagerScript());
    }
    else if (className == "HashManagerScript") {
        if (!qtCtx()->hashManagerScript()) {
            qtCtx()->createHashManagerScript();
            qtCtx()->hashManagerScript()->moveToThread(qtCtx()->mainWindow()->thread());
        }
        obj = qobject_cast<QObject*>(qtCtx()->hashManagerScript());
    }
    else if (className == "LogManagerScript") {
        if (!qtCtx()->logManagerScript()) {
            qtCtx()->createLogManagerScript();
            qtCtx()->logManagerScript()->moveToThread(qtCtx()->mainWindow()->thread());
        }
        obj = qobject_cast<QObject*>(qtCtx()->logManagerScript());
    }
    else if (className == "WulforUtil")
        obj = qobject_cast<QObject*>(qtCtx()->wulforUtil());
    else if (className == "WulforSettings")
        obj = qobject_cast<QObject*>(qtCtx()->settings());

    return m_engine->newQObject(obj);
}

QJSValue ScriptBridge::createHubFrame(const QString &url, const QString &enc) {
    HubFrame *fr = ArenaWidgetFactory().create<HubFrame, MainWindow*, QString, QString>(
        qtCtx()->mainWindow(), url, enc);
    return m_engine->newQObject(qobject_cast<QObject*>(fr));
}

QJSValue ScriptBridge::createSearchFrame() {
    SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame>();
    return m_engine->newQObject(qobject_cast<QObject*>(fr));
}

QJSValue ScriptBridge::createShellCommandRunner(const QString &cmd, const QStringList &args) {
    ShellCommandRunner *runner = new ShellCommandRunner(cmd, args, qtCtx()->mainWindow());
    QObject::connect(runner, qOverload<bool, const QString &>(&ShellCommandRunner::finished), runner, &QObject::deleteLater);
    return m_engine->newQObject(qobject_cast<QObject*>(runner));
}

QJSValue ScriptBridge::createMainWindowScript() {
    return m_engine->newQObject(qobject_cast<QObject*>(
        new MainWindowScript(m_engine, qtCtx()->mainWindow())));
}

QJSValue ScriptBridge::createScriptWidget() {
    return m_engine->newQObject(qobject_cast<QObject*>(
        ArenaWidgetFactory().create<ScriptWidget>()));
}

QJSValue ScriptBridge::createIcon(const QString &path) {
    return m_engine->toScriptValue(QIcon(path));
}

ScriptEngine::ScriptEngine(dcpp::DCContext& ctx) :
        QtContextAware(ctx),
        QObject(nullptr)
{
    DEBUG_BLOCK

    setObjectName("ScriptEngine");

    qRegisterMetaType<ArenaWidget::Flags>("Flags");
    qRegisterMetaType<ArenaWidget::Flags>("ArenaWidget::Flags");

    connect(qtCtx()->settings(), &WulforSettings::strValueChanged, this, &ScriptEngine::slotWSKeyChanged);
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &ScriptEngine::slotScriptChanged);
    connect(qtCtx()->globalTimer(), &GlobalTimer::second, this, &ScriptEngine::slotProcessChangedFiles);

    loadScripts();
}

ScriptEngine::~ScriptEngine(){
    DEBUG_BLOCK

    stopScripts();
}

void ScriptEngine::loadScripts(){
    DEBUG_BLOCK

    QStringList enabled = QString(QByteArray::fromBase64(qtCtx()->settings()->getStr(WS_APP_ENABLED_SCRIPTS).toLatin1())).split("\n");

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

    QJSValue scriptPath = QJSValue(file.left(file.lastIndexOf(QDir::separator())) + QDir::separator());
    obj->engine.globalObject().setProperty("SCRIPT_PATH", scriptPath);

    scripts.insert(file, obj);
    watcher.addPath(file);

    QJSValue result = obj->engine.evaluate(data, file);
    if (result.isError()) {
        qDebug() << "ScriptEngine> Error loading" << file << ":"
                 << result.property("stack").toString();
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

    for (const QString &key : scripts.keys()) {
        stopScript(key);
    }

    scripts.clear();
}

void ScriptEngine::stopScript(const QString &path){
    DEBUG_BLOCK

    if (!scripts.contains(path))
        return;

    watcher.removePath(path);

    ScriptObject *obj = scripts.value(path);

    QJSValue deinit = obj->engine.globalObject().property("deinit");
    if (deinit.isCallable())
        deinit.call();

    obj->engine.setInterrupted(true);

    scripts.remove(path);

    delete obj->bridge;
    delete obj;
}

void ScriptEngine::slotProcessChangedFiles() {
    DEBUG_BLOCK

    for (const QString &file : changedFiles)
        emit scriptChanged(file);

    changedFiles.clear();
}

// ============ prepareThis using ScriptBridge ============

void ScriptEngine::prepareThis(QJSEngine &engine){
    DEBUG_BLOCK

    ScriptBridge *bridge = new ScriptBridge(&engine);
    QJSValue bridgeVal = engine.newQObject(bridge);
    engine.globalObject().setProperty("_bridge", bridgeVal);

    // Store bridge in the corresponding ScriptObject (if any) for cleanup
    for (auto it = scripts.begin(); it != scripts.end(); ++it) {
        if (&it.value()->engine == &engine) {
            it.value()->bridge = bridge;
            break;
        }
    }

#ifndef _WIN32
    QJSValue scriptsPath = QJSValue(QString(CLIENT_SCRIPTS_DIR) + QDir::separator());
#else
    QJSValue scriptsPath = QJSValue(
        qApp->applicationDirPath() + QDir::separator() + CLIENT_SCRIPTS_DIR + QDir::separator());
#endif
    engine.globalObject().setProperty("SCRIPTS_PATH", scriptsPath);

    // Expose singleton QObjects directly
    engine.globalObject().setProperty("MainWindow",
        engine.newQObject(qtCtx()->mainWindow()));
    engine.globalObject().setProperty("WulforUtil",
        engine.newQObject(qtCtx()->wulforUtil()));
    engine.globalObject().setProperty("WulforSettings",
        engine.newQObject(qtCtx()->settings()));
    engine.globalObject().setProperty("WidgetManager",
        engine.newQObject(qtCtx()->arenaWidgetManager()));

    // LinkParser namespace via evaluate
    engine.evaluate(QStringLiteral(
        "var LinkParser = {"
        "  parse: function(text) { return _bridge.parseChatLinks(text); },"
        "  parseMagnetAlias: function(text) { return _bridge.parseMagnetAlias(text); }"
        "};"
    ));

    // Create backward-compatible JS function wrappers for utility functions
    engine.evaluate(QStringLiteral(
        "function shellExec(cmd) {"
        "  var args = [];"
        "  for (var i = 1; i < arguments.length; i++) args.push(arguments[i]);"
        "  _bridge.shellExec(cmd, args);"
        "}"
        "function getMagnets() {"
        "  var files = [];"
        "  for (var i = 0; i < arguments.length; i++) files.push(arguments[i]);"
        "  return _bridge.getMagnets(files);"
        "}"
        "function printErr(msg) {"
        "  _bridge.printErr(msg);"
        "}"
        "function Import(name) {"
        "  _bridge.Import(name);"
        "}"
        "function Include(path) {"
        "  return _bridge.Include(path);"
        "}"
        "function Eval(code) {"
        "  return _bridge.Eval(code);"
        "}"
    ));

    // Create backward-compatible constructor wrappers (work with 'new' keyword)
    registerStaticMembers(engine);
    registerDynamicMembers(engine);
}

void ScriptEngine::registerStaticMembers(QJSEngine &engine){
    DEBUG_BLOCK

    static QStringList staticMembers = QStringList()
        << "AntiSpam" << "DownloadQueue" << "FavoriteHubs"
        << "Notification" << "HubManager" << "ClientManagerScript"
        << "LogManagerScript" << "FavoriteUsers" << "HashManagerScript";

    // WulforUtil and WulforSettings are already exposed as direct QObject references
    for (const auto &cl : staticMembers) {
        QString wrapper = QStringLiteral("function %1() { return _bridge.getStaticMember('%1'); }").arg(cl);
        engine.evaluate(wrapper);
    }
}

void ScriptEngine::registerDynamicMembers(QJSEngine &engine){
    DEBUG_BLOCK

    // Create JS constructor functions that delegate to bridge factory methods
    engine.evaluate(QStringLiteral(
        "function HubFrame(url, enc) { return _bridge.createHubFrame(url, enc); }"
        "function SearchFrame() { return _bridge.createSearchFrame(); }"
        "function ShellCommandRunner(cmd) {"
        "  var args = [];"
        "  for (var i = 1; i < arguments.length; i++) args.push(arguments[i]);"
        "  return _bridge.createShellCommandRunner(cmd, args);"
        "}"
        "function MainWindowScript() { return _bridge.createMainWindowScript(); }"
        "function ScriptWidget() { return _bridge.createScriptWidget(); }"
        "function QIcon(path) { return _bridge.createIcon(path); }"
    ));
}

void ScriptEngine::slotWSKeyChanged(const QString &key, const QString &value){
    DEBUG_BLOCK

    if (key == WS_APP_ENABLED_SCRIPTS){
        QStringList enabled = QString(QByteArray::fromBase64(value.toLatin1())).split("\n", Qt::SkipEmptyParts);
        QMap<QString, ScriptObject*>::iterator it;

        for (const auto &script : enabled){
            it = scripts.find(script);

            if (it == scripts.end())
                loadScript(script);
        }

        QMap<QString, ScriptObject*> copy = scripts;
        for (const QString &key : copy.keys()){
            if (!enabled.contains(key)) {
                stopScript(key);
            }
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
