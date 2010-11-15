/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SpyFrame.h"
#include "MainWindow.h"
#include "SpyModel.h"
#include "WulforUtil.h"
#include "SearchFrame.h"

#include <QMenu>
#include <QMessageBox>
#include <QItemSelectionModel>

using namespace dcpp;

SpyFrame::SpyFrame(QWidget *parent) :
    QWidget(parent)
{
    setupUi(this);

    setUnload(false);

    model = new SpyModel();

    treeView->setModel(model);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    MainWindow::getInstance()->addArenaWidget(this);

    connect(this, SIGNAL(coreIncomingSearch(QString,bool)), model, SLOT(addResult(QString,bool)), Qt::QueuedConnection);

    connect(pushButton, SIGNAL(clicked()), this, SLOT(slotStartStop()));
    connect(pushButton_CLEAR, SIGNAL(clicked()), this, SLOT(slotClear()));
    connect(treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu()));
}

SpyFrame::~SpyFrame(){
    ClientManager::getInstance()->removeListener(this);
}

void SpyFrame::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

        //setAttribute(Qt::WA_DeleteOnClose);

        //save();

        e->accept();
    }
    else {
        if (pushButton->text() == tr("Stop")){
            int ret = QMessageBox::question(this, tr("Search Spy"),
                                            tr("Search Spy is now running.\n"
                                               "It will continue to work when the widget is hidden.\n"
                                               "Do you want to stop it?\n"),
                                            QMessageBox::Yes | QMessageBox::No);
            if(ret == QMessageBox::Yes)
                slotStartStop();
        }

        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);

        e->ignore();
    }
}

void SpyFrame::slotClear(){
    model->clearModel();
}

void SpyFrame::slotStartStop(){
    static bool started = false;

    if (!started){
        pushButton->setText(tr("Stop"));

        ClientManager::getInstance()->addListener(this);
    }
    else {
        pushButton->setText(tr("Start"));

        ClientManager::getInstance()->removeListener(this);
    }

    started = !started;
}

void SpyFrame::contextMenu(){
    QModelIndexList list = treeView->selectionModel()->selectedRows(0);

    if (list.isEmpty())
        return;

    SpyItem *item = reinterpret_cast<SpyItem*>(list.at(0).internalPointer());

    QMenu *m = new QMenu(this);
    m->addAction(tr("Search"));

    QAction *ret = m->exec(QCursor::pos());

    if (!ret)
        return;

    SearchFrame *fr = new SearchFrame(this);
    QString src = item->data(COLUMN_SPY_STRING).toString();

    if (item->isTTH){
        src.remove(0, 4);

        fr->searchAlternates(src);
    }
    else
        fr->searchFile(src);

}

void SpyFrame::on(dcpp::ClientManagerListener::IncomingSearch, const string &s) throw(){
    bool isTTH = _q(s).startsWith("TTH:");

    if (checkBox_IGNORETTH->isChecked() && isTTH)
        return;

    emit coreIncomingSearch(_q(s).replace("$", " "), isTTH);

    if (checkBox_AUTOSCROLLING->isChecked())
        treeView->scrollToBottom();
}
