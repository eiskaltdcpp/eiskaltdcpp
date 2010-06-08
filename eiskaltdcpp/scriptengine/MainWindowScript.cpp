#include "MainWindowScript.h"
#include "MainWindow.h"

#include <QtDebug>

MainWindowScript::MainWindowScript(QScriptEngine *engine, QObject *parent) :
    engine(engine),
    QObject(parent)
{
    Q_ASSERT_X(engine != NULL, Q_FUNC_INFO, "engine == NULL");
}

MainWindowScript::~MainWindowScript(){

}

bool MainWindowScript::addToolButton(const QString &name, const QString &title, const QIcon &icon){
    QScriptValue mwToolBar = engine->globalObject().property("MainWindow").property("ToolBar");

    if(mwToolBar.isUndefined()){
        qDebug() << engine->currentContext()->backtrace();

        engine->abortEvaluation();

        return false;
    }

    if (name.isEmpty())
        return false;

    QAction *act = new QAction(icon, title, MainWindow::getInstance());
    act->setObjectName("scriptToolbarButton"+name);
    actions.insert(name, act);

    QScriptValue act_val = engine->newQObject(act);
    mwToolBar.setProperty(name, act_val);

    MainWindow::getInstance()->addActionOnToolBar(act);

    return true;
}

bool MainWindowScript::remToolButton(const QString &name){
    QScriptValue mwToolBar = engine->globalObject().property("MainWindow").property("ToolBar");

    if(mwToolBar.isUndefined()){
        qDebug() << engine->currentContext()->backtrace();

        engine->abortEvaluation();

        return false;
    }

    if (mwToolBar.property(name).isUndefined() || !actions.contains(name))
        return false;

    QAction *act = actions.value(name);

    QScriptValue act_val();
    mwToolBar.setProperty(name, act_val);

    MainWindow::getInstance()->remActionFromToolBar(act);

    return true;
}
