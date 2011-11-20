/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "MultiLineToolBar.h"
#include "WulforSettings.h"
#include "ArenaWidgetManager.h"

#include <QMenu>
#include <QWheelEvent>

MultiLineToolBar::MultiLineToolBar(QWidget *parent) :
    QToolBar(parent)
{
    setObjectName("multiLineTabbar");

    frame = new TabFrame();

    addWidget(frame);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ArenaWidgetManager::getInstance(), SIGNAL(added(ArenaWidget*)),     frame, SLOT(insertWidget(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(removed(ArenaWidget*)),   frame, SLOT(removeWidget(ArenaWidget*)));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(updated(ArenaWidget*)),   frame, SLOT(redraw()));
    connect(ArenaWidgetManager::getInstance(), SIGNAL(activated(ArenaWidget*)), frame, SLOT(mapped(ArenaWidget*)));
    connect(this, SIGNAL(nextTab()), frame, SLOT(nextTab()));
    connect(this, SIGNAL(prevTab()), frame, SLOT(prevTab()));
    connect(this, SIGNAL(moveTabLeft()), frame, SLOT(moveLeft()));
    connect(this, SIGNAL(moveTabRight()), frame, SLOT(moveRight()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
}

MultiLineToolBar::~MultiLineToolBar(){
    frame->deleteLater();
}

void MultiLineToolBar::wheelEvent(QWheelEvent *e){
    e->ignore();

    if (e->delta() > 0)
        emit nextTab();
    else if (e->delta() < 0)
        emit prevTab();
}

void MultiLineToolBar::slotContextMenu(){
    QMenu *m = new QMenu(this);
    QAction *act = new QAction(tr("Show close buttons"), m);

    act->setCheckable(true);
    act->setChecked(WBGET(WB_APP_TBAR_SHOW_CL_BTNS));

    m->addAction(act);

    if (m->exec(QCursor::pos()) != NULL){
        WBSET(WB_APP_TBAR_SHOW_CL_BTNS, act->isChecked());
    }

    m->deleteLater();
}
