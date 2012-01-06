/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SearchBlacklistDialog.h"
#include "SearchBlacklist.h"
#include "WulforUtil.h"

#include <QComboBox>
#include <QLineEdit>
#include <QMenu>
#include <QItemSelectionModel>
#include <QResizeEvent>

SearchBlackListDialog::SearchBlackListDialog(QWidget *parent): QDialog(parent){
    setupUi(this);

    model = new SearchBlackListModel();
    treeView_RULES->setModel(model);
    treeView_RULES->setItemDelegate(new SearchBlackListDelegate(this));
    treeView_RULES->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RULES->setSortingEnabled(true);
    treeView_RULES->sortByColumn(COLUMN_SBL_KEY, Qt::AscendingOrder);

    connect(treeView_RULES, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
    connect(this, SIGNAL(accepted()), this, SLOT(ok()));
}

SearchBlackListDialog::~SearchBlackListDialog(){
    model->deleteLater();
}

void SearchBlackListDialog::ok(){
    model->save();
}

void SearchBlackListDialog::resizeEvent(QResizeEvent *e){
    e->accept();

    treeView_RULES->resizeColumnToContents(COLUMN_SBL_TYPE);

    int sblTypeWidth = treeView_RULES->columnWidth(COLUMN_SBL_TYPE);
    int sblKeyWidth = treeView_RULES->contentsRect().width() - sblTypeWidth;

    treeView_RULES->setColumnWidth(COLUMN_SBL_KEY, sblKeyWidth);
}

void SearchBlackListDialog::slotContextMenu(){
    QItemSelectionModel *s_m = treeView_RULES->selectionModel();
    QModelIndexList indexes = s_m->selectedRows(0);

    QMenu *menu = new QMenu(this);
    QAction *add = new QAction(WICON(WulforUtil::eiEDITADD), tr("Add new"), nullptr);
    QAction *rem = new QAction(WICON(WulforUtil::eiEDITDELETE), tr("Remove"), nullptr);

    menu->addActions(QList<QAction*>() << add << rem);

    QAction *ret = menu->exec(QCursor::pos());

    menu->deleteLater();

    if (ret == add){
        s_m->select(model->addEmptyItem(), QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    }
    else if (ret && !indexes.isEmpty()){
        foreach (const QModelIndex &index, indexes){
            SearchBlackListItem *i = reinterpret_cast<SearchBlackListItem*>(index.internalPointer());

            if (!i)
                continue;

            i->parent()->childItems.removeAt(i->row());
        }

        model->repaint();
    }
}

SearchBlackListModel::SearchBlackListModel(QObject * parent) :
        QAbstractItemModel(parent),
        sortColumn(COLUMN_SBL_KEY)
{
    rootItem = new SearchBlackListItem(nullptr);

    SearchBlacklist *SB = SearchBlacklist::getInstance();

    QList<QString> names = SB->getList(SearchBlacklist::NAME);
    QList<QString> tths  = SB->getList(SearchBlacklist::TTH);

    foreach (const QString &name, names){
        SearchBlackListItem * item = new SearchBlackListItem(rootItem);
        item->title = name;
        item->argument = SearchBlacklist::NAME;

        rootItem->appendChild(item);
    }

    foreach (const QString &tth, tths){
        SearchBlackListItem * item = new SearchBlackListItem(rootItem);
        item->title = tth;
        item->argument = SearchBlacklist::TTH;

        rootItem->appendChild(item);
    }

    sortColumn = -1;
}


SearchBlackListModel::~SearchBlackListModel() {
    delete rootItem;
}

void SearchBlackListModel::save(){
    QList<QString> names;
    QList<QString> tths;

    foreach (SearchBlackListItem *item, rootItem->childItems){
        QList<QString> &l = (item->argument == SearchBlacklist::NAME? names : tths);

        l.push_back(item->title);
    }

    SearchBlacklist *SB = SearchBlacklist::getInstance();
    SB->setList(SearchBlacklist::NAME, names);
    SB->setList(SearchBlacklist::TTH, tths);
}

int SearchBlackListModel::rowCount(const QModelIndex & ) const {
    return rootItem->childCount();
}

int SearchBlackListModel::columnCount(const QModelIndex & ) const {
    return 2;
}

Qt::ItemFlags SearchBlackListModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant SearchBlackListModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    SearchBlackListItem * item = static_cast<SearchBlackListItem*>(index.internalPointer());

    if (!item)
        return QVariant();

    switch (role){
        case Qt::DisplayRole:
        {
            if (index.column() == 0)
                return item->title;
            else
                return (item->argument == SearchBlacklist::NAME? tr("Filename") : tr("TTH"));

            break;
        }
        default:
            break;
    }

    return QVariant();
}


QVariant SearchBlackListModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
            case 0: return tr("Key");
            case 1: return tr("Type");
        }
    }

    return QVariant();
}

QVariant SearchBlackListItem::data(int column) const {
    if (column == COLUMN_SBL_KEY && childItems.size() > 0 && parentItem != 0)
        return childItems.size()+1;

    switch(column){
        case COLUMN_SBL_TYPE:
            return QVariant (argument);
        default:
            return QVariant (title);
    }
}

namespace {
    template <Qt::SortOrder order>
    struct Compare {
        void static sort(int col, QList<SearchBlackListItem*>& items) {
            qStableSort(items.begin(), items.end(), getAttrComp(col));
        }

        void static insertSorted(int col, QList<SearchBlackListItem*>& items, SearchBlackListItem* item) {
            QList<SearchBlackListItem*>::iterator it = qLowerBound(items.begin(), items.end(), item, getAttrComp(col));
            items.insert(it, item);
        }

        private:
            typedef bool (*AttrComp)(const SearchBlackListItem * l, const SearchBlackListItem * r);
            AttrComp static getAttrComp(int column) {
                switch (column){
                    case COLUMN_SBL_TYPE:
                        return AttrCmp<COLUMN_SBL_TYPE>;
                    default:
                        return AttrCmp<COLUMN_SBL_KEY>;
                }
            }
            template <int i>
            bool static AttrCmp(const SearchBlackListItem * l, const SearchBlackListItem * r) {
                return Cmp(QString::localeAwareCompare(l->data(i).toString(), r->data(i).toString()), 0);
            }
            template <typename T, T (SearchBlackListItem::*attr)>
            bool static AttrCmp(const SearchBlackListItem * l, const SearchBlackListItem * r) {
                return Cmp(l->*attr, r->*attr);
            }
            template <int i>
            bool static NumCmp(const SearchBlackListItem * l, const SearchBlackListItem * r) {
                return Cmp(l->data(i).toULongLong(), r->data(i).toULongLong());
            }
            template <typename T>
            bool static Cmp(const T& l, const T& r);
    };

    template <> template <typename T>
    bool inline Compare<Qt::AscendingOrder>::Cmp(const T& l, const T& r) {
        return l < r;
    }

    template <> template <typename T>
    bool inline Compare<Qt::DescendingOrder>::Cmp(const T& l, const T& r) {
        return l > r;
    }
} //namespace

void SearchBlackListModel::sort(int column, Qt::SortOrder order) {
    static int c = 0;

    if (column < 0)
        column = c;

    emit layoutAboutToBeChanged();

    if (order == Qt::AscendingOrder)
        Compare<Qt::AscendingOrder>().sort(column, rootItem->childItems);
    else if (order == Qt::DescendingOrder)
        Compare<Qt::DescendingOrder>().sort(column, rootItem->childItems);

    c = column;

    emit layoutChanged();
}

QModelIndex SearchBlackListModel::index(int row, int column, const QModelIndex &) const {
    if (row > (rootItem->childCount() - 1) || row < 0)
        return QModelIndex();

    return createIndex(row, column, rootItem->child(row));
}

QModelIndex SearchBlackListModel::parent(const QModelIndex & ) const {
    return QModelIndex();
}

QModelIndex SearchBlackListModel::addEmptyItem(){
    SearchBlackListItem *item = new SearchBlackListItem(rootItem);
    item->title = tr("Set text...");

    rootItem->appendChild(item);

    emit layoutChanged();

    return createIndex(item->row(), 0, item);
}

int SearchBlackListModel::getSortColumn() const{
    return sortColumn;
}

void SearchBlackListModel::setSortColumn(int sc){
    sortColumn = sc;
}

SearchBlackListItem::SearchBlackListItem(SearchBlackListItem *parent) : parentItem(parent), argument(0)
{
}

SearchBlackListItem::~SearchBlackListItem()
{
    qDeleteAll(childItems);
}

void SearchBlackListItem::appendChild(SearchBlackListItem *item) {
    item->parentItem = this;
    childItems.append(item);
}

SearchBlackListItem *SearchBlackListItem::child(int row) {
    return childItems.value(row);
}

int SearchBlackListItem::childCount() const {
    return childItems.count();
}

int SearchBlackListItem::columnCount() const {
    return itemData.count();
}
SearchBlackListItem *SearchBlackListItem::parent() {
    return parentItem;
}

int SearchBlackListItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<SearchBlackListItem*>(this));

    return 0;
}

SearchBlackListDelegate::SearchBlackListDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{
}

SearchBlackListDelegate::~SearchBlackListDelegate(){
}

QWidget *SearchBlackListDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.column() == 1){
        QComboBox *edit = new QComboBox(parent);

        edit->addItem(tr("Filename"));
        edit->addItem(tr("TTH"));

        return edit;
    }
    else{
        SearchBlackListItem *item = reinterpret_cast<SearchBlackListItem*>(index.internalPointer());
        QLineEdit *edit = new QLineEdit(parent);
        edit->setText(item->title);

        return edit;
    }
}

void SearchBlackListDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    editor->setGeometry(option.rect);
}

void SearchBlackListDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    SearchBlackListItem *item = reinterpret_cast<SearchBlackListItem*>(index.internalPointer());

    if (!item)
        return;

    if (index.column() == 1){
        QComboBox *edit = qobject_cast<QComboBox*>(editor);

        if (!edit)
            return;

        edit->setCurrentIndex(item->argument);
    }
    else{
        QLineEdit *edit = qobject_cast<QLineEdit*>(editor);

        if (!edit)
            return;

        edit->setText(item->title);
    }
}

void SearchBlackListDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{
    SearchBlackListModel *m = qobject_cast<SearchBlackListModel* >(model);
    SearchBlackListItem *item = reinterpret_cast<SearchBlackListItem* >(index.internalPointer());

    if (!m)
        return;

    if (index.column() == 1){
        QComboBox *edit = qobject_cast<QComboBox*>(editor);

        if (!edit || !item)
            return;

        item->argument = edit->currentIndex();
    }
    else{
        QLineEdit *edit = qobject_cast<QLineEdit*>(editor);

        if (!edit || !item)
            return;

        item->title = edit->text();
    }
}
