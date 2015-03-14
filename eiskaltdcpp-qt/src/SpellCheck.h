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
#include <QStringList>

#include <aspell.h>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"

class SpellCheck :
        public QObject,
        public dcpp::Singleton<SpellCheck>
{
Q_OBJECT
friend class dcpp::Singleton<SpellCheck>;

public:
    bool ok(const QString &word);
    void suggestions(const QString &word, QStringList &list);
    void addToDict(const QString &word);

private:
    SpellCheck(QObject *parent = 0);
    ~SpellCheck();

    AspellSpeller *spell_checker;
};
