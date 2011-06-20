/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SHAREBROWSERSEARCH_H
#define SHAREBROWSERSEARCH_H

#include <QDialog>
#include <QRegExp>
#include <QHash>
#include <QList>

#include "ui_UIShareBrowserSearch.h"

class FileBrowserItem;
class QCloseEvent;

class ShareBrowserSearch: public QDialog, protected Ui::UIShareBrowserSearch{
    Q_OBJECT

public:
    ShareBrowserSearch(QWidget * = NULL);
    virtual ~ShareBrowserSearch();

    void setSearchRoot(FileBrowserItem *);

protected:
    void closeEvent(QCloseEvent *);

Q_SIGNALS:
    void indexClicked(FileBrowserItem*);
    void gotItem(QString item, FileBrowserItem *path);

private Q_SLOTS:
    void slotStartSearch();
    void slotGotItem(QString,FileBrowserItem*);
    void slotItemActivated(QTreeWidgetItem*,int);

private:
    void findMatches(FileBrowserItem *);

    FileBrowserItem *searchRoot;
    QRegExp regexp;
    QList<QTreeWidgetItem*> items;
    QHash<QTreeWidgetItem*,FileBrowserItem*> hash;
};

#endif
