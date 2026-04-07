/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#pragma once

#include <QApplication>
#include <QEvent>
#include <QTimer>
#include <QSessionManager>
#include "WulforSettings.h"
#include "QtContext.h"
#include "QtContextAware.h"
#include "MainWindow.h"
#include "qtsingleapp/qtsinglecoreapplication.h"

class EiskaltEventFilter: public QObject{
Q_OBJECT
public:
    EiskaltEventFilter(): counter(0), has_activity(true) {
        timer.setInterval(60000);

        connect(&timer, &QTimer::timeout, this, &EiskaltEventFilter::tick);

        timer.start();
    }

    virtual ~EiskaltEventFilter() {}

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event){
        switch (event->type()){
            case QEvent::MouseButtonPress:
            case QEvent::MouseButtonRelease:
            case QEvent::MouseButtonDblClick:
            case QEvent::MouseMove:
            case QEvent::KeyPress:
            case QEvent::KeyRelease:
            case QEvent::Wheel:
            {
                has_activity = true;
                counter = 0;

                break;
            }
            default:
            {
                has_activity = false;

                break;
            }
        }

        return QObject::eventFilter(obj, event);
    }

private Q_SLOTS:
    void tick(){
        if (!has_activity)
            ++counter;

        if (qtCtx()->settings()->getBool(WB_APP_AUTOAWAY_BY_TIMER)){
            int mins = qtCtx()->settings()->getInt(WI_APP_AUTOAWAY_INTERVAL);

            if (!mins)
                return;

            int mins_done = (counter*timer.interval()/1000)/60;

            if (mins <= mins_done)
                dcpp::Util::setAway(qtCtx()->dcCtx(), true);
        }
        else if (has_activity && !dcpp::Util::getManualAway())
            dcpp::Util::setAway(qtCtx()->dcCtx(), false);
    }

private:
    QTimer timer;
    int counter;
    bool has_activity;
};

class EiskaltApp: public QtSingleCoreApplication {
Q_OBJECT
public:
    EiskaltApp(int &argc, char *argv[], const QString& uniqKey): QtSingleCoreApplication(argc, argv, uniqKey)
    {
        installEventFilter(&ef);
    }

    void commitData(QSessionManager& manager){
        if (qtCtx()->mainWindow()){
            qtCtx()->mainWindow()->beginExit();
            qtCtx()->mainWindow()->close();
        }

        manager.release();
    }

    void saveState(QSessionManager &){ /** Do nothing */ }

private:
    EiskaltEventFilter ef;
};
