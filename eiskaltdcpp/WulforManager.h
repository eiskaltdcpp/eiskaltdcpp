/*
 * Copyright © 2004-2008 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#ifndef WULFOR_MANAGER_HH
#define WULFOR_MANAGER_HH

#include <QTimer>
#include <QMutex>
#include <QList>
#include <QMap>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

#include "Func.h"

class WulforManager:
        public QObject,
        public dcpp::Singleton<WulforManager>
{
Q_OBJECT

friend class dcpp::Singleton<WulforManager>;

public:
    void start();
    void stop();

    void dispatchConditionFunc(BFuncBase*, FuncBase*);

private slots:
    void condTimerDone();

private:
    WulforManager();
    virtual ~WulforManager();

    void processConditionQueue();

    void clearQueues();

    bool abort;

    QTimer *condTimer;
    QMutex *condMutex;
    QMap <BFuncBase*, FuncBase*> condFuncs;
};

#else
class WulforManager;
#endif
