Import("qt.core");
Import("qt.gui");

function LogWidget(parent) {
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
  
  this.logManager = new LogManagerScript();
  this.logManager["message(QString, QString)"].connect(this, this.newMessage);
}

LogWidget.prototype = new QWidget();

LogWidget.prototype.newMessage = function(timeStamp, message) {
  message = LinkParser.parse(message);

  this.textEdit_OUTPUT.append("<b>[" + timeStamp +"]</b> " + message); 
}

function toggleScriptWidget(){
  var script_widget = new ScriptWidget();
    
  script_widget.title             = "LogManager View";
  script_widget.shortTitle        = script_widget.title;
  script_widget.widget            = new LogWidget(null);
  script_widget.pixmap	          = new QPixmap(SCRIPT_PATH+"log_file.png");

  WidgetManager.activate(script_widget);
}

var a = new MainWindowScript();
a.addToolButton("LogManager", "LogManager View", new QIcon(SCRIPT_PATH+"log_file.png"));

MainWindow.ToolBar.LogManager.triggered.connect(toggleScriptWidget);
MainWindow.ToolBar.LogManager.checkable = false;

function deinit(){
  a.remToolButton("LogManager");
}
