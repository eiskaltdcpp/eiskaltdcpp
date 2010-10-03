#include "ArenaWidget.h"

#include <QUrl>
#include <QFile>
#include <QVBoxLayout>

#include "WulforUtil.h"
#include "MainWindow.h"

ArenaWidget::ArenaWidget(): _arenaUnload(true), toolBtn(NULL)
{
}

ArenaWidget::~ArenaWidget(){
}

ScriptWidget::ScriptWidget(){
    _wgt = NULL;
    _menu = NULL;
}

ScriptWidget::~ScriptWidget(){
}

QWidget *ScriptWidget::getWidget(){ return _wgt; }
QString ScriptWidget::getArenaTitle() { return _arenaTitle; }
QString ScriptWidget::getArenaShortTitle() { return _arenaShortTitle; }
QMenu *ScriptWidget::getMenu() { return _menu; }
const QPixmap &ScriptWidget::getPixmap() { return pxm; }

void  ScriptWidget::setWidget(QWidget *wgt) { _wgt = wgt; }
void  ScriptWidget::setArenaTitle(QString t) { _arenaTitle = t; }
void  ScriptWidget::setArenaShortTitle(QString st) { _arenaShortTitle = st; }
void  ScriptWidget::setMenu(QMenu *_m) { _menu = _m; }
void  ScriptWidget::setPixmap(const QPixmap &px) { pxm = px; }

#ifdef USE_QML
DeclarativeWidget::DeclarativeWidget(const QString &file) : QWidget(NULL) {
    view = new QDeclarativeView();
    view->setSource(QUrl::fromLocalFile(file));

    setLayout(new QVBoxLayout());
    layout()->addWidget(view);
}

DeclarativeWidget::~DeclarativeWidget(){
}

void DeclarativeWidget::closeEvent(QCloseEvent *e){
    e->accept();

    setAttribute(Qt::WA_DeleteOnClose);

    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
    MainWindow::getInstance()->MainWindow::getInstance()->unmapArenaWidget(this);
}

QWidget *DeclarativeWidget::getWidget(){
    return this;
}

QString DeclarativeWidget::getArenaTitle(){
    QString fname = view->source().toLocalFile();

    return (fname.right(fname.length()-fname.lastIndexOf(QDir::separator())-1));
}

QString DeclarativeWidget::getArenaShortTitle(){
    return getArenaTitle();
}

QMenu *DeclarativeWidget::getMenu(){
    return NULL;
}

const QPixmap &DeclarativeWidget::getPixmap(){
    return WICON(WulforUtil::eiFILETYPE_APPLICATION);
}
#endif
