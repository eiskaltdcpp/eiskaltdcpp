/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ScriptManagerDialog.h"
#include "WulforSettings.h"
#include "WulforUtil.h"

#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTextStream>
#include <QtDebug>
#include <QApplication>

#include "dcpp/stdinc.h"
#include "dcpp/Util.h"

#ifndef CLIENT_SCRIPTS_DIR
#define CLIENT_SCRIPTS_DIR ""
#endif

ScriptManagerDialog::ScriptManagerDialog(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    model = new ScriptManagerModel(NULL);
    connect(this, SIGNAL(accepted()), model, SLOT(save()));
    connect(comboBox, SIGNAL(activated(int)), this, SLOT(slotSetChangedAction(int)));

    treeView->setModel(model);

    comboBox->setCurrentIndex(WIGET("scriptmanager/script-changed-action", 0));

    setWindowTitle(tr("Script Manager"));
}

ScriptManagerDialog::~ScriptManagerDialog(){
    delete model;
}

void ScriptManagerDialog::slotSetChangedAction(int index){
    WISET("scriptmanager/script-changed-action", index);
}

ScriptManagerModel::ScriptManagerModel(QObject * parent) : QAbstractItemModel(parent) {
    rootItem = new ScriptManagerItem(NULL);

    load();
}


ScriptManagerModel::~ScriptManagerModel() {
    delete rootItem;
}


int ScriptManagerModel::rowCount(const QModelIndex & ) const {
    return rootItem->childCount();
}

int ScriptManagerModel::columnCount(const QModelIndex & ) const {
    return 1;
}

Qt::ItemFlags ScriptManagerModel::flags(const QModelIndex &index) const{
    Qt::ItemFlags f = QAbstractItemModel::flags(index);

    f |= Qt::ItemIsUserCheckable;

    return f;
}

QVariant ScriptManagerModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    ScriptManagerItem * item = static_cast<ScriptManagerItem*>(index.internalPointer());

    if (!item)
        return QVariant();

    switch (role){
        case Qt::DisplayRole:
        {
            switch (index.column()) {
                case 0: return item->name;
            }

            break;
        }
        case Qt::TextAlignmentRole:
        {
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
        case Qt::CheckStateRole:
        {
            return static_cast<int>(item->isOn? Qt::Checked : Qt::Unchecked);
        }
        case Qt::ToolTipRole:
        {
            return item->desc;
        }
        case Qt::DecorationRole:
        {
            return item->icon;
        }
        case Qt::FontRole:
        {
            QFont f;
            f.setBold(true);

            return f;
        }
    }

    return QVariant();
}

bool ScriptManagerModel::setData(const QModelIndex &index, const QVariant &value, int role){
    if (index.isValid() && !index.column() && role == Qt::CheckStateRole){
        ScriptManagerItem *item = reinterpret_cast<ScriptManagerItem*>(index.internalPointer());

        item->isOn = (value.toInt() == Qt::Checked)? true : false;

        return true;
    }

    return QAbstractItemModel::setData(index, value, role);
}

QVariant ScriptManagerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);

    return QVariant();
}

QModelIndex ScriptManagerModel::index(int row, int column, const QModelIndex &) const {
    if (row > (rootItem->childCount() - 1) || row < 0)
        return QModelIndex();

    return createIndex(row, column, rootItem->child(row));
}

QModelIndex ScriptManagerModel::parent(const QModelIndex & ) const {
    return QModelIndex();
}

void ScriptManagerModel::load(){
    enabled = QString(QByteArray::fromBase64(WSGET(WS_APP_ENABLED_SCRIPTS).toLatin1())).split("\n");

#if !defined(Q_WS_WIN)
    QDir dir(CLIENT_SCRIPTS_DIR);
#else
    QDir dir(qApp->applicationDirPath()+QDir::separator()+CLIENT_SCRIPTS_DIR);
#endif
    if (dir.exists()){
        for (const auto &d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            loadDir(QString(CLIENT_SCRIPTS_DIR)+QDir::separator()+d);
    }

    dir = QDir(_q(dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG)+"scripts"));

    if (dir.exists()){
        for (const auto &d : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
            loadDir(_q(dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG))+"scripts"+QDir::separator()+d);
    }
}

void ScriptManagerModel::loadDir(const QString &path){
    QFile script_desc(path+"/script.desc");

    if (!script_desc.exists())
        return;

    if (!script_desc.open(QIODevice::ReadOnly))
        return;

    QTextStream stream(&script_desc);
    ScriptManagerItem *item = new ScriptManagerItem(rootItem);

    while (!stream.atEnd()){
        QString line = stream.readLine();

        if (line.startsWith("auth=", Qt::CaseInsensitive)){
            item->auth = line.remove("auth=", Qt::CaseInsensitive).trimmed();
            item->auth.replace("\n", "");
        }
        else if (line.startsWith("name=", Qt::CaseInsensitive)){
            item->name = line.remove("name=", Qt::CaseInsensitive).trimmed();
            item->name.replace("\n", "");
        }
        else if (line.startsWith("desc=", Qt::CaseInsensitive)){
            item->desc = line.remove("desc=", Qt::CaseInsensitive).trimmed();
            item->desc.replace("\n", "");
        }
        else if (line.startsWith("icon=", Qt::CaseInsensitive)){
            QString icon = line.remove("icon=", Qt::CaseInsensitive).trimmed();
            icon.replace("\n", "");

            item->icon = QIcon(path + QDir::separator() + icon);
        }
        else if (line.startsWith("mainfile=", Qt::CaseInsensitive)){
            item->path = line.remove("mainfile=", Qt::CaseInsensitive);
            item->path.replace("\n", "");
        }
    }

    script_desc.close();

    item->path = (item->path.isEmpty()? (path + QDir::separator() + "main.js") : path + QDir::separator() + item->path);

    if (item->name.isEmpty() || !QFile(item->path).exists()){
        delete item;

        return;
    }

    if (enabled.contains(item->path))
        item->isOn = true;

    rootItem->appendChild(item);
}

void ScriptManagerModel::save(){
    QString all = "";
    for (const auto &i : rootItem->childItems){
        if (i->isOn)
            all += i->path + "\n";
    }

    WSSET(WS_APP_ENABLED_SCRIPTS, all.toLatin1().toBase64());
}

ScriptManagerItem::ScriptManagerItem(ScriptManagerItem *parent) :
    isOn(false), parentItem(parent)
{
}

ScriptManagerItem::~ScriptManagerItem()
{
    qDeleteAll(childItems);
}

void ScriptManagerItem::appendChild(ScriptManagerItem *item) {
    item->parentItem = this;
    childItems.append(item);
}

ScriptManagerItem *ScriptManagerItem::child(int row) {
    return childItems.value(row);
}

int ScriptManagerItem::childCount() const {
    return childItems.count();
}

int ScriptManagerItem::columnCount() const {
    return 1;
}
ScriptManagerItem *ScriptManagerItem::parent() {
    return parentItem;
}

int ScriptManagerItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ScriptManagerItem*>(this));

    return 0;
}
