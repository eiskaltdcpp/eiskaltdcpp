/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ShortcutManager.h"
#include "WulforUtil.h"

#include "dcpp/Util.h"

#include <QFile>
#include <QTextStream>
#include <QStringList>

using namespace dcpp;

ShortcutManager::ShortcutManager(){
    load();
}

ShortcutManager::~ShortcutManager(){
    save();
}

void ShortcutManager::load(){
#ifdef _WIN32
    QString configFile = QString::fromUtf8( Util::getPath(Util::PATH_USER_CONFIG).c_str() ) + "shortcuts.txt";
#else
    QString configFile = _q(Util::getPath(Util::PATH_USER_CONFIG)) + "shortcuts.txt";
#endif

    QFile f(configFile);

    if (!(f.exists() && f.open(QIODevice::ReadOnly)))
        return;

    QTextStream stream(&f);
    QString str = "";
    QStringList ops;

    while (!stream.atEnd()){
        str = stream.readLine();

        ops = str.split(" ");

        if (ops.count() != 2)
            continue;

        shortcuts.insert(ops.at(0), QKeySequence::fromString(ops.at(1)));
    }

    f.close();
}

void ShortcutManager::save(){
#ifdef _WIN32
    QString configFile = QString::fromUtf8( Util::getPath(Util::PATH_USER_CONFIG).c_str() ) + "shortcuts.txt";
#else
    QString configFile = _q(Util::getPath(Util::PATH_USER_CONFIG)) + "shortcuts.txt";
#endif

    QFile f(configFile);

    if (!f.open(QIODevice::WriteOnly))
        return;

    QTextStream stream(&f);
    auto it = shortcuts.begin();

    for (; it != shortcuts.end(); ++it)
        stream << it.key() << " " << it.value().toString() << "\n";

    f.close();
}

bool ShortcutManager::registerShortcut(QAction *act, const QString &key){
    if (!act || act->objectName().isEmpty())
        return false;

    QString objName = act->objectName();
    auto it = shortcuts.find(objName);
    QKeySequence sq = (it != shortcuts.end())? (it.value()) : (QKeySequence::fromString(key));

    act->setShortcut(sq);

    shortcuts.insert(objName, sq);

    return true;
}

bool ShortcutManager::updateShortcut(QAction *act, const QString &key){
    if (!act || act->objectName().isEmpty())
        return false;

    QString objName = act->objectName();
    auto it = shortcuts.find(objName);

    if (it == shortcuts.end())//not registered shortcut
        return false;

    QKeySequence sq = QKeySequence::fromString(key);

    act->setShortcut(sq);
    shortcuts.insert(objName, sq);

    return true;
}
