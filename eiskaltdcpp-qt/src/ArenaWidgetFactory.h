/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/ 

#pragma once

#include "ArenaWidgetManager.h"
#include "dcpp/Singleton.h"

class ArenaWidgetFactory {
public:

    ArenaWidgetFactory(){
        
    }
    
    virtual ~ArenaWidgetFactory(){
        
    }
    
    template <class T, typename ... Params>
    T *create(const Params& ... args) {
        T *t = new T(args ...);
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template < template < class > class Type = dcpp::Singleton, class T > 
    auto create () -> decltype(Type<T>::getInstance()) {       
        if (!Type<T>::getInstance())
            Type<T>::newInstance();
        
        ArenaWidgetManager::getInstance()->add(Type<T>::getInstance());
        
        return Type<T>::getInstance();
    }
    
private:    
    ArenaWidgetFactory(const ArenaWidgetFactory &);
    ArenaWidgetFactory& operator=(const ArenaWidgetFactory&);
};
