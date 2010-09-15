#include "SettingsSharing.h"
#include "HashProgress.h"
#include "WulforUtil.h"
#include "WulforSettings.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ShareManager.h"

#include <QListWidgetItem>
#include <QTreeWidget>
#include <QList>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QInputDialog>
#include <QMessageBox>
#include <QStack>

using namespace dcpp;

SettingsSharing::SettingsSharing(QWidget *parent):
        QWidget(parent)
{
    setupUi(this);

    model = NULL;

    init();
}

SettingsSharing::~SettingsSharing(){
    delete model;
}

void SettingsSharing::showEvent(QShowEvent *e){
    e->accept();

    if (WSGET(WS_SHAREHEADER_STATE).isEmpty()){
        for (int i = 0; i < model->columnCount(); i++)
            treeView->setColumnWidth(i, treeView->width()/4);
    }
}

void SettingsSharing::ok(){
    SettingsManager *SM = SettingsManager::getInstance();

    SM->set(SettingsManager::FOLLOW_LINKS, checkBox_FOLLOW->isChecked());
    SM->set(SettingsManager::SHARE_TEMP_FILES, checkBox_SHARE_TEMP_FILES->isChecked());
    SM->set(SettingsManager::MIN_UPLOAD_SPEED, spinBox_EXTRA->value());
    SM->set(SettingsManager::SLOTS_PRIMARY, spinBox_UPLOAD->value());
    SM->set(SettingsManager::MAX_HASH_SPEED, spinBox_MAXHASHSPEED->value());
    SM->set(SettingsManager::FAST_HASH, checkBox_FASTHASH->isChecked());
    SM->set(SettingsManager::AUTO_REFRESH_TIME, spinBox_REFRESH_TIME->value());
    SM->set(SettingsManager::ALLOW_UPDATE_FILELIST_ON_STARTUP, checkBox_REFRESH_ON_STARTUP->isChecked());
    SM->set(SettingsManager::HASH_BUFFER_NORESERVE, checkBox_MAPNORESERVE->isChecked());
    SM->set(SettingsManager::HASH_BUFFER_POPULATE, checkBox_MAPPOPULATE->isChecked());
    SM->set(SettingsManager::HASH_BUFFER_PRIVATE, checkBox_MAPPRIVATE->isChecked());
    SM->set(SettingsManager::HASH_BUFFER_SIZE_MB, comboBox_BUFSIZE->currentText().toInt());

    QStringList list;
    for (int k = 0; k < listWidget_SKIPLIST->count(); ++k)
        list << listWidget_SKIPLIST->item(k)->text();

    SM->set(SettingsManager::SKIPLIST_SHARE, _tq(list.join("|")));

    WBSET(WB_SIMPLE_SHARE_MODE, checkBox_SIMPLE_SHARE_MODE->isChecked());

    if (checkBox_SIMPLE_SHARE_MODE->isChecked())
        SM->save();

    WSSET(WS_SHAREHEADER_STATE, treeView->header()->saveState().toBase64());
    WBSET(WB_APP_REMOVE_NOT_EX_DIRS, checkBox_AUTOREMOVE->isChecked());

    QFile f(_q(Util::getPath(Util::PATH_USER_CONFIG) + "PerFolderLimit.conf"));

    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){

        QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard, 0);
        QTextStream stream(&f);

        foreach (QTreeWidgetItem *i, items)
            stream << (i->text(1).left(i->text(1).indexOf(" ")) + " ") << i->text(0) << '\n';

        stream.flush();

        f.close();
    }
}

void SettingsSharing::init(){
    WulforUtil *WU = WulforUtil::getInstance();

    toolButton_ADD->setIcon(WU->getPixmap(WulforUtil::eiBOOKMARK_ADD));
    toolButton_EDIT->setIcon(WU->getPixmap(WulforUtil::eiEDIT));
    toolButton_DELETE->setIcon(WU->getPixmap(WulforUtil::eiEDITDELETE));
    toolButton_BROWSE->setIcon(WU->getPixmap(WulforUtil::eiFOLDER_BLUE));

    toolButton_RECREATE->setIcon(WU->getPixmap(WulforUtil::eiRELOAD));

    checkBox_SHAREHIDDEN->setChecked(BOOLSETTING(SHARE_HIDDEN));
    checkBox_SHARE_TEMP_FILES->setChecked(BOOLSETTING(SHARE_TEMP_FILES));
    checkBox_FOLLOW->setChecked(BOOLSETTING(FOLLOW_LINKS));
    spinBox_UPLOAD->setValue(SETTING(SLOTS_PRIMARY));
    spinBox_MAXHASHSPEED->setValue(SETTING(MAX_HASH_SPEED));
    spinBox_EXTRA->setValue(SETTING(MIN_UPLOAD_SPEED));
    spinBox_REFRESH_TIME->setValue(SETTING(AUTO_REFRESH_TIME));
    checkBox_REFRESH_ON_STARTUP->setChecked(BOOLSETTING(ALLOW_UPDATE_FILELIST_ON_STARTUP));
    checkBox_AUTOREMOVE->setChecked(WBGET(WB_APP_REMOVE_NOT_EX_DIRS));

    checkBox_FASTHASH->setChecked(BOOLSETTING(FAST_HASH));
    groupBox_FASTHASH->setEnabled(BOOLSETTING(FAST_HASH));

    listWidget_SKIPLIST->addItems(_q(SETTING(SKIPLIST_SHARE)).split('|', QString::SkipEmptyParts));

    label_TOTALSHARED->setText(tr("Total shared: %1")
                               .arg(WulforUtil::formatBytes(ShareManager::getInstance()->getShareSize())));

    checkBox_SIMPLE_SHARE_MODE->setChecked(WBGET(WB_SIMPLE_SHARE_MODE));
    treeWidget_SIMPLE_MODE->setVisible(WBGET(WB_SIMPLE_SHARE_MODE));
    treeView->setHidden(WBGET(WB_SIMPLE_SHARE_MODE));

    treeWidget_SIMPLE_MODE->setContextMenuPolicy(Qt::CustomContextMenu);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    checkBox_MAPNORESERVE->setChecked(SETTING(HASH_BUFFER_NORESERVE));
    checkBox_MAPPOPULATE->setChecked(SETTING(HASH_BUFFER_POPULATE));
    checkBox_MAPPRIVATE->setChecked(SETTING(HASH_BUFFER_PRIVATE));

    int ind = comboBox_BUFSIZE->findText(QString().setNum(SETTING(HASH_BUFFER_SIZE_MB)));
    if (ind >= 0)
        comboBox_BUFSIZE->setCurrentIndex(ind);

    QFile f(_q(Util::getPath(Util::PATH_USER_CONFIG) + "PerFolderLimit.conf"));

    if (f.open(QIODevice::ReadOnly | QIODevice::Text)){
        QTextStream stream(&f);
        QString line = "";
        QMap<QString, unsigned> restrict_map;

        while (!(line = stream.readLine(0)).isNull()){
            QStringList list = line.split(' ');

            if (list.size() < 2)
                continue;

            bool ok = false;
            unsigned size = 0;

            size = list.at(0).toUInt(&ok);

            if (!ok)
                continue;

            QString virt_path = line.remove(0, list.at(0).length() + 1);

            if (!virt_path.startsWith('/'))
                virt_path.prepend('/');

            if (!virt_path.isEmpty() && size > 0 && !restrict_map.contains(virt_path))
                restrict_map.insert(virt_path, size);
        }

        f.close();

        QMap< QString, unsigned>::iterator it = restrict_map.begin();
        for (; it != restrict_map.end(); ++it){
            QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

            item->setText(0, it.key());
            item->setText(1, tr("%1 GiB").arg(it.value()));
        }

    }

    connect(toolButton_ADD, SIGNAL(clicked()), this, SLOT(slotAddExeption()));
    connect(toolButton_EDIT, SIGNAL(clicked()), this, SLOT(slotEditExeption()));
    connect(toolButton_DELETE, SIGNAL(clicked()), this, SLOT(slotDeleteExeption()));
    connect(toolButton_BROWSE, SIGNAL(clicked()), this, SLOT(slotAddDirExeption()));
    connect(listWidget_SKIPLIST, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(slotEditExeption()));

    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotRestrictMenu()));
    connect(toolButton_RECREATE, SIGNAL(clicked()), this, SLOT(slotRecreateShare()));
    connect(checkBox_SHAREHIDDEN, SIGNAL(clicked(bool)), this, SLOT(slotShareHidden(bool)));
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));

    connect(treeWidget_SIMPLE_MODE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(checkBox_SIMPLE_SHARE_MODE, SIGNAL(clicked()), this, SLOT(slotSimpleShareModeChanged()));

    slotSimpleShareModeChanged();
}

void SettingsSharing::updateShareView(){
    if (checkBox_SIMPLE_SHARE_MODE->isChecked()){
        treeWidget_SIMPLE_MODE->clear();

        StringPairList directories = ShareManager::getInstance()->getDirectories();
        for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it){
            QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_SIMPLE_MODE);

            item->setText(0, it->second.c_str());
            item->setText(1, it->first.c_str());
            item->setText(2, Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str());
            item->setText(3, QString().setNum(ShareManager::getInstance()->getShareSize(it->second)));
        }
    }

    label_TOTALSHARED->setText(tr("Total shared: %1")
                               .arg(WulforUtil::formatBytes(ShareManager::getInstance()->getShareSize())));
}

void SettingsSharing::slotRecreateShare(){
    ShareManager *SM = ShareManager::getInstance();

    SM->setDirty();
    SM->refresh(true);

    HashProgress progress(this);

    if (progress.exec() == QDialog::Accepted){
        updateShareView();
    }
}

void SettingsSharing::slotShareHidden(bool share){
    SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, share);
    ShareManager::getInstance()->setDirty();
    ShareManager::getInstance()->refresh(true);

    updateShareView();
}

void SettingsSharing::slotGetName(QModelIndex index){
    bool ok = false;
    QString dir_alias = QInputDialog::getText(this, tr("Select directory"), tr("Name"),
                                              QLineEdit::Normal, QDir(model->filePath(index)).dirName(), &ok);

    dir_alias = dir_alias.trimmed();

    if (!ok || dir_alias.isEmpty())
        return;

    model->setAlias(index, dir_alias);
}

void SettingsSharing::slotHeaderMenu(){
    WulforUtil::headerMenu(treeView);
}

void SettingsSharing::slotRestrictMenu(){
    WulforUtil *WU = WulforUtil::getInstance();
    QMenu *m = new QMenu(this);
    QAction *add = new QAction(WU->getPixmap(WulforUtil::eiEDITADD), tr("Add"), m);
    m->addAction(add);

    QAction *rem = NULL;

    QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();

    if (selected.size() > 0){
        rem = new QAction(WU->getPixmap(WulforUtil::eiEDITDELETE), tr("Remove"), m);
        m->addAction(rem);
    }

    QAction *ret = m->exec(QCursor::pos());

    delete m;

    if (ret == add){
        int size = -1;
        QString virt_path = "";
        bool ok = false;

        virt_path = QInputDialog::getText(this, tr("Enter virtual path name"), tr("Virtual path"), QLineEdit::Normal, "", &ok);

        if (!ok || virt_path.isEmpty())
            return;

        ok = false;
#if QT_VERSION >= 0x040500
        size = QInputDialog::getInt(this, tr("Enter restriction (in GiB)"), tr("Restriction"), 1, 1, 1000, 1, &ok);
#else
        size = QInputDialog::getInteger(this, tr("Enter restriction (in GiB)"), tr("Restriction"), 1, 1, 1000, 1, &ok);
#endif

        if (!ok)
            return;

        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

        if (!virt_path.startsWith('/'))
            virt_path.prepend('/');

        item->setText(0, virt_path);
        item->setText(1, tr("%1 GiB").arg(size));
    }
    else if (ret)
        qDeleteAll(selected);
}

void SettingsSharing::slotAddExeption(){
    bool ok;
    QString text = QInputDialog::getText(this, tr("Add item"), tr("Enter text:"), QLineEdit::Normal, "", &ok);

    if (ok && !text.isEmpty())
        listWidget_SKIPLIST->addItem(text);
}

void SettingsSharing::slotAddDirExeption(){
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose the directory"), QDir::home().dirName());

    if (!dir.isEmpty()){
        dir = QDir::toNativeSeparators(dir);

        if (!dir.endsWith(QDir::separator()))
            dir += QDir::separator();

        listWidget_SKIPLIST->addItem(dir + "*");
    }
}

void SettingsSharing::slotEditExeption(){
    QListWidgetItem *item = listWidget_SKIPLIST->currentItem();

    if (!item)
        return;

    QString old_text = item->text();
    int row = listWidget_SKIPLIST->row(item);
    bool ok = false;

    QString new_text = QInputDialog::getText(this, tr("Add item"), tr("Enter text:"), QLineEdit::Normal, old_text, &ok);

    if (ok && !new_text.isEmpty()){
        delete item;
        listWidget_SKIPLIST->insertItem(row, new_text);
        listWidget_SKIPLIST->setCurrentRow(row);
    }
}

void SettingsSharing::slotDeleteExeption(){
    QListWidgetItem *item = listWidget_SKIPLIST->currentItem();

    if (item)
        delete item;
}

void SettingsSharing::slotSimpleShareModeChanged(){
    if (!checkBox_SIMPLE_SHARE_MODE->isChecked()){
        if (!model){
            model = new ShareDirModel();
            treeView->setModel(model);

            treeView->setSortingEnabled(true);
            treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
            treeView->header()->hideSection(1);
            treeView->header()->hideSection(2);

            if (!WSGET(WS_SHAREHEADER_STATE).isEmpty())
                treeView->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SHAREHEADER_STATE).toAscii()));

            connect(model, SIGNAL(getName(QModelIndex)), this, SLOT(slotGetName(QModelIndex)));
            connect(model, SIGNAL(expandMe(QModelIndex)), treeView, SLOT(expand(QModelIndex)));

            model->beginExpanding();
        }
        else{
            model->beginExpanding();
        }
    }
    else{
        updateShareView();
    }
}

void SettingsSharing::slotContextMenu(const QPoint &){
    QList<QTreeWidgetItem*> selected = treeWidget_SIMPLE_MODE->selectedItems();
    QMenu *menu = new QMenu();
    QAction *add_new = NULL, *rem = NULL, *rename = NULL;
    WulforUtil *WU = WulforUtil::getInstance();

    add_new = new QAction(WU->getPixmap(WulforUtil::eiEDITADD), tr("Add"), menu);
    menu->addAction(add_new);

    if (selected.size() == 1){
        rename = new QAction(WU->getPixmap(WulforUtil::eiEDIT), tr("Rename"), menu);
        menu->addAction(rename);
    }

    if (selected.size() > 0){
        rem = new QAction(WU->getPixmap(WulforUtil::eiEDITDELETE), tr("Remove"), menu);
        menu->addAction(rem);
    }

    QAction *res = menu->exec(QCursor::pos());

    delete menu;

    if (!res)
        return;

    if (res == add_new){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select directory"), QDir::homePath());

        if (dir.isEmpty())
            return;

        dir = QDir::toNativeSeparators(dir);

        if (!dir.endsWith(PATH_SEPARATOR))
            dir += PATH_SEPARATOR_STR;

        bool ok = false;
        QString dir_alias = QInputDialog::getText(this, tr("Select directory"), tr("Name"),
                                                  QLineEdit::Normal, QDir(dir).dirName(), &ok);

        dir_alias = dir_alias.trimmed();

        if (!ok || dir_alias.isEmpty())
            return;

        try
        {
            ShareManager::getInstance()->addDirectory(dir.toStdString(), dir_alias.toStdString());
        }
        catch (const ShareException &e)
        {
            QMessageBox msg_box(QMessageBox::Critical,
                                tr("Error"),
                                QString::fromStdString(e.getError()),
                                QMessageBox::Ok);

            msg_box.exec();

            return;
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget_SIMPLE_MODE);

        item->setText(0, dir);
        item->setText(1, dir_alias);
        item->setText(2, "");
        item->setText(3, "");
    }
    else if (res == rem){
        foreach(QTreeWidgetItem *i, selected)
            ShareManager::getInstance()->removeDirectory(i->text(0).toStdString());
    }
    else if (res == rename){
        QTreeWidgetItem *item = selected.at(0);
        QString realname = item->text(0);
        QString virtname = item->text(1);
        bool ok = false;
        QString new_virtname = QInputDialog::getText(this, tr("Enter new name"),
                                                     tr("Name"), QLineEdit::Normal, virtname, &ok);

        if (!ok || new_virtname.isEmpty() || new_virtname == virtname)
            return;

        try {
            ShareManager::getInstance()->renameDirectory(realname.toStdString(), new_virtname.toStdString());
        }
        catch (const ShareException &e){
            QMessageBox msg_box(QMessageBox::Critical,
                                tr("Error"),
                                QString::fromStdString(e.getError()),
                                QMessageBox::Ok);

            msg_box.exec();

            return;
        }
    }

    updateShareView();
}

QString ShareDirModel::filePath( const QModelIndex & index ) const {
    return QDir::toNativeSeparators( QDirModel::filePath(index) );
}

ShareDirModel::ShareDirModel(QObject *parent){
    QDirModel::setParent(parent);
    QDirModel::setFilter((QDir::AllDirs | QDir::NoDotAndDotDot));

    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it){
        QString path = it->second.c_str();

        if (path.endsWith(QDir::separator()))
            path = path.left(path.lastIndexOf(QDir::separator()));

        emit expandMe(index(path));

        checked.insert(path);
    }
}

ShareDirModel::~ShareDirModel(){

}

Qt::ItemFlags ShareDirModel::flags(const QModelIndex& index) const{
    Qt::ItemFlags f = QDirModel::flags(index);

    if (index.column() == 0)
        f |= Qt::ItemIsUserCheckable;

    QString fp = filePath(index);

    foreach (QString file, checked){
#if QT_VERSION >= 0x040500
        if (fp.startsWith(file) && (fp.split(QDir::separator()).length() != file.split(QDir::separator()).length()) && fp != file){
#else
        if (fp.startsWith(file) && (fp.split(QDir::separator()).size() != file.split(QDir::separator()).size()) && fp != file){
#endif
            f &= ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

            break;
        }
    }

    return f;
}

QVariant ShareDirModel::data(const QModelIndex& index, int role = Qt::DisplayRole) const{
    if (!index.isValid())
        return QVariant();

    QString fp = filePath(index);

    switch (role){
        case Qt::CheckStateRole:
        {
            if (index.column() == 0){
                foreach (QString f, checked){
                    if (fp.startsWith(f) && fp.length() == f.length())
                        return Qt::Checked;
                }

                return (checked.contains(fp)? Qt::Checked : Qt::Unchecked);
            }

            break;
        }
        case Qt::ForegroundRole:
        {
            /*foreach (QString f, checked){
                if (f.startsWith(fp))
                    return QColor(0x1F, 0x8F, 0x1F);
            }*/

            break;
        }
        case Qt::FontRole:
        {
            if (index.column() == 0){
                QFont font;
                font.setBold(true);

                foreach (QString f, checked){
                    if (f == fp)
                        return font;
                }

                if (checked.contains(fp))
                    return font;
            }

            break;
        }
    }

    return QDirModel::data(index, role);
}

bool ShareDirModel::setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole)
{
    if (index.isValid() && index.column() == 0 && role == Qt::CheckStateRole){

        if (value.toInt() == Qt::Checked)
            emit getName(index);//checked.insert(filePath(index));
        else{
            try {
                QString path = filePath(index);

                if (!path.endsWith(QDir::separator()))
                    path += QDir::separator();

                ShareManager::getInstance()->removeDirectory(path.toStdString());
            }
            catch (const Exception&){}

            checked.remove(filePath(index));
        }

        return true;
    }

    return QDirModel::setData(index, value, role);
}

void ShareDirModel::setAlias(const QModelIndex &index, const QString &alias){
    QString fp = filePath(index);

    if (checked.contains(fp) || !QDir(fp).exists())
        return;

    checked.insert(fp);

    try
    {
        if (!fp.endsWith(QDir::separator()))
            fp += QDir::separator();

        ShareManager::getInstance()->addDirectory(fp.toStdString(), alias.toStdString());
    }
    catch (const ShareException &e)
    {
        QMessageBox msg_box(QMessageBox::Critical,
                            tr("Error"),
                            QString::fromStdString(e.getError()),
                            QMessageBox::Ok);

        msg_box.exec();

        return;
    }

    QDirModel::setData(index, true, Qt::CheckStateRole);

    emit layoutChanged();
}

void ShareDirModel::beginExpanding(){
    foreach (QString f, checked){
        QStack<QModelIndex> stack;
        QModelIndex i = index(f);

        while (i.isValid()){
            stack.push(i);

            i = i.parent();
        }

        while (!stack.isEmpty())
            emit expandMe(stack.pop());
    }
}
