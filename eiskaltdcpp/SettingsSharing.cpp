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

void SettingsSharing::ok(){
    SettingsManager *SM = SettingsManager::getInstance();

    SM->set(SettingsManager::FOLLOW_LINKS, checkBox_FOLLOW->isChecked());
    SM->set(SettingsManager::MIN_UPLOAD_SPEED, spinBox_EXTRA->value());
    SM->set(SettingsManager::SLOTS, spinBox_UPLOAD->value());

    WSSET(WS_SHAREHEADER_STATE, treeView->header()->saveState().toBase64());

    SM->save();
}

void SettingsSharing::init(){
    checkBox_SHAREHIDDEN->setChecked(BOOLSETTING(SHARE_HIDDEN));
    checkBox_FOLLOW->setChecked(BOOLSETTING(FOLLOW_LINKS));

    spinBox_UPLOAD->setValue(SETTING(SLOTS));
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

    treeView->header()->restoreState(QByteArray::fromBase64(WSGET(WS_SHAREHEADER_STATE).toAscii()));

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
    QMenu * mcols = new QMenu(this);
    QAction * column;
    int index;

    for (int i = 0; i < model->columnCount(); ++i) {
        index = treeView->header()->logicalIndex(i);
        column = mcols->addAction(model->headerData(index, Qt::Horizontal).toString());
        column->setCheckable(true);

        column->setChecked(!treeView->header()->isSectionHidden(index));
        column->setData(index);
    }

    QAction * chosen = mcols->exec(QCursor::pos());

    if (chosen) {
        index = chosen->data().toInt();

        if (treeView->header()->isSectionHidden(index)) {
            treeView->header()->showSection(index);
        } else {
            treeView->header()->hideSection(index);
        }
    }

    delete mcols;
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
