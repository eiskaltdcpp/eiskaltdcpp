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

#include "PublicHubs.h"
#include "dcpp/DCPlusPlus.h"
#include "MainWindow.h"
#include "WulforSettings.h"
#include "AutoToolTip.h"
#include "QtContext.h"
#include "QtContextAware.h"

#include <QApplication>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QHeaderView>
#include <QKeyEvent>

using namespace dcpp;

PublicHubs::PublicHubs(dcpp::DCContext& ctx, QWidget *parent) :
    QtContextAware(ctx),
    QWidget(parent), proxy(nullptr)
{
    setupUi(this);

    setUnload(false);

    model = new PublicHubModel();

    treeView->setModel(model);
    treeView->setItemDelegate(new AutoToolTipDelegate(treeView));
    treeView->header()->restoreState(qtCtx()->settings()->getVar(WS_PUBLICHUBS_STATE, QByteArray()).toByteArray());

    lineEdit_FILTER->installEventFilter(this);

    dcCtx().getFavoriteManager()->addListener(this);

    QString hubs = _q(dcCtx().getSettingsManager()->get(SettingsManager::HUBLIST_SERVERS));

    comboBox_HUBS->addItems(hubs.split(";", Qt::SkipEmptyParts));
    comboBox_HUBS->setCurrentIndex(dcCtx().getFavoriteManager()->getSelectedHubList());

    for (int i = 0; i < model->columnCount(); i++)
        comboBox_FILTER->addItem(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());

    comboBox_FILTER->setCurrentIndex(COLUMN_PHUB_DESC);

    frame->hide();

    entries = dcCtx().getFavoriteManager()->getPublicHubs();

    updateList();

    if(dcCtx().getFavoriteManager()->isDownloading()) {
        label_STATUS->setText(tr("Downloading public hub list..."));
    } else if(entries.empty()) {
        dcCtx().getFavoriteManager()->refresh();
    }

    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    toolButton_CLOSEFILTER->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEDITDELETE));

    connect(this, &PublicHubs::coreDownloadStarted,  this, &PublicHubs::setStatus, Qt::QueuedConnection);
    connect(this, &PublicHubs::coreDownloadFailed,   this, &PublicHubs::setStatus, Qt::QueuedConnection);
    connect(this, &PublicHubs::coreDownloadFinished, this, &PublicHubs::onFinished, Qt::QueuedConnection);
    connect(this, &PublicHubs::coreCacheLoaded,      this, &PublicHubs::onFinished, Qt::QueuedConnection);

    connect(treeView, &QTreeView::customContextMenuRequested, this, &PublicHubs::slotContextMenu);
    connect(treeView, &QTreeView::doubleClicked, this, &PublicHubs::slotDoubleClicked);
    connect(treeView->header(), &QHeaderView::customContextMenuRequested, this, &PublicHubs::slotHeaderMenu);
    connect(toolButton_CLOSEFILTER, &QToolButton::clicked, this, &PublicHubs::slotFilter);
    connect(comboBox_HUBS, qOverload<int>(&QComboBox::activated), this, &PublicHubs::slotHubChanged);
    connect(qtCtx()->settings(), &WulforSettings::strValueChanged, this, &PublicHubs::slotSettingsChanged);
    
    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
    if (comboBox_HUBS->count())
        slotHubChanged(comboBox_HUBS->currentIndex());
}

PublicHubs::~PublicHubs(){
    qtCtx()->settings()->setVar(WS_PUBLICHUBS_STATE, treeView->header()->saveState());
    
    delete model;
    delete proxy;

    dcCtx().getFavoriteManager()->removeListener(this);
}

bool PublicHubs::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if (static_cast<LineEdit*>(obj) == lineEdit_FILTER && k_e->key() == Qt::Key_Escape){
            lineEdit_FILTER->clear();

            requestFilter();

            return true;
        }
    }

    return QWidget::eventFilter(obj, e);
}

void PublicHubs::closeEvent(QCloseEvent *e){
    isUnload()? e->accept() : e->ignore();
}

void PublicHubs::setStatus(const QString &stat){
    label_STATUS->setText(stat);
}

void PublicHubs::updateList(){
    if (!model)
        return;

    model->clearModel();
    QList<QVariant> data;

    for (auto &i : entries) {
        HubEntry *entry = const_cast<HubEntry*>(&i);
        data.clear();

        data << _q(entry->getName())         << _q(entry->getDescription())  << entry->getUsers()
             << _q(entry->getServer())       << _q(entry->getCountry())      << (qlonglong)entry->getShared()
             << (qint64)entry->getMinShare() << (qint64)entry->getMinSlots() << (qint64)entry->getMaxHubs()
             << (qint64)entry->getMaxUsers() << static_cast<double>(entry->getReliability()) << _q(entry->getRating());

        model->addResult(data, entry);
    }
}

void PublicHubs::onFinished(const QString &stat){
    setStatus(stat);

    entries = dcCtx().getFavoriteManager()->getPublicHubs();

    updateList();
}

void PublicHubs::slotContextMenu(){
    QItemSelectionModel *sel_model = treeView->selectionModel();
    QModelIndexList indexes = sel_model->selectedRows(0);

    if (indexes.isEmpty())
        return;

    if (proxy)
        std::transform(indexes.begin(), indexes.end(), indexes.begin(), [&](QModelIndex i) { return proxy->mapToSource(i); });

    WulforUtil *WU = qtCtx()->wulforUtil();

    QMenu *m = new QMenu();
    QAction *connect = new QAction(WU->getPixmap(WulforUtil::eiCONNECT), tr("Connect"), m);
    QAction *add_fav = new QAction(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD), tr("Add to favorites"), m);
    QAction *copy    = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy &address to clipboard"), m);

    m->addActions(QList<QAction*>() << connect << add_fav << copy);

    QAction *ret = m->exec(QCursor::pos());

    m->deleteLater();

    if (ret == connect){
        PublicHubItem * item = nullptr;
        MainWindow *MW = qtCtx()->mainWindow();

        for (const auto &i : indexes){
            item = reinterpret_cast<PublicHubItem*>(i.internalPointer());

            if (item)
                MW->newHubFrame(item->data(COLUMN_PHUB_ADDRESS).toString(), "");

            item = nullptr;
        }
    }
    else if (ret == add_fav){
        PublicHubItem * item = nullptr;

        for (const auto &i : indexes){
            item = reinterpret_cast<PublicHubItem*>(i.internalPointer());

            if (item && item->entry){
                try{
                    FavoriteHubEntry fhe(dcCtx(), *item->entry);
                    dcCtx().getFavoriteManager()->addFavorite(fhe);
                }
                catch (const std::exception&){}
            }

            item = nullptr;
        }
    }
    else if (ret == copy){
        PublicHubItem * item = nullptr;
        QString out = "";

        for (const auto &i : indexes){
            item = reinterpret_cast<PublicHubItem*>(i.internalPointer());

            if (item)
                out += item->data(COLUMN_PHUB_ADDRESS).toString() + "\n";

            item = nullptr;
        }

        if (!out.isEmpty())
            qApp->clipboard()->setText(out, QClipboard::Clipboard);
    }
}

void PublicHubs::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void PublicHubs::slotDoubleClicked(const QModelIndex &index){
    if (!index.isValid())
        return;

    QModelIndex i = proxy? proxy->mapToSource(index) : index;

    PublicHubItem * item = reinterpret_cast<PublicHubItem*>(i.internalPointer());
    MainWindow *MW = qtCtx()->mainWindow();

    if (item)
        MW->newHubFrame(item->data(COLUMN_PHUB_ADDRESS).toString(), "");
}

void PublicHubs::slotFilter(){
    if (frame->isVisible()){
        treeView->setModel(model);

        disconnect(lineEdit_FILTER, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);

        delete proxy;
        proxy = nullptr;
    }
    else {
        proxy = new PublicHubProxyModel();
        proxy->setDynamicSortFilter(true);
        proxy->setFilterFixedString(lineEdit_FILTER->text());
        proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        proxy->setFilterKeyColumn(comboBox_FILTER->currentIndex());
        proxy->setSourceModel(model);

        treeView->setModel(proxy);

        connect(lineEdit_FILTER, &QLineEdit::textChanged, proxy, &QSortFilterProxyModel::setFilterFixedString);
        connect(comboBox_FILTER, qOverload<int>(&QComboBox::currentIndexChanged), this, &PublicHubs::slotFilterColumnChanged);

        lineEdit_FILTER->setFocus();

        if (!lineEdit_FILTER->text().isEmpty())
            lineEdit_FILTER->selectAll();
    }

    frame->setVisible(!frame->isVisible());
}

void PublicHubs::slotHubChanged(int pos){
    dcCtx().getFavoriteManager()->setHubList(pos);
    dcCtx().getFavoriteManager()->refresh();
}

void PublicHubs::slotFilterColumnChanged(){
    if (proxy)
        proxy->setFilterKeyColumn(comboBox_FILTER->currentIndex());

    if (comboBox_FILTER->hasFocus())
        lineEdit_FILTER->setFocus();
}

void PublicHubs::slotSettingsChanged(const QString &key, const QString&){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void PublicHubs::on(DownloadStarting, const std::string& l) noexcept{
    emit coreDownloadStarted(tr("Downloading public hub list... (%1)").arg(_q(l)));
}

void PublicHubs::on(DownloadFailed, const std::string& l) noexcept{
    emit coreDownloadFailed(tr("Download failed: %1").arg(_q(l)));
}

void PublicHubs::on(DownloadFinished, const std::string& l) noexcept{
    emit coreDownloadFinished(tr("Hub list downloaded... (%1)").arg(_q(l)));
}

void PublicHubs::on(LoadedFromCache, const std::string& l, const std::string& d) noexcept{
    emit coreCacheLoaded(tr("Locally cached (as of %1) version of the hub list loaded (%2)").arg(_q(d)).arg(_q(l)));
}

void PublicHubs::on(Corrupted, const string& l) noexcept {
    if (l.empty())
        emit coreDownloadFailed(tr("Cached hub list is corrupted or unsupported"));
    else
        emit coreDownloadFailed(tr("Downloaded hub list is corrupted or unsupported (%1)").arg(_q(l)));
}
