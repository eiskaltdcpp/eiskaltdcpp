#include "WulforManager.h"

WulforManager::WulforManager():
        abort(false),
        condTimer(NULL),
        condMutex(NULL)
{
}

WulforManager::~WulforManager()
{
    if (condTimer)
        condTimer->stop();

    clearQueues();

    delete condMutex;
    delete condTimer;
}

void WulforManager::start(){
    if (condMutex || condTimer)
        return;

    abort = false;

    condTimer = new QTimer(this);
    condTimer->setSingleShot(true);
    condTimer->setInterval(1000);

    condMutex = new QMutex();

    clearQueues();

    connect(condTimer, SIGNAL(timeout()), this, SLOT(condTimerDone()));

    condTimer->start();
}

void WulforManager::stop(){
    abort = true;

    condTimer->stop();

    clearQueues();

    delete condTimer;
    delete condMutex;

    condMutex   = NULL;
    condTimer   = NULL;
}

void WulforManager::processConditionQueue(){
    while (!abort){
        condMutex->lock();

        QMap<BFuncBase*, FuncBase* >::iterator it = condFuncs.begin();

        for (; it != condFuncs.end(); ++it){
            if (it.key()->call()){
                it.value()->call();

                it = (condFuncs.erase(it)-1);

                delete it.key();
                delete it.value();
            }
        }

        condMutex->unlock();

        return;
    }
}

void WulforManager::dispatchConditionFunc(BFuncBase *bfunc, FuncBase *func){
    if (!condMutex || !condTimer || !bfunc || !func || abort)
        return;

    condMutex->lock();

    condFuncs.insert(bfunc, func);

    condMutex->unlock();
}

void WulforManager::condTimerDone(){
    if (condFuncs.empty()){
        condTimer->start();

        return;
    }

    processConditionQueue();

    condTimer->start();
}

void WulforManager::clearQueues(){
    if (condMutex) condMutex->lock();
    {
        QMap<BFuncBase*, FuncBase*>::iterator it = condFuncs.begin();

        for(; it != condFuncs.end(); ++it){
            delete it.key();
            delete it.value();
        }

        condFuncs.clear();
    }
    if (condMutex) condMutex->unlock();
}
