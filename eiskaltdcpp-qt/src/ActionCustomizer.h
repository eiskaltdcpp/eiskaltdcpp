/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef ACTIONCUSTOMIZER_H
#define ACTIONCUSTOMIZER_H

#include <QDialog>
#include <QList>
#include <QAction>
#include <QMap>

#include "ui_UIActionCustomizer.h"

class ActionCustomizer :
        public QDialog,
        private Ui::UIActionCustomizer
{
Q_OBJECT
public:
    explicit ActionCustomizer(const QList<QAction*> &available, const QList<QAction*> &enabled, QWidget *parent = 0);

Q_SIGNALS:
    void done(const QList<QAction*> &enabled);

private Q_SLOTS:
    void moveDown();
    void moveUp();
    void moveToEnabled();
    void moveToAvailable();
    void accepted();

private:
    QMap<QListWidgetItem*, QAction*> enabled_items;
    QMap<QListWidgetItem*, QAction*> avail_items;
};

#endif // ACTIONCUSTOMIZER_H
