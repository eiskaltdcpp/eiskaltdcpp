#ifndef LOGMANAGERSCRIPT_H
#define LOGMANAGERSCRIPT_H

#include <QObject>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/LogManager.h"
#include "dcpp/LogManagerListener.h"

class LogManagerScript :
        public QObject,
        public dcpp::Singleton<LogManagerScript>,
        public dcpp::LogManagerListener
{
Q_OBJECT
friend class dcpp::Singleton<LogManagerScript>;

Q_SIGNALS:
    void message(const QString &tstamp, const QString &msg);

protected:
    virtual void on(Message, time_t, const dcpp::string&) throw();

private:
    LogManagerScript(QObject *parent = 0);
    LogManagerScript(const LogManagerScript&){}
    ~LogManagerScript();
    LogManagerScript &operator=(const LogManagerScript&){}
};

#endif // LOGMANAGERSCRIPT_H
