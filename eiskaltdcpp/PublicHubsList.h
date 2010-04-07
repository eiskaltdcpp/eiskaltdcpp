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
    PublicHubsList(QWidget* = NULL);

private slots:
    void slotAccepted();
    void slotUp();
    void slotDown();
    void slotAdd();
    void slotRem();
};

#endif // PUBLICHUBSLIST_H
