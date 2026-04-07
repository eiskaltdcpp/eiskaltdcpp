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

#include "SpyFrame.h"
#include "dcpp/DCPlusPlus.h"
#include "SpyModel.h"
#include "WulforUtil.h"
#include "SearchFrame.h"
#include "ArenaWidgetFactory.h"
#include "QtContext.h"
#include "QtContextAware.h"

#include <QMenu>
#include <QMessageBox>
#include <QItemSelectionModel>

using namespace dcpp;

SpyFrame::SpyFrame(dcpp::DCContext& ctx, QWidget *parent)
    : QtContextAware(ctx)
    , QWidget(parent)
    , model(new SpyModel())
{
    setupUi(this);

    setUnload(false);

    treeView->setModel(model);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->restoreState(qtCtx()->settings()->getVar("spyframe-header-state", QByteArray()).toByteArray());

    connect(this, SIGNAL(coreIncomingSearch(QString,bool)), model, SLOT(addResult(QString,bool)), Qt::QueuedConnection);
    connect(pushButton, &QPushButton::clicked, this, &SpyFrame::slotStartStop);
    connect(pushButton_CLEAR, &QPushButton::clicked, this, &SpyFrame::slotClear);
    connect(treeView, &QWidget::customContextMenuRequested, this, &SpyFrame::contextMenu);
    connect(qtCtx()->settings(), &WulforSettings::strValueChanged, this, &SpyFrame::slotSettingsChanged);
    
    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

SpyFrame::~SpyFrame(){
    qtCtx()->settings()->setVar("spyframe-header-state", treeView->header()->saveState());
    
    dcCtx().getClientManager()->removeListener(this);
}

void SpyFrame::closeEvent(QCloseEvent *e){
    if (isUnload()){
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

        dcCtx().getClientManager()->addListener(this);
    }
    else {
        pushButton->setText(tr("Start"));

        dcCtx().getClientManager()->removeListener(this);
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

    SearchFrame *fr = ArenaWidgetFactory().create<SearchFrame, QWidget*>(this);
    QString src = item->data(COLUMN_SPY_STRING).toString();

    if (item->isTTH){
        src.remove(0, 4);

        fr->searchAlternates(src);
    }
    else
        fr->searchFile(src);

}
void SpyFrame::slotSettingsChanged(const QString &key, const QString&){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void SpyFrame::on(dcpp::ClientManagerListener::IncomingSearch, const string &s) noexcept{
    bool isTTH = _q(s).startsWith("TTH:");

    if (checkBox_IGNORETTH->isChecked() && isTTH)
        return;

    emit coreIncomingSearch(_q(s).replace("$", " "), isTTH);

    if (checkBox_AUTOSCROLLING->isChecked()){
        treeView->scrollToTop();
        model->setSort(false);
    }
    else{
        model->setSort(true);
    }
}
