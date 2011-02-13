Import("qt.core");
Import("qt.gui");

function shellDone(ok, msg){
  var hubMgr = new HubManager();
  var activeHub = hubMgr.getHubObject();

  msg = LinkParser.parseMagnetAlias(msg);

  if (ok && (activeHub != null))
    activeHub.sendMsg(msg);
}

function actionClicked(){
  var shell = new ShellCommandRunner(SCRIPTS_PATH+"clementine_nowplay/clementine.sh")
  shell["finished(bool,QString)"].connect(shellDone);

  shell.run();
}

var a = new MainWindowScript();
a.addToolButton("Clementine", "Clementine", new QIcon(SCRIPTS_PATH+"clementine_nowplay/clementine.png"));

MainWindow.ToolBar.Clementine.triggered.connect(actionClicked);

function deinit(){
  a.remToolButton("Clementine");
}

