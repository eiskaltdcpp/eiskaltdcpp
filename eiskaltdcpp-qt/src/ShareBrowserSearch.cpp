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

#include "ShareBrowserSearch.h"
#include "QtContextAware.h"
#include "QtContext.h"
#include "WulforUtil.h"
#include "ShareBrowser.h"
#include "FileBrowserModel.h"

#include <QCloseEvent>
#include <QDir>
#include <QComboBox>

ShareBrowserSearch::ShareBrowserSearch(FileBrowserModel *model, QWidget *parent): QDialog(parent), searchRoot(nullptr) {
    if ( !model )
        throw 0;
    
    setupUi(this);
    
    this->model = model;

    if (qtCtx()->settings()->getVar("sharebrowsersearch/size").isValid())
        resize(qtCtx()->settings()->getVar("sharebrowsersearch/size").toSize());

    comboBox_TYPE_SEARCH->setCurrentIndex(qtCtx()->settings()->getVar("sharebrowsersearch/currenttype").toInt());
    treeWidget->header()->restoreState(qtCtx()->settings()->getVar("sharebrowsersearch/columnstate").toByteArray());

    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(pushButton_SEARCH, &QPushButton::clicked, this, &ShareBrowserSearch::slotStartSearch);
    connect(this, &ShareBrowserSearch::gotItem, this, &ShareBrowserSearch::slotGotItem, Qt::QueuedConnection);
    connect(treeWidget, &QTreeWidget::itemActivated, this, &ShareBrowserSearch::slotItemActivated);

    show();
}

ShareBrowserSearch::~ShareBrowserSearch(){
    hash.clear();

    treeWidget->clear();
}

void ShareBrowserSearch::closeEvent(QCloseEvent *e){
    qtCtx()->settings()->setVar("sharebrowsersearch/size", size());
    qtCtx()->settings()->setVar("sharebrowsersearch/columnstate", treeWidget->header()->saveState());
    qtCtx()->settings()->setVar("sharebrowsersearch/currenttype", comboBox_TYPE_SEARCH->currentIndex());

    QDialog::closeEvent(e);
}

void ShareBrowserSearch::setSearchRoot(FileBrowserItem *root){
    searchRoot = root;
}

void ShareBrowserSearch::slotStartSearch(){
    treeWidget->clear();

    hash.clear();
    items.clear();

    label_STATS->setText("");

    if (lineEdit_SEARCHSTR->text().isEmpty())
        return;

    setWindowTitle(tr("Search - %1").arg(lineEdit_SEARCHSTR->text()));

    AsyncRunner *runner = new AsyncRunner(this);
    runner->setRunFunction([this]() { this->findMatches(this->searchRoot); });

    connect(runner, &QThread::finished, runner, &QObject::deleteLater);

    regexp = QRegularExpression(
        QRegularExpression::wildcardToRegularExpression(lineEdit_SEARCHSTR->text()),
        QRegularExpression::CaseInsensitiveOption);

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

void ShareBrowserSearch::slotGotItem(const QString &item, FileBrowserItem *path){
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
                if (fname.indexOf(lineEdit_SEARCHSTR->text(), 0, Qt::CaseInsensitive) >= 0 || regexp.match(fname).hasMatch())
                    emit gotItem(_q(i->dir->getName()), item);
            }
            findMatches(i);
            if (type_search == 0 || type_search == 2) {
                for (const auto &it_file : i->dir->files){
                    fname = _q(it_file->getName());

                    if (fname.indexOf(lineEdit_SEARCHSTR->text(), 0, Qt::CaseInsensitive) >= 0 || regexp.match(fname).hasMatch())
                        emit gotItem(_q(it_file->getName()), i);
                }
            }
        }
    }
}
