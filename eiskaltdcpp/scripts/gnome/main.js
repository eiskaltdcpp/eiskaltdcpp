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
	var shell = new ShellCommandRunner(SCRIPTS_PATH+"gnome/commands.ru_RU.UTF-8.php")
  shell["finished(bool,QString)"].connect(shellDone);

  shell.run();
}

var a = new MainWindowScript();
a.addToolButton("GnomePlay", "GnomePlay", new QIcon(SCRIPTS_PATH+"gnome/gnome.png"));

MainWindow.ToolBar.GnomePlay.triggered.connect(actionClicked);

function deinit(){
  a.remToolButton("GnomePlay");
}

