#ifndef ARENAWIDGETCONTAINER_H
#define ARENAWIDGETCONTAINER_H

#include "ArenaWidget.h"

class ArenaWidgetContainer
{
public:
    virtual void removeWidget(ArenaWidget *awgt) = 0;
    virtual void insertWidget(ArenaWidget *awgt) = 0;
    virtual bool hasWidget(ArenaWidget *awgt) const = 0;
    virtual void mapped(ArenaWidget *awgt) = 0;
};

#endif // ARENAWIDGETCONTAINER_H
