/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "CustomFontModel.h"
#include "WulforUtil.h"

#include <QtGui>
#include <QFontDialog>
#include <QList>
#include <QStringList>
#include <QColor>

#include "dcpp/ShareManager.h"

using namespace dcpp;

#include <set>

CustomFontModel::CustomFontModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << tr("Description") << tr("Font");

    rootItem = new CustomFontItem(rootData, NULL);

    addNewFont(WS_APP_FONT,         tr("Application"));
    addNewFont(WS_CHAT_FONT,        tr("Public Chat: Chat"));
    addNewFont(WS_CHAT_ULIST_FONT,  tr("Public Chat: Userlist"));
    addNewFont(WS_CHAT_PM_FONT,     tr("Private Chat"));

    connect(this, SIGNAL(fontChanged(QString,QString)), WulforSettings::getInstance(), SIGNAL(fontChanged(QString,QString)));
}

CustomFontModel::~CustomFontModel()
{
    if (rootItem)
        delete rootItem;
}

int CustomFontModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<CustomFontItem*>(parent.internalPointer())->columnCount();
    else
        return rootItem->columnCount();
}

QVariant CustomFontModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    CustomFontItem *item = static_cast<CustomFontItem*>(index.internalPointer());

    switch(role) {
        case Qt::DisplayRole:
        {
            return item->data(index.column());
        }
        case Qt::FontRole:
        {
            QFont f;

            if (!item->custom_font.isEmpty())
                f.fromString(item->custom_font);
            else
                f.fromString(item->data(COLUMN_CUSTOM_FONT_DISP).toString());

            return f;
        }
        case Qt::TextAlignmentRole:
        case Qt::ForegroundRole:
        case Qt::BackgroundColorRole:
        case Qt::ToolTipRole:
        case Qt::DecorationRole:
        default:
            break;
    }

    return QVariant();
}

QVariant CustomFontModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    QList<QVariant> rootData;
    rootData << tr("Description") << tr("Font");

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootData.at(section);

    return QVariant();
}

QModelIndex CustomFontModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    CustomFontItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<CustomFontItem*>(parent.internalPointer());

    CustomFontItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex CustomFontModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    CustomFontItem *childItem = static_cast<CustomFontItem*>(index.internalPointer());
    CustomFontItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int CustomFontModel::rowCount(const QModelIndex &parent) const
{
    CustomFontItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<CustomFontItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void CustomFontModel::addNewFont(const QString &wkey, const QString &desc){
    if (wkey.isEmpty() || desc.isEmpty())
        return;

    QString font_desc = WSGET(wkey.toAscii().constData());
    QFont f;

    if (font_desc.isEmpty())
        f = qApp->font();
    else
        f = f.fromString(font_desc)? f : qApp->font();

    CustomFontItem *item = new CustomFontItem(QList<QVariant>() << desc << f.toString(), rootItem);
    item->key = wkey;
    item->custom_font = "";

    rootItem->appendChild(item);

    emit layoutChanged();
}

void CustomFontModel::itemDoubleClicked(const QModelIndex &i){
    if (!(i.isValid() && i.internalPointer()))
        return;

    CustomFontItem *item = reinterpret_cast<CustomFontItem*>(i.internalPointer());

    bool ok = false;

    QFont f = QFontDialog::getFont(&ok);

    if (ok){
        item->custom_font = f.toString();
        item->updateColumn(COLUMN_CUSTOM_FONT_DISP, item->custom_font);

        emit layoutChanged();
    }
}

void CustomFontModel::ok(){
    foreach (CustomFontItem *i, rootItem->childItems){
        if (!i->custom_font.isEmpty()){
            WSSET(i->key.toAscii().constData(), i->custom_font);

            emit fontChanged(i->key, i->custom_font);
        }
    }
}

CustomFontItem::CustomFontItem(const QList<QVariant> &data, CustomFontItem *parent) :
    itemData(data), parentItem(parent)
{
}

CustomFontItem::~CustomFontItem()
{
    qDeleteAll(childItems);
}

void CustomFontItem::appendChild(CustomFontItem *item) {
    childItems.append(item);
}

CustomFontItem *CustomFontItem::child(int row) {
    return childItems.value(row);
}

int CustomFontItem::childCount() const {
    return childItems.count();
}

int CustomFontItem::columnCount() const {
    return itemData.count();
}

QVariant CustomFontItem::data(int column) const {
    return itemData.value(column);
}

CustomFontItem *CustomFontItem::parent() {
    return parentItem;
}

int CustomFontItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<CustomFontItem*>(this));

    return 0;
}

void CustomFontItem::updateColumn(int column, QVariant var){
    if (column > (itemData.size()-1))
        return;

    itemData[column] = var;
}
