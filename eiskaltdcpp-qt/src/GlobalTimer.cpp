/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/ 

#include "GlobalTimer.h"

#include <QTimer>

GlobalTimer::GlobalTimer() : QObject(nullptr), timer(new QTimer()), tickCount(0) {
    timer->setInterval(1000);
    timer->setSingleShot(false);
    
    connect(timer.get(), SIGNAL(timeout()), this, SLOT(slotTick()));
    
    timer->start();
}

GlobalTimer::~GlobalTimer() {

}

void GlobalTimer::slotTick() {
    tickCount++;
    
    emit second();
    
    if (tickCount % 60 == 0)
        emit minute();
}

quint64 GlobalTimer::getTicks() const {
    return tickCount;
}
