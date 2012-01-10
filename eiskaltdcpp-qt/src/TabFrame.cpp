/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "TabFrame.h"

#include "FlowLayout.h"
#include "TabButton.h"
#include "WulforUtil.h"
#include "ArenaWidgetManager.h"
#include "DebugHelper.h"
#include "GlobalTimer.h"

#include <QtGui>
#include <QPushButton>
#include <QWheelEvent>

#include <boost/function.hpp>
#include <boost/bind.hpp>

TabFrame::TabFrame(QWidget *parent) :
    QFrame(parent)
{
    DEBUG_BLOCK
    
    setAcceptDrops(true);

    fr_layout = new FlowLayout(this);
    fr_layout->setContentsMargins(0, 0, 0, 0);

    setMinimumHeight(20);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    shortcuts << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_1), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_2), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_3), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_4), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_5), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_6), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_7), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_8), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_9), this))
              << (new QShortcut(QKeySequence(Qt::ALT + Qt::Key_0), this));

    foreach (QShortcut *s, shortcuts){
        s->setContext(Qt::ApplicationShortcut);

        connect(s, SIGNAL(activated()), this, SLOT(slotShorcuts()));
    }
    
    connect(GlobalTimer::getInstance(), SIGNAL(second()), this, SLOT(redraw()));
}


TabFrame::~TabFrame(){
    DEBUG_BLOCK
    
    QMap<TabButton*, ArenaWidget*>::iterator it = tbtn_map.begin();

    for  (; it != tbtn_map.end(); ++it){
        TabButton *btn = const_cast<TabButton*>(it.key());

        btn->deleteLater();
    }
}

void TabFrame::resizeEvent(QResizeEvent *e){
    e->accept();

    QFrame::updateGeometry();
}

bool TabFrame::eventFilter(QObject *obj, QEvent *e){
    TabButton *btn = qobject_cast<TabButton*>(obj);
    QWheelEvent *w_e = reinterpret_cast<QWheelEvent*>(e);

    if (btn && (e->type() == QEvent::Wheel) && w_e){
        int numDegrees = (w_e->delta() < 0)? (-1*w_e->delta()/8) : (w_e->delta()/8);
        int numSteps = numDegrees/15;
        boost::function<void()> f = (w_e->delta() < 0)? boost::bind(&TabFrame::nextTab, this) : boost::bind(&TabFrame::prevTab, this);

        for (int i = 0; i < numSteps; i++)
            f();

        return true;
    }

    return QFrame::eventFilter(obj, e);
}

QSize TabFrame::sizeHint() const {
    QSize s(fr_layout->sizeHint().width() , fr_layout->heightForWidth(width()));
    return s;
}

QSize TabFrame::minimumSizeHint() const{
    return sizeHint();
}

void TabFrame::removeWidget(ArenaWidget *awgt){
    DEBUG_BLOCK
    
    if (!awgt_map.contains(awgt))
        return;

    TabButton *btn = const_cast<TabButton*>(awgt_map.value(awgt));

    fr_layout->removeWidget(btn);
    tbtn_map.remove(btn);
    awgt_map.remove(awgt);

    btn->deleteLater();

    historyPurge(awgt);
    historyPop();
    
     if (awgt->toolButton())
        awgt->toolButton()->setChecked(false);
}

void TabFrame::insertWidget(ArenaWidget *awgt){
    DEBUG_BLOCK
    
    if (awgt_map.contains(awgt) || (awgt && (awgt->state() & ArenaWidget::Hidden)) || !awgt)
        return;

    TabButton *btn = new TabButton();
    btn->setText(awgt->getArenaShortTitle().left(32));
    btn->setToolTip(WulforUtil::getInstance()->compactToolTipText(awgt->getArenaTitle(), 60, "\n"));
    btn->setWidgetIcon(awgt->getPixmap());
    btn->setContextMenuPolicy(Qt::CustomContextMenu);
    btn->installEventFilter(this);

    fr_layout->addWidget(btn);

    awgt_map.insert(awgt, btn);
    tbtn_map.insert(btn, awgt);
    
    if (awgt->toolButton())
        awgt->toolButton()->setChecked(true);

    connect(btn, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
    connect(btn, SIGNAL(clicked()), this, SLOT(buttonClicked()));
    connect(btn, SIGNAL(closeRequest()), this, SLOT(closeRequsted()));
    connect(btn, SIGNAL(dropped(TabButton*)), this, SLOT(slotDropped(TabButton*)));
}

bool TabFrame::hasWidget(ArenaWidget *awgt) const{
    DEBUG_BLOCK
    
    return awgt_map.contains(awgt);
}

void TabFrame::mapped(ArenaWidget *awgt){
    DEBUG_BLOCK
    
    if (!awgt_map.contains(awgt))
        return;

    TabButton *btn = const_cast<TabButton*>(awgt_map.value(awgt));

    btn->setChecked(true);
    btn->setFocus();

    historyPush(awgt);
}

void TabFrame::updated ( ArenaWidget* awgt ) {
    DEBUG_BLOCK
    
    if (awgt->state() & ArenaWidget::Hidden){
        removeWidget(awgt);
    }
    else if (!awgt_map.contains(awgt)){
        insertWidget(awgt);
    }
}

void TabFrame::redraw() {
    DEBUG_BLOCK
    
    QMap<TabButton*, ArenaWidget*>::iterator it = tbtn_map.begin();
    int maxWidth = 0;

    for  (; it != tbtn_map.end(); ++it){
        TabButton *btn = const_cast<TabButton*>(it.key());
        ArenaWidget *awgt = const_cast<ArenaWidget*>(it.value());

        btn->setText(awgt->getArenaShortTitle().left(32));
        btn->setToolTip(WulforUtil::getInstance()->compactToolTipText(awgt->getArenaTitle(), 60, "\n"));
        btn->setWidgetIcon(awgt->getPixmap());

        maxWidth = qMax(maxWidth, btn->normalWidth());//recalculate maximal width
    }

    TabButton::setMaxWidth(maxWidth);

    for  (it = tbtn_map.begin(); it != tbtn_map.end(); ++it){
        TabButton *btn = const_cast<TabButton*>(it.key());
        ArenaWidget *awgt = const_cast<ArenaWidget*>(it.value());
        
        if (awgt->state() & ArenaWidget::Hidden)
            continue;
        else
            btn->resetGeometry();
    }
}

void TabFrame::historyPush(ArenaWidget *awgt){
    DEBUG_BLOCK
    
    historyPurge(awgt);

    history.push_back(awgt);
}

void TabFrame::historyPurge(ArenaWidget *awgt){
    DEBUG_BLOCK
    
    if (history.contains(awgt))
        history.removeAt(history.indexOf(awgt));
}

void TabFrame::historyPop(){
    DEBUG_BLOCK
    
    if (history.isEmpty() && fr_layout->count() > 0){
        QLayoutItem *item = fr_layout->itemAt(0);

        if (!item)
            return;

        TabButton *btn = qobject_cast<TabButton*>(item->widget());

        if (btn)
            ArenaWidgetManager::getInstance()->activate(tbtn_map[btn]);

        return;
    }
    else if (history.isEmpty()){
        ArenaWidgetManager::getInstance()->activate(NULL);
        
        return;
    }

    ArenaWidget *awgt = history.takeLast();

    ArenaWidgetManager::getInstance()->activate(awgt);
}

void TabFrame::buttonClicked(){
    DEBUG_BLOCK
    
    TabButton *btn = qobject_cast<TabButton*>(sender());

    if (!(btn && tbtn_map.contains(btn)))
        return;

    btn->setFocus();

    ArenaWidgetManager::getInstance()->activate(tbtn_map[btn]);
}

void TabFrame::closeRequsted() {
    DEBUG_BLOCK
    
    TabButton *btn = qobject_cast<TabButton*>(sender());

    if (!(btn && tbtn_map.contains(btn)))
        return;

    ArenaWidget *awgt = const_cast<ArenaWidget*>(tbtn_map[btn]);
    ArenaWidgetManager::getInstance()->rem(awgt);
}

void TabFrame::nextTab(){
    DEBUG_BLOCK
    
    TabButton *next = NULL;

    for (int i = 0; i < fr_layout->count(); i++){
        TabButton *t = qobject_cast<TabButton*>(fr_layout->itemAt(i)->widget());

        if (t && t->isChecked()){
            if (i == (fr_layout->count()-1)){
                next = qobject_cast<TabButton*>(fr_layout->itemAt(0)->widget());
                break;
            }

            next = qobject_cast<TabButton*>(fr_layout->itemAt(i+1)->widget());
            break;
        }
    }

    if (!next)
        return;

    ArenaWidgetManager::getInstance()->activate(tbtn_map[next]);
}

void TabFrame::prevTab(){
    DEBUG_BLOCK
    
    TabButton *next = NULL;

    for (int i = 0; i < fr_layout->count(); i++){
        TabButton *t = qobject_cast<TabButton*>(fr_layout->itemAt(i)->widget());

        if (t && t->isChecked()){
            if (i == 0){
                next = qobject_cast<TabButton*>(fr_layout->itemAt(fr_layout->count()-1)->widget());
                break;
            }

            next = qobject_cast<TabButton*>(fr_layout->itemAt(i-1)->widget());
            break;
        }
    }

    if (!next)
        return;

   ArenaWidgetManager::getInstance()->activate(tbtn_map[next]);
}

void TabFrame::slotShorcuts(){
    DEBUG_BLOCK
    
    QShortcut *sh = qobject_cast<QShortcut*>(sender());

    if (!sh)
        return;

    int index = shortcuts.indexOf(sh);

    if (index >= 0 && fr_layout->count() >= (index + 1)){
        TabButton *next = qobject_cast<TabButton*>(fr_layout->itemAt(index)->widget());

        if (!next)
            return;

        ArenaWidgetManager::getInstance()->activate(tbtn_map[next]);
    }
}

void TabFrame::slotContextMenu() {
    DEBUG_BLOCK
    
    TabButton *btn = qobject_cast<TabButton*>(sender());

    if (!(btn && tbtn_map.contains(btn)))
        return;

    ArenaWidget *awgt = const_cast<ArenaWidget*>(tbtn_map[btn]);

    if (awgt && awgt->getMenu())
        awgt->getMenu()->exec(btn->mapToGlobal(btn->rect().bottomLeft()));
    else if (awgt){
        QMenu *m = new QMenu(this);
        m->addAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE), tr("Close"));

        if (m->exec(QCursor::pos()))
            ArenaWidgetManager::getInstance()->rem(awgt);
    }
}

void TabFrame::slotDropped(TabButton *dropped){
    DEBUG_BLOCK
    
    TabButton *on = qobject_cast<TabButton*>(sender());

    if (!(on && dropped && on != dropped))
        return;

    fr_layout->place(on, dropped);
}

void TabFrame::moveLeft(){
    DEBUG_BLOCK
    
    for (int i = 0; i < fr_layout->count(); i++){
        QLayoutItem *item = const_cast<QLayoutItem*>(fr_layout->itemAt(i));
        TabButton *t = qobject_cast<TabButton*>(item->widget());

        if (t && t->isChecked()){
            fr_layout->moveLeft(item);

            break;
        }
    }
}

void TabFrame::moveRight(){
    DEBUG_BLOCK
    
    for (int i = 0; i < fr_layout->count(); i++){
        QLayoutItem *item = const_cast<QLayoutItem*>(fr_layout->itemAt(i));
        TabButton *t = qobject_cast<TabButton*>(item->widget());

        if (t && t->isChecked()){
            fr_layout->moveRight(item);

            break;
        }
    }
}

void TabFrame::toggled ( ArenaWidget* awgt ) {
    DEBUG_BLOCK
    
    if (!awgt)
        return;
    
    if (!(awgt->state() & ArenaWidget::Singleton))
        return;
    
    if (awgt->state() & ArenaWidget::Hidden)
        ArenaWidgetManager::getInstance()->activate(awgt);
    else
        ArenaWidgetManager::getInstance()->rem(awgt);
}

