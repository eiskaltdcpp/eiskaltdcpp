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

private:
    ShortcutManager();
    ShortcutManager(const ShortcutManager&){}
    virtual ~ShortcutManager();

    void load();
    void save();

    QMap<QString, QKeySequence> shortcuts;
};

#endif // SHORTCUTMANAGER_H
