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

#include <assert.h>
#include <iostream>

ArenaWidgetManager::ArenaWidgetManager() : QObject(NULL) {
    DEBUG_BLOCK
}

ArenaWidgetManager::ArenaWidgetManager(const ArenaWidgetManager &m) {
    DEBUG_BLOCK
}

ArenaWidgetManager::~ArenaWidgetManager(){
    DEBUG_BLOCK
}

ArenaWidgetManager &ArenaWidgetManager::operator=(const ArenaWidgetManager &) { 
    DEBUG_BLOCK
    
    return *this; 
}

void ArenaWidgetManager::add ( ArenaWidget *awgt) {
    DEBUG_BLOCK
    
    if (!awgt){
        assert(0);
        
        return;
    }
    
    bool exists = widgets.contains(awgt);
    
    if (exists && (awgt->state() & ArenaWidget::Hidden)){
        awgt->setState(ArenaWidget::Flags(awgt->state() & ~ArenaWidget::Hidden));
        
        emit updated(awgt);
        
        return;
    }
    
    widgets.push_back(awgt);
    
    emit added(awgt);
}

void ArenaWidgetManager::rem ( ArenaWidget *awgt ) {
    DEBUG_BLOCK
    assert(0);
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
    
    if (awgt == dynamic_cast<ArenaWidget*>(awgt->getWidget())){ // ArenaWidget is a parent class of Widget
        awgt->getWidget()->setAttribute(Qt::WA_DeleteOnClose);
        
        awgt->getWidget()->close();
    }
    else
        assert(0);//We do not know what to do with this object...
}

void ArenaWidgetManager::activate ( ArenaWidget *awgt ) {
    if (!widgets.contains(awgt))
        emit activated(reinterpret_cast<ArenaWidget*>(NULL));
    
    emit activated(awgt);
}


