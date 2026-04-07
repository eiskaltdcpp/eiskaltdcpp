/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QObject>
#include <QList>

class ArenaWidget;
class ArenaWidgetFactory;
#include "QtContextAware.h"

class ArenaWidgetManager: public QObject,
        public QtContextAware
{
Q_OBJECT

friend class QtContext;
friend class ArenaWidgetFactory;

public:
    explicit ArenaWidgetManager(dcpp::DCContext& ctx);
    ~ArenaWidgetManager() override;


public Q_SLOTS:
    void rem(ArenaWidget*);
    void activate(ArenaWidget*);
    void toggle(ArenaWidget*);

Q_SIGNALS:
    void added(ArenaWidget*);
    void removed(ArenaWidget*);
    void updated(ArenaWidget*);
    void activated(ArenaWidget*);
    void toggled(ArenaWidget*);

public:
    void add(ArenaWidget*);

private:
    ArenaWidgetManager(const ArenaWidgetManager &m) = delete;
    ArenaWidgetManager &operator=(const ArenaWidgetManager &) = delete;

    QList<ArenaWidget*> widgets;
};
