/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QPixmap>
#include <QLabel>
#include <QMouseEvent>

class EmoticonLabel: public QLabel{
Q_OBJECT
public:
    EmoticonLabel(QWidget *parent = NULL) : QLabel(parent){}
    virtual ~EmoticonLabel(){}

Q_SIGNALS:
    void clicked();

protected:
    virtual void mousePressEvent(QMouseEvent *ev){
        QLabel::mousePressEvent(ev);

        emit clicked();
    }
};

struct EmoticonObject{
    QString fileName;
    QPixmap pixmap;

    int id;
};

typedef QMap<QString, EmoticonObject*> EmoticonMap;
typedef QList<EmoticonObject*> EmoticonList;
