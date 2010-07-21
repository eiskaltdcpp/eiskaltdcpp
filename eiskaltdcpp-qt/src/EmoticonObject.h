/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

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
