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
#include "QtContextAware.h"
#include "QtContext.h"

class ArenaWidgetFactory {
public:

    ArenaWidgetFactory() = default;
    virtual ~ArenaWidgetFactory() {}

    template <class T, typename ... Params>
    T *create(const Params& ... args) {
        T *t = new T(args ...);

        qtCtx()->arenaWidgetManager()->add(t);

        return t;
    }

private:
    ArenaWidgetFactory(const ArenaWidgetFactory &);
    ArenaWidgetFactory& operator=(const ArenaWidgetFactory&);
};
