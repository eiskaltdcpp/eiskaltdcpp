#include "LogManagerScript.h"
#include "WulforUtil.h"

#include <QDateTime>

#include "dcpp/Util.h"

LogManagerScript::LogManagerScript(QObject *parent) :
    QObject(parent)
{
    dcpp::LogManager::getInstance()->addListener(this);
}

LogManagerScript::~LogManagerScript(){
    dcpp::LogManager::getInstance()->removeListener(this);
}

void LogManagerScript::on(Message, time_t t, const dcpp::string &msg) throw(){
    Q_UNUSED(t);

    emit message(QDateTime::currentDateTime().toString("hh:mm:ss"), _q(msg));
}
