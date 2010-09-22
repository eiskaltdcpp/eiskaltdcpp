/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MAGNET_H
#define MAGNET_H

#include <QDialog>
#include <QEvent>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SearchResult.h"
#include "dcpp/SearchManager.h"

#include "ui_UIMagnet.h"

class Magnet :
        public QDialog,
        private Ui::UIMagnet
{
Q_OBJECT
public:
    explicit Magnet(QWidget *parent = 0);
    virtual ~Magnet();

    void setLink(const QString&);

private slots:
    void search();
    void download();
    void slotBrowse();
    void showUI(const QString &, const qulonglong &, const QString &);
    void search(const QString&);
    void download(const QString &, const qulonglong &, const QString &);
};

#endif // MAGNET_H
