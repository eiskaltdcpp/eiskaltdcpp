/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
