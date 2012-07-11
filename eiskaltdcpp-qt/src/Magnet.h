/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QDialog>
#include <QEvent>

#include "dcpp/stdinc.h"
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
    enum {
      MAGNET_ACTION_SHOW_UI = 0,
      MAGNET_ACTION_SEARCH = 1,
      MAGNET_ACTION_DOWNLOAD = 2
    };

private slots:
    void search();
    void download();
    void slotBrowse();
    void showUI(const QString &, const qulonglong &, const QString &);
    void search(const QString &, const qulonglong &, const QString &);
    void searchTTH(const QString&);
    void searchFile(const QString&);
    void download(const QString &, const qulonglong &, const QString &);
};
