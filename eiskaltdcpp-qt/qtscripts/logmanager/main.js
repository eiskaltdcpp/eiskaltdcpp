Import("qt.core");
Import("qt.gui");

var script_widget = null;

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

function hideMe() {
  MainWindow.remArenaWidgetFromToolbar(script_widget);
  MainWindow.remWidgetFromArena(script_widget);
  MainWindow.remArenaWidget(script_widget);

  script_widget.shown = false;

  MainWindow.ToolBar.LogManager.checked = false;
}

function showMe() {
  MainWindow.addArenaWidget(script_widget);
  MainWindow.addArenaWidgetOnToolbar(script_widget);
  MainWindow.mapWidgetOnArena(script_widget);

  MainWindow.ToolBar.LogManager.checked = true;
  
  script_widget.shown = true;
}

LogWidget.prototype.closeEvent = function(e) {
  hideMe();

  e.accept();
}

function toggleScriptWidget(){
  if (script_widget == null){
    script_widget = new ScriptWidget();
    script_widget.title             = "LogManager View";
    script_widget.shortTitle        = script_widget.title;
    script_widget.widget            = new LogWidget(null);
    script_widget.pixmap	    = new QPixmap(SCRIPT_PATH+"log_file.png");

    script_widget.shown    	    = false;
  }

  if (script_widget.shown)
    hideMe();
  else
    showMe();
}

var a = new MainWindowScript();
a.addToolButton("LogManager", "LogManager View", new QIcon(SCRIPT_PATH+"log_file.png"));

MainWindow.ToolBar.LogManager.triggered.connect(toggleScriptWidget);
MainWindow.ToolBar.LogManager.checkable = true;

function deinit(){
  a.remToolButton("LogManager");
}
