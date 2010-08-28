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
#include <QtDebug>

using namespace dcpp;

ADLS::ADLS(QWidget *parent):
        QWidget(parent),
        model(NULL)
{
    setupUi(this);

    init();
}

ADLS::~ADLS(){
    ADLSearchManager::getInstance()->Save();
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

    ADLSearchManager::SearchCollection& collection = ADLSearchManager::getInstance()->collection;

    for (ADLSearchManager::SearchCollection::iterator i = collection.begin(); i != collection.end(); ++i) {
        ADLSearch &search = *i;
        addItem(search);
    }

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
    upButton->setIcon(WU->getPixmap(WulforUtil::eiUP));
    downButton->setIcon(WU->getPixmap(WulforUtil::eiDOWN));
    line_2->hide();
    upButton->hide();
    downButton->hide();
    load();

    int row_num = model->rowCount();
    if (row_num == 0){
        changeButton->setEnabled(false);
        removeButton->setEnabled(false);
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
            slotAdd_newButtonClicked();
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
        QAction *sep1    = new QAction(menu);
        sep1->setSeparator(true);

        menu->addActions(QList<QAction*>() << change
                                           << remove
                                           << sep1
                                           << add_new);

        QAction *res = menu->exec(QCursor::pos());

        if (res == change)
            slotChangeButtonClicked();
        else if (res == remove)
            slotRemoveButtonClicked();
        else if (res == add_new)
            slotAdd_newButtonClicked();
    }

    delete menu;
}

void ADLS::slotDblClicked(){
    slotChangeButtonClicked();
}

void ADLS::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void ADLS::slotClicked(const QModelIndex &index){
    if (!index.isValid() || index.column() != COLUMN_CHECK || !index.internalPointer())
        return;

    ADLSItem *item = reinterpret_cast<ADLSItem*>(index.internalPointer());
    ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
    int i = item->row();
    ADLSearch entry = collection[i];

        bool check = !item->data(COLUMN_CHECK).toBool();

        entry.isActive = check;
        item->updateColumn(COLUMN_CHECK, check);
        if (i < collection.size())
            collection[i] = entry;
        model->repaint();

        ADLSearchManager::getInstance()->Save();
}

void ADLS::initEditor(ADLSEditor &editor){

}
void ADLS::slotAdd_newButtonClicked(){
    ADLSEditor editor;
    ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
    ADLSearch search;

    initEditor(editor);

    if (editor.exec() == QDialog::Accepted){
        StrMap map;

        getParams(editor, map);
        updateEntry(search, map);
        collection.push_back(search);
        ADLSearchManager::getInstance()->Save();
        addItem(search);
    }
}

void ADLS::slotChangeButtonClicked(){
    ADLSItem *item = getItem();

    if (!item)
        return;

    int i = item->row();

    ADLSEditor editor;
    ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
    ADLSearch search = collection[i];

        StrMap map;

        getParams(search, map);
        initEditor(editor, map);

        if (editor.exec() == QDialog::Accepted){
            getParams(editor, map);
            updateItem(item, map);
            updateEntry(search, map);
            if (i < collection.size())
                collection[i] = search;

        }

}

void ADLS::slotRemoveButtonClicked(){
    ADLSItem *item = getItem();

    if (!item)
        return;
    int i = item->row();
    ADLSearchManager::SearchCollection &collection = ADLSearchManager::getInstance()->collection;
        if (i < collection.size()) {
            collection.erase(collection.begin() + i);
            model->removeItem(item);
        }
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

void ADLS::updateItem(ADLSItem *item, StrMap &map) {
    if (!item)
        return;

    WulforUtil *WU = WulforUtil::getInstance();

    item->updateColumn(COLUMN_CHECK, map["CHECK"]);
    item->updateColumn(COLUMN_SSTRING, map["SSTRING"]);
    item->updateColumn(COLUMN_DIRECTORY, map["DIRECTORY"]);
    item->updateColumn(COLUMN_MINSIZE, map["MINSIZE"]);
    item->updateColumn(COLUMN_MAXSIZE, map["MAXSIZE"]);
    item->updateColumn(COLUMN_TYPESIZE, SizeTypeToString((ADLSearch::SizeType)map["TYPESIZE"].toInt()));
    item->updateColumn(COLUMN_TYPE, SourceTypeToString((ADLSearch::SourceType)map["SOURCETYPE"].toInt()));

}

void ADLS::updateEntry(ADLSearch &entry, StrMap &map){
    entry.isActive = (int)map["CHECK"].toBool();
    entry.searchString = map["SSTRING"].toString().toStdString();
    entry.destDir = map["DIRECTORY"].toString().toStdString();
    entry.isAutoQueue = (int)map["AUTOQUEUE"].toBool();
    entry.sourceType = (ADLSearch::SourceType)map["SOURCETYPE"].toInt();
    entry.minFileSize = map["MINSIZE"].toLongLong();
    entry.maxFileSize = map["MAXSIZE"].toLongLong();
    entry.typeFileSize = (ADLSearch::SizeType)map["TYPESIZE"].toInt();

}
void ADLS::getParams(const ADLSEditor &editor, StrMap &map){
    WulforUtil *WU = WulforUtil::getInstance();

    map["SSTRING"]      = editor.lineEdit_SSTRING->text();
    map["DIRECTORY"]    = editor.lineEdit_DIRECTORY->text();
    map["AUTOQUEUE"]    = editor.checkBox_DOWNLOAD->isChecked();
    map["CHECK"]        = editor.checkBox_CHECK->isChecked();
    map["SOURCETYPE"]   = editor.comboBox_TYPE->currentIndex();
    map["TYPESIZE"]     = editor.comboBox_TYPESIZE->currentIndex();
    map["MINSIZE"]      = editor.spinBox_MINSIZE->value();
    map["MAXSIZE"]      = editor.spinBox_MAXSIZE->value();

}
void ADLS::initEditor(ADLSEditor &editor, StrMap &map){
    initEditor(editor);

    editor.checkBox_CHECK->setChecked(map["CHECK"].toBool());
    editor.checkBox_DOWNLOAD->setChecked(map["AUTOQUEUE"].toBool());
    editor.lineEdit_SSTRING->setText(map["SSTRING"].toString());
    editor.lineEdit_DIRECTORY->setText(map["DIRECTORY"].toString());
    editor.spinBox_MINSIZE->setValue(map["MINSIZE"].toLongLong());
    editor.spinBox_MAXSIZE->setValue(map["MAXSIZE"].toLongLong());
    editor.comboBox_TYPESIZE->setCurrentIndex(map["TYPESIZE"].toInt());
    editor.comboBox_TYPE->setCurrentIndex(map["SOURCETYPE"].toInt());
}
void ADLS::getParams(/*const*/ ADLSearch &entry, StrMap &map){

    map["SSTRING"]     = _q(entry.searchString);
    map["DIRECTORY"]   = _q(entry.destDir);
    map["CHECK"]       = entry.isActive;
    map["AUTOQUEUE"]   = entry.isAutoQueue;
    map["MINSIZE"]     = (qlonglong)entry.minFileSize;
    map["MAXSIZE"]     = (qlonglong)entry.maxFileSize;
    map["SOURCETYPE"]  = entry.sourceType;
    map["TYPESIZE"]    = entry.typeFileSize;

}
void ADLS::addItem(ADLSearch &search){
        QList<QVariant> data;

        data << search.isActive
             << _q(search.searchString)
             << SourceTypeToString(search.sourceType)
             << _q(search.destDir)
             << (qlonglong)search.minFileSize
             << (qlonglong)search.maxFileSize
             << SizeTypeToString(search.typeFileSize);


        model->addResult(data);
}
QString ADLS::SourceTypeToString(ADLSearch::SourceType t){
    switch(t) {
        default:
        case ADLSearch::OnlyFile:      return tr("Filename");
        case ADLSearch::OnlyDirectory: return tr("Directory");
        case ADLSearch::FullPath:      return tr("Full Path");
    }
}
QString ADLS::SizeTypeToString(ADLSearch::SizeType t){
    switch(t) {
        default:
        case ADLSearch::SizeBytes:     return tr("B");
        case ADLSearch::SizeKibiBytes: return tr("KiB");
        case ADLSearch::SizeMebiBytes: return tr("MiB");
        case ADLSearch::SizeGibiBytes: return tr("GiB");
    }
}
