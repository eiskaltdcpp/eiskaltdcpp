/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
