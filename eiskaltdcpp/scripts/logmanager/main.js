Import("qt");
Import("qt.core");
Import("qt.gui");

function LogDialog(parent) {
  QWidget.call(this, parent);
  
  this.mainLayout = new QVBoxLayout(this);
  this.setLayout(this.mainLayout);
  
  this.textEdit_OUTPUT = new QTextEdit(this);
  this.textEdit_OUTPUT.readOnly = true;
  this.textEdit_OUTPUT.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding);
  this.textEdit_OUTPUT.alignment = Qt.Alignment(Qt.AlignCenter);
  this.textEdit_OUTPUT.append("<b> == Logs == </b><br/><br/>");
  
  this.mainLayout.addWidget(this.textEdit_OUTPUT, 0, 0);
  
  this.windowTitle = "Log Viewer";
  this.resize(450, 400);
  
  this.logManager = new LogManagerScript();
  this.logManager["message(QString, QString)"].connect(this, this.newMessage);
}

LogDialog.prototype = new QWidget();

LogDialog.prototype.newMessage = function(timeStamp, message) {
  message = LinkParser.parse(message);

  print(message);  

  this.textEdit_OUTPUT.append("<b>[" + timeStamp +"]</b> " + message); 
}

var dlg = new LogDialog(null);

function actionClicked(){
  dlg.show();
}

var a = new MainWindowScript();
a.addToolButton("LogViewer", "LogViewer", new 
QIcon(SCRIPTS_PATH+"logmanager/log_file.png"));//or SCRIPT_PATH+"log_file.png"

MainWindow.ToolBar.LogViewer.triggered.connect(actionClicked);

function deinit(){
  a.remToolButton("LogViewer");
  dlg.close();
}
