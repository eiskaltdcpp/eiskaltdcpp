/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "UCModel.h"
#include "MainWindow.h"
#include "WulforUtil.h"

#include <QRegExp>

#include "dcpp/NmdcHub.h"

using namespace dcpp;

UCModel::UCModel(QObject *parent): QAbstractItemModel(parent)
{
    rootItem = new UCItem(NULL);
}

UCModel::~UCModel()
{
    delete rootItem;
}

int UCModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant UCModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.column() > columnCount(QModelIndex()))
        return QVariant();

    UCItem *item = static_cast<UCItem*>(index.internalPointer());

    switch(role) {
        case Qt::DecorationRole: // icon
            break;
        case Qt::DisplayRole:
        {
            switch (index.column()){
                case 0: return item->name;
                case 1: return item->comm;
                case 2: return item->hub;
                default: break;
            }
        }
        case Qt::TextAlignmentRole:
            break;
        case Qt::ForegroundRole:
            break;
        case Qt::ToolTipRole:
            break;
    }

    return QVariant();
}

Qt::ItemFlags UCModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return flags;
}

QVariant UCModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){
        switch (section){
            case 0: return tr("Name");
            case 1: return tr("Command");
            case 2: return tr("Hub");
            default: break;
        }
    }

    return QVariant();
}

QModelIndex UCModel::index(int row, int column, const QModelIndex &parent)
            const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    UCItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<UCItem*>(parent.internalPointer());

    UCItem *childItem = parentItem->child(row);

    if (childItem && rootItem->childItems.contains(childItem))
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex UCModel::parent(const QModelIndex &index) const
{
    return QModelIndex();
}

int UCModel::rowCount(const QModelIndex &parent) const
{
    UCItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<UCItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void UCModel::sort(int column, Qt::SortOrder order) {
    Q_UNUSED(column);
    Q_UNUSED(order);
    //Model without sorting
}

void UCModel::loadUC(){
#warning
    /*UserCommand::List lst = FavoriteManager::getInstance()->getUserCommands();
    for(UserCommand::List::const_iterator i = lst.begin(); i != lst.end(); ++i) {
        const UserCommand& uc = *i;
        if(!uc.isSet(UserCommand::FLAG_NOSAVE))
            addUC(uc);
    }*/
}

void UCModel::addUC(const dcpp::UserCommand &uc){
#warning
//    UCItem *item = new UCItem(rootItem);

//    item->name = ((uc.getType() == dcpp::UserCommand::TYPE_SEPARATOR)? tr("Separator") : _q(uc.getName()));
//    item->comm = _q(uc.getCommand());
//    item->hub  = _q(uc.getHub());
//    item->id   = uc.getId();
//    item->type = uc.getType();
//    item->ctx  = uc.getCtx();

//    beginInsertRows(QModelIndex(), rootItem->childCount(), rootItem->childCount());
//    rootItem->appendChild(item);
//    endInsertRows();
}

void UCModel::newUC(){
    /*UCDialog ucd(MainWindow::getInstance());

    if (ucd.exec() == QDialog::Accepted){
        addUC(FavoriteManager::getInstance()->addUserCommand(ucd.getType(),
                                                             ucd.getCtx(),
                                                             0,
                                                             _tq(ucd.getName()),
                                                             _tq(ucd.getCmd()),
                                                             _tq(ucd.getHub())
                                                             ));
    }*/
}

void UCModel::changeUC(const QModelIndex &i){
    if (!i.isValid())
        return;

    UCItem *item = reinterpret_cast<UCItem*>(i.internalPointer());

    if (!rootItem->childItems.contains(item))
        return;

    UCDialog ucd(MainWindow::getInstance());

    initDlgFromItem(ucd, *item);

    if (ucd.exec() == QDialog::Accepted){
        UserCommand uc;
        FavoriteManager::getInstance()->getUserCommand(item->id, uc);

        uc.setName(_tq(ucd.getName()));
        uc.setCommand(_tq(ucd.getCmd()));
        uc.setHub(_tq(ucd.getHub()));
        uc.setType(ucd.getType());
        uc.setCtx(ucd.getCtx());
        FavoriteManager::getInstance()->updateUserCommand(uc);

        item->name = ((uc.getType() == dcpp::UserCommand::TYPE_SEPARATOR)? tr("Separator") : _q(uc.getName()));
        item->comm = _q(uc.getCommand());
        item->hub  = _q(uc.getHub());
        item->id   = uc.getId();
        item->type = uc.getType();
        item->ctx  = uc.getCtx();

        emit layoutChanged();
    }
}

void UCModel::remUC(const QModelIndex &i){
    if (!i.isValid())
        return;

    UCItem *item = reinterpret_cast<UCItem*>(i.internalPointer());

    if (!rootItem->childItems.contains(item))
        return;
    
    FavoriteManager::getInstance()->removeUserCommand(item->id);

    beginRemoveRows(QModelIndex(), item->row(), item->row());
    rootItem->childItems.removeAt(item->row());
    delete item;
    endRemoveRows();
}

void UCModel::moveUp(const QModelIndex &i){
    if (!i.isValid())
        return;

    UCItem *item = reinterpret_cast<UCItem*>(i.internalPointer());

    if (!rootItem->childItems.contains(item) || item->row() == 0)
        return;

    emit layoutAboutToBeChanged();
    int r = item->row();
    rootItem->childItems.removeAt(r);
    rootItem->childItems.insert(r-1, item);
    emit layoutChanged();

    FavoriteManager::getInstance()->moveUserCommand(item->id, -1);

    emit selectIndex(index(item->row(), 0, QModelIndex()));
}

void UCModel::moveDown(const QModelIndex &i){
    if (!i.isValid())
        return;

    UCItem *item = reinterpret_cast<UCItem*>(i.internalPointer());

    if (!rootItem->childItems.contains(item) || item->row() == rootItem->childCount()-1)
        return;

    emit layoutAboutToBeChanged();
    int r = item->row();
    rootItem->childItems.removeAt(r);
    rootItem->childItems.insert(r+1, item);
    emit layoutChanged();

    FavoriteManager::getInstance()->moveUserCommand(item->id, 1);

    emit selectIndex(index(item->row(), 0, QModelIndex()));
}

void UCModel::initDlgFromItem(UCDialog &dlg, const UCItem &item){
    /*unsigned long ctx   = item.ctx;
    unsigned long type  = item.type;
    QString name        = item.name;
    QString comm        = item.comm;
    QString hub         = item.hub;

    if (type == 0){
        dlg.radioButton_SEP->toggle();
        dlg.lineEdit_CMD->clear();
    }
    else if (comm.startsWith("<%[myNI]> ")){
        dlg.radioButton_CHAT->toggle();

        int from = QString("<%[myNI]> ").length();
        QString cmd = comm.mid(from, comm.length()-from-1);

        cmd = _q(NmdcHub::validateMessage(_tq(cmd), true));

        dlg.lineEdit_CMD->setText(cmd);
    }
    else if ( comm.startsWith("$To: ") && comm.indexOf(" From: %[myNI] $<%[myNI]> ") > 0){
        QRegExp reg_exp("^\\$To: ([^\t]+) From: %\\[myNI\\] \\$<%\\[myNI\\]> ([^\t^|]+)");
        (void)reg_exp.indexIn(comm);
        QStringList list = reg_exp.capturedTexts();

        if (list.size() != 3)
            return;

        dlg.radioButton_PM->toggle();
        dlg.lineEdit_CMD->setText(list.at(2));
        dlg.lineEdit_TO->setText(list.at(1));
    }
    else{
        dlg.radioButton_RAW->toggle();
        dlg.lineEdit_CMD->setText(comm);
    }

    if(type == UserCommand::TYPE_RAW_ONCE)
        dlg.checkBox_SENDONCE->setChecked(true);

    dlg.lineEdit_HUB->setText(hub);
    dlg.lineEdit_NAME->setText(name);

    dlg.checkBox_FB->setChecked(ctx & UserCommand::CONTEXT_FILELIST);
    dlg.checkBox_HUB->setChecked(ctx & UserCommand::CONTEXT_HUB);
    dlg.checkBox_USER->setChecked(ctx & UserCommand::CONTEXT_CHAT);
    dlg.checkBox_SEARCH->setChecked(ctx & UserCommand::CONTEXT_SEARCH);*/
}

UCItem::UCItem(UCItem *parent) :
    parentItem(parent), ctx(0), type(0), id(0)
{
}

UCItem::~UCItem()
{
    if (childItems.size() > 0)
        qDeleteAll(childItems);
}

void UCItem::appendChild(UCItem *item) {
    childItems.append(item);
}

UCItem *UCItem::child(int row) {
    return childItems.value(row);
}

int UCItem::childCount() const {
    return childItems.count();
}

int UCItem::columnCount() const {
    return 3;
}

UCItem *UCItem::parent() {
    return parentItem;
}

int UCItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<UCItem*>(this));

    return 0;
}

UCDialog::UCDialog(QWidget *parent): QDialog(parent){
    setupUi(this);

    connect(lineEdit_CMD,     SIGNAL(textChanged(QString)), this, SLOT(updateLines()));
    connect(lineEdit_TO,      SIGNAL(textChanged(QString)), this, SLOT(updateLines()));
    connect(radioButton_CHAT, SIGNAL(toggled(bool)),        this, SLOT(updateLines()));
    connect(radioButton_PM,   SIGNAL(toggled(bool)),        this, SLOT(updateLines()));
    connect(radioButton_RAW,  SIGNAL(toggled(bool)),        this, SLOT(updateLines()));
    connect(radioButton_SEP,  SIGNAL(toggled(bool)),        this, SLOT(updateLines()));
}

unsigned long UCDialog::getCtx() const {
    unsigned long ctx = 0;

//    if (checkBox_HUB->isChecked())
//        ctx |= UserCommand::CONTEXT_HUB;
//    if (checkBox_SEARCH->isChecked())
//        ctx |= UserCommand::CONTEXT_SEARCH;
//    if (checkBox_USER->isChecked())
//        ctx |= UserCommand::CONTEXT_CHAT;
//    if (checkBox_FB->isChecked())
//        ctx |= UserCommand::CONTEXT_FILELIST;

    return ctx;
}

unsigned long UCDialog::getType() const {
    int _type = -1;

    if (radioButton_SEP->isChecked())
        _type = 0;
    else if (radioButton_CHAT->isChecked())
        _type = 2;
    else if (radioButton_PM->isChecked())
        _type = 3;
    else
        _type = 1;

    if (_type != 0)
        _type = checkBox_SENDONCE->isChecked()? 2 : 1;

    return _type;
}

QString UCDialog::getCmd() const {
    return lineEdit_RESULT->text();
}

QString UCDialog::getName() const {
    return lineEdit_NAME->text();
}

QString UCDialog::getHub() const {
    return lineEdit_HUB->text();
}

void UCDialog::updateLines(){
    unsigned long type = getType();
    QString cmd = "";

    if(type == 0)
        cmd.clear();
    else if(type == 1)
        cmd = lineEdit_CMD->text();
    else if(type == 2)
        cmd = "<%[myNI]> " + _q(NmdcHub::validateMessage(Text::fromT(_tq(lineEdit_CMD->text())), false)) + "|";
    else if(type == 3)
        cmd = "$To: " + lineEdit_TO->text() + " From: %[myNI] $<%[myNI]> " + _q(NmdcHub::validateMessage(_tq(lineEdit_CMD->text()), false)) + "|";

    lineEdit_RESULT->setText(cmd);
}

