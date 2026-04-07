/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#pragma once

#include <QDialog>
#include <QRegularExpression>
#include <QHash>
#include <QList>

#include "ui_UIShareBrowserSearch.h"

class FileBrowserItem;
class FileBrowserModel;
class QCloseEvent;

class ShareBrowserSearch: public QDialog, protected Ui::UIShareBrowserSearch{
    Q_OBJECT

public:
    ShareBrowserSearch(FileBrowserModel *model, QWidget *parent = nullptr);
    virtual ~ShareBrowserSearch();

    void setSearchRoot(FileBrowserItem *);

protected:
    void closeEvent(QCloseEvent *);

Q_SIGNALS:
    void indexClicked(FileBrowserItem*);
    void gotItem(QString item, FileBrowserItem *path);

private Q_SLOTS:
    void slotStartSearch();
    void slotGotItem(const QString &, FileBrowserItem*);
    void slotItemActivated(QTreeWidgetItem*,int);

private:
    void findMatches(FileBrowserItem *);

    FileBrowserItem *searchRoot;
    QRegularExpression regexp;
    QList<QTreeWidgetItem*> items;
    QHash<QTreeWidgetItem*,FileBrowserItem*> hash;
    FileBrowserModel *model;
};
