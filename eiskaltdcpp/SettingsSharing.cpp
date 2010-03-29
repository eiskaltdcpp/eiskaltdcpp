#include "SettingsSharing.h"
#include "HashProgress.h"
#include "WulforUtil.h"
#include "WulforSettings.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/ShareManager.h"

#include <QTreeWidget>
#include <QList>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
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
    SM->set(SettingsManager::SLOTS, spinBox_UPLOAD->value());
    SM->set(SettingsManager::SKIPLIST_SHARE, _tq(lineEdit_SKIPLIST->text()));
    SM->set(SettingsManager::MAX_HASH_SPEED, spinBox_MAXHASHSPEED->value());
    SM->set(SettingsManager::FAST_HASH, checkBox_FASTHASH->isChecked());

    WSSET(WS_SHAREHEADER_STATE, treeView->header()->saveState().toBase64());

    SM->save();

    QFile f(_q(Util::getPath(Util::PATH_USER_CONFIG) + "PerFolderLimit.conf"));

    if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)){

        QList<QTreeWidgetItem*> items = treeWidget->findItems("*", Qt::MatchWildcard, 0);
        QTextStream stream(&f);

        foreach (QTreeWidgetItem *i, items)
            stream << i->text(1).left(i->text(1).indexOf(tr("GiB"))) << i->text(0) << '\n';

        stream.flush();

        f.close();
    }
}

void SettingsSharing::init(){
    checkBox_SHAREHIDDEN->setChecked(BOOLSETTING(SHARE_HIDDEN));
    checkBox_SHARE_TEMP_FILES->setChecked(BOOLSETTING(SHARE_TEMP_FILES));
    checkBox_FOLLOW->setChecked(BOOLSETTING(FOLLOW_LINKS));
    checkBox_FASTHASH->setChecked(BOOLSETTING(FAST_HASH));
    lineEdit_SKIPLIST->setText(_q(SETTING(SKIPLIST_SHARE)));
    spinBox_UPLOAD->setValue(SETTING(SLOTS));
	spinBox_MAXHASHSPEED->setValue(SETTING(MAX_HASH_SPEED));
    spinBox_EXTRA->setValue(SETTING(MIN_UPLOAD_SPEED));

    label_TOTALSHARED->setText(tr("Total shared: %1")
                               .arg(QString::fromStdString(Util::formatBytes(ShareManager::getInstance()->getShareSize()))));

    model = new ShareDirModel();
    treeView->setModel(model);

    toolButton_RECREATE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiRELOAD));

    treeView->setSortingEnabled(true);
    treeView->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView->header()->hideSection(1);
    treeView->header()->hideSection(2);

    if (!WSGET(WS_SHAREHEADER_STATE).isEmpty())
        treeView->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SHAREHEADER_STATE).toAscii()));

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
    else
        treeWidget->setEnabled(false);

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotRestrictMenu()));
    connect(toolButton_RECREATE, SIGNAL(clicked()), this, SLOT(slotRecreateShare()));
    connect(model, SIGNAL(getName(QModelIndex)), this, SLOT(slotGetName(QModelIndex)));
    connect(model, SIGNAL(expandMe(QModelIndex)), treeView, SLOT(expand(QModelIndex)));
    connect(checkBox_SHAREHIDDEN, SIGNAL(clicked(bool)), this, SLOT(slotShareHidden(bool)));
    connect(treeView->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu()));

    model->beginExpanding();
}

void SettingsSharing::updateShareView(){
    label_TOTALSHARED->setText(tr("Total shared: %1")
                               .arg(QString::fromStdString(Util::formatBytes(ShareManager::getInstance()->getShareSize()))));
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

        size = QInputDialog::getInt(this, tr("Enter restriction (in GiB)"), tr("Restriction"), 1, 1, 1000, 1, &ok);

        if (!ok)
            return;

        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

        item->setText(0, virt_path);
        item->setText(1, tr("%1 GiB").arg(size));
    }
    else if (ret)
        qDeleteAll(selected);
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
        if (fp.startsWith(file) && fp != file){
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
                    if (fp.startsWith(f))
                        return Qt::Checked;
                }

                return (checked.contains(fp) ? Qt::Checked : Qt::Unchecked);
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
                    if (f.startsWith(fp))
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
