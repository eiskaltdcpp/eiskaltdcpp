/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/ 

#ifndef ARENAWIDGETFACTORY_H
#define ARENAWIDGETFACTORY_H

#include "ArenaWidgetManager.h"

#include "dcpp/Singleton.h"

class NullType {};

class ArenaWidgetFactory {
public:

    ArenaWidgetFactory(){
        
    }
    
    virtual ~ArenaWidgetFactory(){
        
    }
    
    template <class T, typename P1 = NullType, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType>
    T *create() {
        T *t = new T();
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template <class T, typename P1, typename P2 = NullType, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType>
    T *create(const P1 &p1) {
        T *t = new T(p1);
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template <class T, typename P1, typename P2, typename P3 = NullType, typename P4 = NullType, typename P5 = NullType>
    T *create(const P1 &p1, const P2 &p2) {
        T *t = new T(p1, p2);
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template <class T, typename P1, typename P2, typename P3, typename P4 = NullType, typename P5 = NullType>
    T *create(const P1 &p1, const P2 &p2, const P3 &p3) {
        T *t = new T(p1, p2, p3);
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template <class T, typename P1, typename P2, typename P3, typename P4, typename P5 = NullType>
    T *create(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) {
        T *t = new T(p1, p2, p3, p4);
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template <class T, typename P1, typename P2, typename P3, typename P4, typename P5>
    T *create(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) {
        T *t = new T(p1, p2, p3, p4, p5);
        
        ArenaWidgetManager::getInstance()->add(t);
        
        return t;
    }
    
    template < template < class > class Type = dcpp::Singleton, class T > 
    T *create () {       
        if (!T::getInstance())
            T::newInstance();
        
        ArenaWidgetManager::getInstance()->add(T::getInstance());
        
        return T::getInstance();
    }
    
private:    
    ArenaWidgetFactory(const ArenaWidgetFactory &);
    ArenaWidgetFactory& operator=(const ArenaWidgetFactory&);
};

#endif // ARENAWIDGETFACTORY_H
