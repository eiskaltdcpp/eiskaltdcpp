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

// Qt6-only: Bridge QObject that exposes native functions to QJSEngine scripts.
// Replaces QScriptEngine::newFunction() callbacks removed in Qt6.
// This header is only moc-processed in Qt6 builds (via CMakeLists.txt).

#include <QObject>
#include <QJSEngine>
#include <QJSValue>
#include <QStringList>

class ScriptBridge : public QObject {
    Q_OBJECT
public:
    explicit ScriptBridge(QJSEngine *engine, QObject *parent = nullptr);

    // Utility functions
    Q_INVOKABLE void shellExec(const QString &cmd, const QStringList &args = QStringList());
    Q_INVOKABLE QJSValue getMagnets(const QStringList &files);
    Q_INVOKABLE void printErr(const QString &msg);
    Q_INVOKABLE void Import(const QString &name);
    Q_INVOKABLE QJSValue Include(const QString &path);
    Q_INVOKABLE QJSValue Eval(const QString &code);
    Q_INVOKABLE QString parseChatLinks(const QString &text);
    Q_INVOKABLE QString parseMagnetAlias(const QString &text);

    // Static member constructors (singletons)
    Q_INVOKABLE QJSValue getStaticMember(const QString &className);

    // Dynamic member constructors (new instances)
    Q_INVOKABLE QJSValue createHubFrame(const QString &url, const QString &enc);
    Q_INVOKABLE QJSValue createSearchFrame();
    Q_INVOKABLE QJSValue createShellCommandRunner(const QString &cmd, const QStringList &args = QStringList());
    Q_INVOKABLE QJSValue createMainWindowScript();
    Q_INVOKABLE QJSValue createScriptWidget();
    Q_INVOKABLE QJSValue createIcon(const QString &path);

private:
    QJSEngine *m_engine;
};
