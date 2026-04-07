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
#include <memory>

#include "QtContextAware.h"
class QTimer;

class GlobalTimer: public QObject,
        public QtContextAware
{
    Q_OBJECT
public:
    explicit GlobalTimer(dcpp::DCContext& ctx);
    ~GlobalTimer() override;


    quint64 getTicks() const;

Q_SIGNALS:
    void second();
    void minute();

private Q_SLOTS:
    void slotTick();

private:
    friend class QtContext;
    GlobalTimer(const GlobalTimer &) = delete;
    GlobalTimer &operator=(const GlobalTimer&) = delete;

    std::unique_ptr<QTimer> timer;
    quint64 tickCount;
};
