/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>
#include <QDialog>

#include "ui_UIDialogScriptConsole.h"

class ScriptConsole : public QDialog,
                      private Ui::DialogScriptConsole
{
Q_OBJECT
public:
    explicit ScriptConsole(QWidget *parent = 0);

private Q_SLOTS:
    void startEvaluation();
    void stopEvaluation();

private:
    QScriptEngine engine;
};
