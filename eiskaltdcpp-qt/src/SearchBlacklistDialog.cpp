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

SearchBlackListDialog::SearchBlackListDialog(QWidget *parent): QDialog(parent){
    setupUi(this);

    model = new SearchBlackListModel();
    treeView_RULES->setModel(model);
    treeView_RULES->setItemDelegate(new SearchBlackListDelegate(this));
    treeView_RULES->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(treeView_RULES, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
    connect(this, SIGNAL(accepted()), this, SLOT(ok()));
}

SearchBlackListDialog::~SearchBlackListDialog(){
    model->deleteLater();
}

void SearchBlackListDialog::ok(){
    model->save();
}

void SearchBlackListDialog::slotContextMenu(){
    QItemSelectionModel *s_m = treeView_RULES->selectionModel();
    QModelIndexList indexes = s_m->selectedRows(0);

    QMenu *menu = new QMenu(this);
    QAction *add = new QAction(WICON(WulforUtil::eiEDITADD), tr("Add new"), NULL);
    QAction *rem = new QAction(WICON(WulforUtil::eiEDITDELETE), tr("Remove"), NULL);

    menu->addActions(QList<QAction*>() << add << rem);

    QAction *ret = menu->exec(QCursor::pos());

    menu->deleteLater();

    if (ret == add){
        s_m->select(model->addEmptyItem(), QItemSelectionModel::SelectCurrent|QItemSelectionModel::Rows);
    }
    else if (ret && !indexes.isEmpty()){
        foreach (QModelIndex index, indexes){
            SearchBlackListItem *i = reinterpret_cast<SearchBlackListItem*>(index.internalPointer());

            if (!i)
                continue;

            i->parent()->childItems.removeAt(i->row());

            delete i;
        }

        model->repaint();
    }
}

SearchBlackListModel::SearchBlackListModel(QObject * parent) : QAbstractItemModel(parent) {
    rootItem = new SearchBlackListItem(NULL);

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

void SearchBlackListModel::sort(int column, Qt::SortOrder order) {
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
    return 7;
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

    if (!(m && item))
        return;

    if (index.column() == 1){
        QComboBox *edit = qobject_cast<QComboBox*>(editor);

        if (!edit)
            return;

        item->argument = edit->currentIndex();
    }
    else{
        QLineEdit *edit = qobject_cast<QLineEdit*>(editor);

        if (!edit)
            return;

        item->title = edit->text();
    }
}
