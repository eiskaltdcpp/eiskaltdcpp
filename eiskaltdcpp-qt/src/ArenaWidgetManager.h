/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef ARENAWIDGETMANAGER_H
#define ARENAWIDGETMANAGER_H

#include <QObject>
#include <QList>

#include "dcpp/Singleton.h"

class ArenaWidget;

class ArenaWidgetManager: public QObject, public dcpp::Singleton<ArenaWidgetManager>
{
Q_OBJECT

friend class dcpp::Singleton<ArenaWidgetManager>;

public Q_SLOTS:
    void add(ArenaWidget*);
    void rem(ArenaWidget*);
    void activate(ArenaWidget*);
    void toggle(ArenaWidget*);
    
Q_SIGNALS:
    void added(ArenaWidget*);
    void removed(ArenaWidget*);
    void updated(ArenaWidget*);
    void activated(ArenaWidget*);
    void toggled(ArenaWidget*);
    
private:
    ArenaWidgetManager();
    ArenaWidgetManager(const ArenaWidgetManager &m);
    virtual ~ArenaWidgetManager();
    ArenaWidgetManager &operator=(const ArenaWidgetManager &);
    
    QList<ArenaWidget*> widgets;
};

#endif