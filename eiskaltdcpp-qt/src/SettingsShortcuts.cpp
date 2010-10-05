#include "SettingsShortcuts.h"
#include "ShortcutManager.h"
#include "MainWindow.h"
#include "CustomSetting.h"

#include <QMap>
#include <QKeySequence>
#include <QByteArray>
#include <QtDebug>

static const QString &TREEVIEW_STATE_KEY = "settings-shortcuts-tableview-state";

SettingsShortcuts::SettingsShortcuts(QWidget *parent) :
    QWidget(parent)
{
    eRegisterCustomSetting(StrSetting, TREEVIEW_STATE_KEY, "");

    setupUi(this);

    model = new ShortcutsModel(this);
    treeView->setModel(model);
    treeView->setItemDelegate(new ShortcutsDelegate(this));
    treeView->header()->restoreState(QByteArray::fromBase64(WSGET(TREEVIEW_STATE_KEY).toAscii()));
}

SettingsShortcuts::~SettingsShortcuts(){
    model->deleteLater();
}

void SettingsShortcuts::ok(){
    model->save();

    WSSET(TREEVIEW_STATE_KEY, treeView->header()->saveState().toBase64());
}

ShortcutsModel::ShortcutsModel(QObject * parent) : QAbstractItemModel(parent) {
    rootItem = new ShortcutItem(NULL);

    MainWindow *MW = MainWindow::getInstance();
    QMap<QString, QKeySequence> shs;
    shs = ShortcutManager::getInstance()->getShortcuts();

    QMap<QString, QKeySequence>::iterator it = shs.begin();
    const QAction *act = NULL;

    for (; it != shs.end(); ++it){
        act = MW->findChild<QAction* >(it.key());

        if (!act)
            continue;

        ShortcutItem *item = new ShortcutItem(rootItem);
        item->title = act->text();
        item->shortcut = it.value().toString();

        rootItem->appendChild(item);

        items.insert(item, it.key());
    }

    emit layoutChanged();
}


ShortcutsModel::~ShortcutsModel() {
    delete rootItem;
}

void ShortcutsModel::save(){
    QHash<ShortcutItem*, QString>::iterator it = items.begin();
    MainWindow *MW = MainWindow::getInstance();
    QAction *act = NULL;

    for (; it != items.end(); ++it){
        act = MW->findChild<QAction* >(it.value());

        if (!act)
            continue;

        ShortcutManager::getInstance()->updateShortcut(act, ((it.key())->shortcut));
    }

    ShortcutManager::getInstance()->save();
}

int ShortcutsModel::rowCount(const QModelIndex & ) const {
    return rootItem->childCount();
}

int ShortcutsModel::columnCount(const QModelIndex & ) const {
    return 2;
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ShortcutsModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return QVariant();

    ShortcutItem * item = static_cast<ShortcutItem*>(index.internalPointer());

    if (!item)
        return QVariant();

    switch (role){
        case Qt::DisplayRole:
        {
            switch (index.column()) {
                case 0: return item->title;
                case 1: return item->shortcut;
            }

            break;
        }
        case Qt::DecorationRole:
        {
            break;
        }
        case Qt::ToolTipRole:
        {
            break;
        }
        case Qt::TextAlignmentRole:
        {
            break;
        }
        case Qt::FontRole:
        {
            break;
        }
    }

    return QVariant();
}


QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
            case 0: return tr("Action");
            case 1: return tr("Hotkey");
        }
    }

    return QVariant();
}

void ShortcutsModel::sort(int column, Qt::SortOrder order) {
    emit layoutChanged();
}

QModelIndex ShortcutsModel::index(int row, int column, const QModelIndex &) const {
    if (row > (rootItem->childCount() - 1) || row < 0)
        return QModelIndex();

    return createIndex(row, column, rootItem->child(row));
}

QModelIndex ShortcutsModel::parent(const QModelIndex & ) const {
    return QModelIndex();
}

ShortcutItem::ShortcutItem(ShortcutItem *parent) : parentItem(parent)
{
}

ShortcutItem::~ShortcutItem()
{
    qDeleteAll(childItems);
}

void ShortcutItem::appendChild(ShortcutItem *item) {
    item->parentItem = this;
    childItems.append(item);
}

ShortcutItem *ShortcutItem::child(int row) {
    return childItems.value(row);
}

int ShortcutItem::childCount() const {
    return childItems.count();
}

int ShortcutItem::columnCount() const {
    return 7;
}
ShortcutItem *ShortcutItem::parent() {
    return parentItem;
}

int ShortcutItem::row() const {
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<ShortcutItem*>(this));

    return 0;
}

ShortcutsDelegate::ShortcutsDelegate(QObject *parent):
        QStyledItemDelegate(parent)
{
}

ShortcutsDelegate::~ShortcutsDelegate(){
}

QWidget *ShortcutsDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    if (index.column() == 1){
        ShortcutEdit *edit = new ShortcutEdit(parent);

        connect(edit, SIGNAL(clearEdit()), edit, SLOT(clear()));

        return edit;
    }
    else
        return NULL;
}

void ShortcutsDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const{
    editor->setGeometry(option.rect);
}

void ShortcutsDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const{
    ShortcutEdit *edit = qobject_cast<ShortcutEdit* >(editor);

    if (edit && index.isValid())
        edit->setText(index.data().toString());
}

void ShortcutsDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const{
    ShortcutEdit *edit = qobject_cast<ShortcutEdit* >(editor);
    ShortcutsModel *m = qobject_cast<ShortcutsModel* >(model);

    if (edit && index.isValid() && m){
        ShortcutItem *item = reinterpret_cast<ShortcutItem* >(index.internalPointer());
        item->shortcut = edit->text();

        m->repaint();
    }
}
