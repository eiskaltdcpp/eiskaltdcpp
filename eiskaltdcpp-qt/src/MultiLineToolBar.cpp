/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "MultiLineToolBar.h"
#include "QtContextAware.h"
#include "QtContext.h"
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

    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::added,     frame, &TabFrame::insertWidget);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::removed,   frame, &TabFrame::removeWidget);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::updated,   frame, &TabFrame::updated);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::activated, frame, &TabFrame::mapped);
    connect(qtCtx()->arenaWidgetManager(), &ArenaWidgetManager::toggled,   frame, &TabFrame::toggled);
    connect(this, &MultiLineToolBar::nextTab, frame, &TabFrame::nextTab);
    connect(this, &MultiLineToolBar::prevTab, frame, &TabFrame::prevTab);
    connect(this, &MultiLineToolBar::moveTabLeft, frame, &TabFrame::moveLeft);
    connect(this, &MultiLineToolBar::moveTabRight, frame, &TabFrame::moveRight);
    connect(this, &QWidget::customContextMenuRequested, this, &MultiLineToolBar::slotContextMenu);
}

MultiLineToolBar::~MultiLineToolBar(){
    frame->deleteLater();
}

void MultiLineToolBar::wheelEvent(QWheelEvent *e){
    e->ignore();

    if (e->angleDelta().y() > 0)
        emit nextTab();
    else if (e->angleDelta().y() < 0)
        emit prevTab();
}

void MultiLineToolBar::slotContextMenu(){
    QMenu *m = new QMenu(this);
    QAction *act = new QAction(tr("Show close buttons"), m);

    act->setCheckable(true);
    act->setChecked(qtCtx()->settings()->getBool(WB_APP_TBAR_SHOW_CL_BTNS));

    m->addAction(act);

    if (m->exec(QCursor::pos())){
        qtCtx()->settings()->setBool(WB_APP_TBAR_SHOW_CL_BTNS, act->isChecked());
    }

    m->deleteLater();
}
