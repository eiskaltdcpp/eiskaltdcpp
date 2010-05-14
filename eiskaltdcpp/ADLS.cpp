#include "ADLS.h"
#include "ADLSModel.h"
#include "MainWindow.h"
#include "WulforUtil.h"
#include "WulforSettings.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ADLSearch.h"

#include <QTreeView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QList>

using namespace dcpp;

ADLS::ADLS(QWidget *parent):
        QWidget(parent),
        model(NULL)
{
    setupUi(this);

    init();
}

ADLS::~ADLS(){

    //MainWindow::getInstance()->remArenaWidget(this);

    delete model;
}

void ADLS::closeEvent(QCloseEvent *e){
    if (isUnload()){
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);
        MainWindow::getInstance()->remArenaWidget(this);

        save();

        //setAttribute(Qt::WA_DeleteOnClose);

        e->accept();
    }
    else {
        MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
        MainWindow::getInstance()->remWidgetFromArena(this);

        e->ignore();
    }
}

QWidget *ADLS::getWidget(){
    return this;
}

QString ADLS::getArenaTitle(){
    return tr("ADLSearch");
}

QString ADLS::getArenaShortTitle(){
    return getArenaTitle();
}

QMenu *ADLS::getMenu(){
    return NULL;
}

void ADLS::load(){
    treeView->header()->restoreState(QByteArray::fromBase64(WSGET(WS_ADLS_STATE).toAscii()));
}

void ADLS::save(){
    WSSET(WS_ADLS_STATE, treeView->header()->saveState().toBase64());
}

void ADLS::init(){
    model = new ADLSModel();
    setUnload(false);

    treeView->setModel(model);


    treeView->setRootIsDecorated(false);
    treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->viewport()->setAcceptDrops(false); // temporary
    treeView->setDragEnabled(false); // temporary
    treeView->setAcceptDrops(false); // temporary
    //treeView->setDragDropMode(QAbstractItemView::InternalMove);

    MainWindow::getInstance()->addArenaWidget(this);

    WulforUtil *WU = WulforUtil::getInstance();

    add_newButton->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));
    changeButton->setIcon(WU->getPixmap(WulforUtil::eiEDIT));
    removeButton->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
    connectButton->setIcon(WU->getPixmap(WulforUtil::eiCONNECT));
    upButton->setIcon(WU->getPixmap(WulforUtil::eiUP));
    downButton->setIcon(WU->getPixmap(WulforUtil::eiDOWN));

    load();

    int row_num = model->rowCount();
    if (row_num == 0){
        changeButton->setEnabled(false);
        removeButton->setEnabled(false);
        connectButton->setEnabled(false);
        upButton->setEnabled(false);
        downButton->setEnabled(false);
    }
    else if(row_num == 1){
        upButton->setEnabled(false);
        downButton->setEnabled(false);
    }

    connect(treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(slotContexMenu(const QPoint&)));
    connect(treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(slotClicked(QModelIndex)));
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));
    connect(treeView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(slotDblClicked()));

    connect(add_newButton, SIGNAL(clicked()), this, SLOT(slotAdd_newButtonClicked()));
    connect(changeButton,  SIGNAL(clicked()), this, SLOT(slotChangeButtonClicked()));
    connect(removeButton,  SIGNAL(clicked()), this, SLOT(slotRemoveButtonClicked()));
    connect(connectButton, SIGNAL(clicked()), this, SLOT(slotConnectButtonClicked()));
    connect(upButton,      SIGNAL(clicked()), this, SLOT(slotUpButtonClicked()));
    connect(downButton,    SIGNAL(clicked()), this, SLOT(slotDownButtonClicked()));
}

void ADLS::slotContexMenu(const QPoint &){
    QItemSelectionModel *s_model = treeView->selectionModel();
    QModelIndexList list = s_model->selectedRows(0);
    WulforUtil *WU = WulforUtil::getInstance();
    bool empty = list.empty();
    QMenu *menu = new QMenu(this);

    if (empty){
        QAction *add_new = new QAction(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD), tr("Add new"), menu);
        menu->addAction(add_new);

        QAction *res = menu->exec(QCursor::pos());

        if (res){
            }
    } else {
        ADLSItem *item = static_cast<ADLSItem*>(list.at(0).internalPointer());

        if (!item){
            delete menu;

            return;
        }

        QAction *add_new = new QAction(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD), tr("Add new"), menu);
        QAction *change  = new QAction(WU->getPixmap(WulforUtil::eiEDIT), tr("Change"), menu);
        QAction *remove  = new QAction(WU->getPixmap(WulforUtil::eiEDITDELETE), tr("Delete"), menu);
        QAction *conn    = new QAction(WU->getPixmap(WulforUtil::eiCONNECT), tr("Connect"), menu);
        QAction *sep1    = new QAction(menu);
        QAction *sep2    = new QAction(menu);
        sep1->setSeparator(true);
        sep2->setSeparator(true);

        menu->addActions(QList<QAction*>() << change
                                           << remove
                                           << sep1
                                           << conn
                                           << sep2
                                           << add_new);

        QAction *res = menu->exec(QCursor::pos());

        if (res == change)
            slotChangeButtonClicked();
        else if (res == remove)
            slotRemoveButtonClicked();
        else if (res == conn)
            slotDblClicked();
        else if (res == add_new)
            slotAdd_newButtonClicked();
    }

    delete menu;
}

void ADLS::slotDblClicked(){
    ADLSItem *item = getItem();

    if (!item)
        return;
}

void ADLS::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void ADLS::slotClicked(const QModelIndex &index){
    if (!index.isValid() || index.column() != COLUMN_CHECK || !index.internalPointer())
        return;
}

void ADLS::slotAdd_newButtonClicked(){

}

void ADLS::slotChangeButtonClicked(){
    ADLSItem *item = getItem();

    if (!item)
        return;

}

void ADLS::slotRemoveButtonClicked(){
    ADLSItem *item = getItem();

    if (!item)
        return;

}

void ADLS::slotConnectButtonClicked(){
    slotDblClicked();
}

void ADLS::slotUpButtonClicked(){
    QItemSelectionModel *s_model = treeView->selectionModel();
    QModelIndexList list = s_model->selectedRows(0);

    foreach (QModelIndex i, list)
        s_model->select(model->moveUp(i), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

void ADLS::slotDownButtonClicked(){
    QItemSelectionModel *s_model = treeView->selectionModel();
    QModelIndexList list = s_model->selectedRows(0);

    foreach (QModelIndex i, list)
         s_model->select(model->moveDown(i), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Rows);
}

ADLSItem *ADLS::getItem(){
    QItemSelectionModel *s_model = treeView->selectionModel();
    QModelIndexList list = s_model->selectedRows(0);

    ADLSItem *item = NULL;
    if (!list.isEmpty())
        item = static_cast<ADLSItem*>(list.first().internalPointer());

    return item;
}
