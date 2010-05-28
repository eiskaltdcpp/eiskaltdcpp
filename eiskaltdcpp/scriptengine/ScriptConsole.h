#ifndef SCRIPTCONSOLE_H
#define SCRIPTCONSOLE_H

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

#endif // SCRIPTCONSOLE_H
