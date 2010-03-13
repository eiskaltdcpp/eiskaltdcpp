#ifndef EMOTICONOBJECT_H
#define EMOTICONOBJECT_H

#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>

struct EmoticonObject{
    unsigned long left, right, top, bottom, id;
};

typedef QMap<QString, EmoticonObject*> EmoticonMap;
typedef QList<EmoticonObject*> EmoticonList;

#endif // EMOTICONOBJECT_H
