#include "SettingsSharing.h"
#include "HashProgress.h"
#include "WulforUtil.h"

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

using namespace dcpp;

SettingsSharing::SettingsSharing(QWidget *parent):
        QWidget(parent)
{
    setupUi(this);

    init();
}

SettingsSharing::~SettingsSharing(){
}

void SettingsSharing::ok(){
    SettingsManager *SM = SettingsManager::getInstance();

    SM->set(SettingsManager::FOLLOW_LINKS, checkBox_FOLLOW->isChecked());
    SM->set(SettingsManager::MIN_UPLOAD_SPEED, spinBox_EXTRA->value());
    SM->set(SettingsManager::SLOTS, spinBox_UPLOAD->value());

    SM->save();
}

void SettingsSharing::init(){
    checkBox_SHAREHIDDEN->setChecked(BOOLSETTING(SHARE_HIDDEN));
    checkBox_FOLLOW->setChecked(BOOLSETTING(FOLLOW_LINKS));

    spinBox_UPLOAD->setValue(SETTING(SLOTS));
    spinBox_EXTRA->setValue(SETTING(MIN_UPLOAD_SPEED));

    label_TOTALSHARED->setText(tr("Total shared: %1")
                               .arg(QString::fromStdString(Util::formatBytes(ShareManager::getInstance()->getShareSize()))));

    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it){
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

        item->setText(0, it->second.c_str());
        item->setText(1, it->first.c_str());
        item->setText(2, Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str());
        item->setText(3, QString().setNum(ShareManager::getInstance()->getShareSize(it->second)));
    }

    treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    toolButton_RECREATE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiRELOAD));

    connect(toolButton_RECREATE, SIGNAL(clicked()), this, SLOT(slotRecreateShare()));
    connect(treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));
    connect(checkBox_SHAREHIDDEN, SIGNAL(clicked(bool)), this, SLOT(slotShareHidden(bool)));
}

void SettingsSharing::updateShareView(){
    treeWidget->clear();

    StringPairList directories = ShareManager::getInstance()->getDirectories();
    for (StringPairList::iterator it = directories.begin(); it != directories.end(); ++it){
        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

        item->setText(0, it->second.c_str());
        item->setText(1, it->first.c_str());
        item->setText(2, Util::formatBytes(ShareManager::getInstance()->getShareSize(it->second)).c_str());
        item->setText(3, QString().setNum(ShareManager::getInstance()->getShareSize(it->second)));
    }

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

void SettingsSharing::slotContextMenu(const QPoint &){
    QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();
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

        QTreeWidgetItem *item = new QTreeWidgetItem(treeWidget);

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

void SettingsSharing::slotShareHidden(bool share){
    SettingsManager::getInstance()->set(SettingsManager::SHARE_HIDDEN, share);
    ShareManager::getInstance()->setDirty();
    ShareManager::getInstance()->refresh(true);

    updateShareView();
}
