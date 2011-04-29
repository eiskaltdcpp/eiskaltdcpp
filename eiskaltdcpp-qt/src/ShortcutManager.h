/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

#include <QObject>
#include <QMap>
#include <QAction>
#include <QShortcut>

class ShortcutManager: public QObject, public dcpp::Singleton<ShortcutManager>
{
    Q_OBJECT

friend class dcpp::Singleton<ShortcutManager>;

public:
    bool registerShortcut(QAction *act, const QString &key);
    bool updateShortcut(QAction *act, const QString &key);
    QMap<QString, QKeySequence> getShortcuts() { return shortcuts; }

    void save();

private:
    ShortcutManager();
    ShortcutManager(const ShortcutManager&){}
    virtual ~ShortcutManager();

    void load();

    QMap<QString, QKeySequence> shortcuts;
};

#endif // SHORTCUTMANAGER_H
