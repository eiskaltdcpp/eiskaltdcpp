/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ShareBrowserSearch.h"
#include "WulforUtil.h"
#include "ShareBrowser.h"
#include "FileBrowserModel.h"

#include <QCloseEvent>
#include <QDir>

ShareBrowserSearch::ShareBrowserSearch(FileBrowserModel *model, QWidget *parent): QDialog(parent), searchRoot(NULL) {
    if ( !model )
        throw 0;
    
    setupUi(this);
    
    this->model = model;

    if (WVGET("sharebrowsersearch/size").isValid())
        resize(WVGET("sharebrowsersearch/size").toSize());

    treeWidget->header()->restoreState(WVGET("sharebrowsersearch/columnstate").toByteArray());

    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(pushButton_SEARCH, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
    connect(this, SIGNAL(gotItem(QString,FileBrowserItem*)), this, SLOT(slotGotItem(QString,FileBrowserItem*)), Qt::QueuedConnection);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));

    show();
}

ShareBrowserSearch::~ShareBrowserSearch(){
    hash.clear();

    treeWidget->clear();
}

void ShareBrowserSearch::closeEvent(QCloseEvent *e){
    WVSET("sharebrowsersearch/size", size());
    WVSET("sharebrowsersearch/columnstate", treeWidget->header()->saveState());

    QDialog::closeEvent(e);
}

void ShareBrowserSearch::setSearchRoot(FileBrowserItem *root){
    searchRoot = root;
}

void ShareBrowserSearch::slotStartSearch(){
    treeWidget->clear();

    hash.clear();

    label_STATS->setText("");

    if (lineEdit_SEARCHSTR->text().isEmpty())
        return;

    setWindowTitle(tr("Search - %1").arg(lineEdit_SEARCHSTR->text()));

    AsyncRunner *runner = new AsyncRunner(this);
    runner->setRunFunction([this]() { this->findMatches(this->searchRoot); });

    connect(runner, SIGNAL(finished()), runner, SLOT(deleteLater()));

    regexp = QRegExp(lineEdit_SEARCHSTR->text(), Qt::CaseInsensitive, QRegExp::Wildcard);

    runner->start();
}

static QString getPath(FileBrowserItem *path){
    QString p = "";

    while (path){
        p = path->data(COLUMN_FILEBROWSER_NAME).toString() + QDir::separator() + p;

        path = path->parent();
    }

    return p;
}

void ShareBrowserSearch::slotGotItem(QString item, FileBrowserItem *path){
    if (!path || item.isEmpty())
        return;

    QTreeWidgetItem *i = new QTreeWidgetItem(treeWidget, QStringList() << item << getPath(path), 0);

    items.push_back(i);
    hash.insert(i, path);

    treeWidget->insertTopLevelItem(0, i);

    label_STATS->setText(QString(tr("Found %1 items")).arg(items.size()));
}

void ShareBrowserSearch::slotItemActivated(QTreeWidgetItem *item, int){
    auto it = hash.find(item);

    if (it != hash.end())
        emit indexClicked(it.value());
}

void ShareBrowserSearch::findMatches(FileBrowserItem *item){
    if (!item)
        return;
    
    QModelIndex index = model->createIndexForItem(item);
    
    if (model->canFetchMore(index))  
        model->fetchMore(index);
    
    QString fname = "";
    int type_search = comboBox_TYPE_SEARCH->currentIndex();
    for (const auto &i : item->childItems){
        if (i->dir){
            if (type_search == 1 || type_search == 2) {
                fname = _q(i->dir->getName());
                if (fname.indexOf(lineEdit_SEARCHSTR->text(), 0, Qt::CaseInsensitive) >= 0 || fname.indexOf(regexp) >= 0 || regexp.exactMatch(fname))
                    emit gotItem(_q(i->dir->getName()), item);
            }
            findMatches(i);
            if (type_search == 0 || type_search == 2) {
                DirectoryListing::File::List *files = &i->dir->files;
                DirectoryListing::File::Iter it_file;

                for (it_file = files->begin(); it_file != files->end(); ++it_file){
                    fname = _q((*it_file)->getName());

                    if (fname.indexOf(lineEdit_SEARCHSTR->text(), 0, Qt::CaseInsensitive) >= 0 || fname.indexOf(regexp) >= 0 || regexp.exactMatch(fname))
                        emit gotItem(_q((*it_file)->getName()), i);
                }
            }
        }
    }
}
