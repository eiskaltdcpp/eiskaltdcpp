/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef PUBLICHUBSLIST_H
#define PUBLICHUBSLIST_H

#include <QDialog>

#include "ui_UIPublicHubsList.h"

class PublicHubsList:
        public QDialog,
        private Ui::UIPublicHubsList
{
Q_OBJECT
public:
    PublicHubsList(QWidget* = nullptr);

private slots:
    void slotAccepted();
    void slotUp();
    void slotDown();
    void slotAdd();
    void slotRem();
    void slotChange();
};

#endif // PUBLICHUBSLIST_H
