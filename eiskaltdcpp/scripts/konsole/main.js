Import("qt.core");
Import("qt.gui");

function actionClicked(){
  shellExec("konsole");
}

var act = new QAction("Konsole", MainWindow);
act.icon = new QIcon(SCRIPTS_PATH+"konsole/konsole.png");
act.triggered.connect(actionClicked);

MainWindow.addActionOnToolBar(act);
