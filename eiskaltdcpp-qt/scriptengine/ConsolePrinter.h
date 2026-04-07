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

// Qt6-only: Helper QObject for ScriptConsole that provides print/printErr.
// Replaces the QScriptEngine callee().data() pattern removed in Qt6.
// This header is only moc-processed in Qt6 builds (via CMakeLists.txt).

#include <QObject>

class QTextEdit;

class ConsolePrinter : public QObject {
    Q_OBJECT
public:
    explicit ConsolePrinter(QTextEdit *edit, QObject *parent = nullptr);

    Q_INVOKABLE void print(const QString &text);
    Q_INVOKABLE void printErr(const QString &text);

private:
    QTextEdit *m_edit;
};
