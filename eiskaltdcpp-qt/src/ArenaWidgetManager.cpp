/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ArenaWidgetManager.h"

#include "ArenaWidget.h"
#include "DebugHelper.h"
#include "WulforUtil.h"

#include <assert.h>

#include <QApplication>

ArenaWidgetManager::ArenaWidgetManager() : QObject(NULL) {
    DEBUG_BLOCK
}

ArenaWidgetManager::~ArenaWidgetManager(){
    DEBUG_BLOCK

    disconnect(this, 0, 0, 0);
    
    foreach ( ArenaWidget *awgt , widgets ) {
        if (!dynamic_cast<QObject*>(awgt))
            continue;
        if (dcpp::ISingleton *isingleton = dynamic_cast<dcpp::ISingleton*>(awgt)){
            isingleton->release();
        }
        else if ( awgt == dynamic_cast<ArenaWidget*> ( awgt->getWidget() ) ) { // ArenaWidget is a parent class of Widget
            awgt->getWidget()->setAttribute ( Qt::WA_DeleteOnClose );
            awgt->setUnload(true);
            awgt->getWidget()->close();
        }
    }
}

void ArenaWidgetManager::add ( ArenaWidget *awgt) {
    DEBUG_BLOCK
    
    if (!awgt)
        return;
    
    if (!widgets.isEmpty()){
        if (widgets.contains(awgt))
            return;
    }
        
    widgets.append(awgt);
    
    emit added(awgt);
    
    if ((awgt->state() & ArenaWidget::RaiseOnStart) && !(awgt->state() & ArenaWidget::Hidden))
        activate(awgt);
}

void ArenaWidgetManager::rem ( ArenaWidget *awgt ) {
    DEBUG_BLOCK
    
    if (!awgt || widgets.isEmpty())
        return;
    
    if (!widgets.contains(awgt))
        return;
    
    if (awgt->state() & ArenaWidget::Singleton) {
        awgt->setState(ArenaWidget::Flags(awgt->state() | ArenaWidget::Hidden));
        
        emit updated(awgt);
        
        return;
    }
    
    widgets.removeAt(widgets.indexOf(awgt));
    
    emit removed(awgt);
    
    QApplication::processEvents();
    
    if (!dynamic_cast<QObject*>(awgt))
        return;
    
    if (awgt == dynamic_cast<ArenaWidget*>(awgt->getWidget())){ // ArenaWidget is a parent class of Widget
        awgt->getWidget()->setAttribute(Qt::WA_DeleteOnClose, false);
        awgt->getWidget()->close();
        awgt->getWidget()->deleteLater();
    }
    else {
        awgt->getWidget()->setAttribute(Qt::WA_DeleteOnClose, true);
        awgt->getWidget()->close();
        
        if (ScriptWidget *wgt = dynamic_cast<ScriptWidget*>(awgt)){
            delete wgt;
        }
    }
}

void ArenaWidgetManager::activate ( ArenaWidget *awgt ) {
    DEBUG_BLOCK
    
    if (!awgt)
        return;
    
    if (!widgets.isEmpty()){
        if (!widgets.contains(awgt)){
            emit activated(NULL);
            return;
        }
    }
    
    if (awgt->state() & ArenaWidget::Hidden){
        awgt->setState(ArenaWidget::Flags(awgt->state() & (~ArenaWidget::Hidden)));
        
        emit updated(awgt);
    }
    
    emit activated(awgt);
}

void ArenaWidgetManager::toggle ( ArenaWidget *awgt) {
    DEBUG_BLOCK
    if (!awgt)
        return;
    
    emit toggled(awgt);
}
