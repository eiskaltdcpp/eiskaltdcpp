#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

#include <QMap>
#include <QAction>
#include <QShortcut>

class ShortcutManager: public dcpp::Singleton<ShortcutManager>
{
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
