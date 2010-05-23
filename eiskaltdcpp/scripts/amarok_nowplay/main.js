Import("qt.core");
Import("qt.gui");

function shellDone(ok, msg){
  var hubMgr = new HubManager();
  var activeHub = hubMgr.getHubObject();

  if (ok && (activeHub != null))
    activeHub.sendMsg(msg);
}

function actionClicked(){
  var shell = new ShellCommandRunner(SCRIPTS_PATH+"amarok_nowplay/amarok.sh")
  shell["finished(bool,QString)"].connect(shellDone);

  shell.run();
}

var act = new QAction("Amarok", MainWindow);
act.icon = new QIcon(SCRIPTS_PATH+"amarok_nowplay/amarok.png");
act.triggered.connect(actionClicked);

MainWindow.addActionOnToolBar(act);
