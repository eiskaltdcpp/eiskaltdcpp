/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ArenaWidget.h"

#include <QUrl>
#include <QFile>
#include <QVBoxLayout>

#include "WulforUtil.h"
#include "MainWindow.h"
#include "ArenaWidgetManager.h"

ArenaWidget::Flags operator|(const ArenaWidget::Flags &a, const ArenaWidget::Flags &b){
    return ArenaWidget::Flags(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

ArenaWidget::Flags operator&(const ArenaWidget::Flags &a, const ArenaWidget::Flags &b){
    return ArenaWidget::Flags(static_cast<unsigned>(a) & static_cast<unsigned>(b));
}

ArenaWidget::Flags operator^(const ArenaWidget::Flags &a, const ArenaWidget::Flags &b){
    return ArenaWidget::Flags(static_cast<unsigned>(a) ^ static_cast<unsigned>(b));
}

ArenaWidget::ArenaWidget(): _arenaUnload(true), toolBtn(NULL), flags(ArenaWidget::NoFlags | ArenaWidget::RaiseOnStart)
{
}

ArenaWidget::~ArenaWidget(){
}

void ArenaWidget::registerThis() {
    ArenaWidgetManager::getInstance()->add(this);
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
    MainWindow::getInstance()->remWidgetFromArena(this);
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
