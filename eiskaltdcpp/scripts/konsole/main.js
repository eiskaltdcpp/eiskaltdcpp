ScriptEngine.importExtension("qt.core");
ScriptEngine.importExtension("qt.gui");

function actionClicked(){
  shellExec("konsole");
}

var act = new QAction("TestScript", MainWindow);
act.icon = new QIcon(SCRIPTS_PATH+"konsole/konsole.png");
act.triggered.connect(actionClicked);

MainWindow.addActionOnToolBar(act);
