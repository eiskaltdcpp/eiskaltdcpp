#include "ArenaWidget.h"

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

void  ScriptWidget::setWidget(QWidget *wgt) { _wgt = wgt; }
void  ScriptWidget::setArenaTitle(QString t) { _arenaTitle = t; }
void  ScriptWidget::setArenaShortTitle(QString st) { _arenaShortTitle = st; }
void  ScriptWidget::setMenu(QMenu *m){ _menu = m; }
