/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ScriptConsole.h"
#include "ScriptEngine.h"

static QScriptValue myPrintFunc(QScriptContext *context, QScriptEngine *engine);
static QScriptValue myPrintErrFunc(QScriptContext *context, QScriptEngine *engine);

ScriptConsole::ScriptConsole(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    setWindowTitle(tr("Script Console"));

    ScriptEngine::getInstance()->prepareThis(engine);

    QScriptValue myPrint = engine.newFunction(myPrintFunc);
    myPrint.setData(engine.newQObject(textEdit_OUTPUT));
    engine.globalObject().setProperty("print", myPrint);
    
    QScriptValue myPrintErr = engine.newFunction(myPrintErrFunc);
    myPrint.setData(engine.newQObject(textEdit_OUTPUT));
    engine.globalObject().setProperty("printErr", myPrintErr);

    connect(pushButton_START, SIGNAL(clicked()), this, SLOT(startEvaluation()));
    connect(pushButton_STOP, SIGNAL(clicked()), this, SLOT(stopEvaluation()));
}

void ScriptConsole::startEvaluation(){
    engine.evaluate(textEdit_INPUT->toPlainText());
}

void ScriptConsole::stopEvaluation(){
    engine.abortEvaluation();

    if (engine.hasUncaughtException()){
        foreach (QString line, engine.uncaughtExceptionBacktrace())
            textEdit_OUTPUT->append(line);
    }
}

static QScriptValue myPrintFunc(QScriptContext *context, QScriptEngine *engine){
    QString result;

    for (int i = 0; i < context->argumentCount(); i++){
        if (i > 0)
            result.append(" ");

        result.append(context->argument(i).toString());
    }

    QScriptValue calleeData = context->callee().data();
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(calleeData.toQObject());

    if (textEdit)
        textEdit->append(result);

    return engine->undefinedValue();
}

static QScriptValue myPrintErrFunc(QScriptContext *context, QScriptEngine *engine){
    if (context->argumentCount() < 1)
        return engine->undefinedValue();
    
    QString result = context->argument(0).toString();
    
    for (int i = 1; i < context->argumentCount(); i++)
        result = result.arg(context->argument(i).toString());
   
    QScriptValue calleeData = context->callee().data();
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(calleeData.toQObject());

    if (textEdit)
        textEdit->append(result);

    return engine->undefinedValue();
}
