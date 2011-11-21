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
    
    foreach ( ArenaWidget *awgt , widgets ) {
        if ( awgt == dynamic_cast<ArenaWidget*> ( awgt->getWidget() ) ) { // ArenaWidget is a parent class of Widget
            awgt->getWidget()->setAttribute ( Qt::WA_DeleteOnClose );
            awgt->setUnload(true);
            awgt->getWidget()->close();
        }
        else 
            assert(0);
    }
}

void ArenaWidgetManager::add ( ArenaWidget *awgt) {
    DEBUG_BLOCK
    
    if (!awgt || widgets.contains(awgt)){
        assert(0);
        
        return;
    }
        
    widgets.push_back(awgt);
    
    emit added(awgt);
    
    if ((awgt->state() & ArenaWidget::RaiseOnStart) && !(awgt->state() & ArenaWidget::Hidden))
        activate(awgt);
}

void ArenaWidgetManager::rem ( ArenaWidget *awgt ) {
    DEBUG_BLOCK

    if (!(awgt && widgets.contains(awgt))){
        assert(0);
        
        return;
    }
    
    if (awgt->state() & ArenaWidget::Singleton) {
        awgt->setState(ArenaWidget::Flags(awgt->state() | ArenaWidget::Hidden));
        
        emit updated(awgt);
        
        return;
    }
    
    widgets.removeAt(widgets.indexOf(awgt));
    
    emit removed(awgt);
    
    QApplication::processEvents();
    
    if (awgt == dynamic_cast<ArenaWidget*>(awgt->getWidget())){ // ArenaWidget is a parent class of Widget
        awgt->getWidget()->setAttribute(Qt::WA_DeleteOnClose, false);
        awgt->getWidget()->close();
        awgt->getWidget()->deleteLater();
    }
    else
        assert(0);//We do not know what to do with this object...
}

void ArenaWidgetManager::activate ( ArenaWidget *awgt ) {
    DEBUG_BLOCK
    
    if (!widgets.contains(awgt)){
        emit activated(reinterpret_cast<ArenaWidget*>(NULL));
        
        return;
    }
    
    if (awgt->state() & ArenaWidget::Hidden){
        awgt->setState(ArenaWidget::Flags(awgt->state() & (~ArenaWidget::Hidden)));
        
        emit updated(awgt);
    }
    
    emit activated(awgt);
}

void ArenaWidgetManager::toggle ( ArenaWidget *awgt) {
    DEBUG_BLOCK
    
    if (!(awgt->state() & ArenaWidget::Singleton))
        return;
    
    if (awgt->state() & ArenaWidget::Hidden)
        activate(awgt);
    else
        rem(awgt);
}
