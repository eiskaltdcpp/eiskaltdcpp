Import("qt.core");
Import("qt.gui");

function actionClicked(){
  shellExec("konsole");
}

var a = new MainWindowScript();
a.addToolButton("Konsole", "Konsole", new QIcon(SCRIPTS_PATH+"konsole/konsole.png"));

MainWindow.ToolBar.Konsole.triggered.connect(actionClicked);

function deinit(){
  a.remToolButton("Konsole");
}
