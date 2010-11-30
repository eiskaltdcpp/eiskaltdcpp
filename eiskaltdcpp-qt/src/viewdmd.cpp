#include "viewdmd.h"

#include "MainWindow.h"

#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QThread>
#include <QScrollArea>
#include <QProgressBar>
#include <QDialog>

#include "dcpp/SettingsManager.h"
#include "dcpp/HashManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/DirectoryListing.h"
#include "dcpp/ADLSearch.h"

#ifdef Q_WS_WIN
    #define nix_file "'"
#else
    #define nix_file "'file://"
#endif

ViewDMD::ViewDMD(QString fileName, QWidget *parent) :
    QWidget(parent),
    arena_menu(NULL),
    fName(fileName)
{
    this->setupUi(this);
    QDmd dmd;
    if (!dmd.load(fileName)){
	error =  tr("Failed to open description");
	deleteLater();
	return;
    }
    QTemporaryFile fCover;
    fCover.open();
    dmd.getCover().save(fCover.fileName()+".jpg", "JPG");
    QString w = tr("%1").arg((dmd.getCover().width() < 256 ? dmd.getCover().width() : 256));

    QString html = "<!DOCTYPE HTML PUBLIC  '-//W3C//DTD HTML 4.01 Transitional//EN' 'http://www.w3.org/TR/html4/loose.dtd'>"
                   "<HTML>"
                   "<HEAD>"
                   "<meta http-equiv='content-type' content='text/html'; charset='UTF-8'>"
                   "<meta http-equiv='content-style-type' content='text/css'>"
                   "<title>Upload</title>"
                   "</HEAD>"
                   "<BODY>"
                   "<img src="+tr(nix_file)+fCover.fileName()+".jpg' width="+w+" align=right><p align=justify>";
    html += dmd.getHTML();
    html += "</BODY>"
            "</HTML>";
    this->textBrowser->setHtml(html);
    treeView->setModel(dmd.getModel());
    treeView->setColumnWidth(0, 450);
    treeView->setColumnWidth(1, 200);

    QPixmap pix;
    int n = 0;
    for (int i = 0; i < dmd.getImageCount(); ++i){
        pix = QPixmap::fromImage(dmd.getImage(i));

        QWidget *nTab = new QWidget();
        nTab->setObjectName(tr("ScreenShot%1").arg(++n));

        QVBoxLayout *mainLay = new QVBoxLayout(nTab);
        mainLay->setObjectName(tr("mainLay%1").arg(n));

        QScrollArea *sArea = new QScrollArea(nTab);
        sArea->setObjectName(tr("sArea%1").arg(n));
        sArea->setWidgetResizable(true);

        QVBoxLayout *inLay = new QVBoxLayout(sArea);
        inLay->setObjectName(tr("inLay%1").arg(n));

        QLabel *l = new QLabel(sArea);
        l->setObjectName(tr("labelPix%1").arg(n));
        l->setStyleSheet(QString::fromUtf8("QLabel {background-color: rgb(255, 255, 255);"));

        inLay->addWidget(l);
        mainLay->addWidget(sArea);

        tabWidget->insertTab(tabWidget->count()-1, nTab, tr("Screenshot %1").arg(n));
        l->setPixmap(pix);
    }
    tabWidget->setCurrentIndex(0);

    MainWindow *mwnd = MainWindow::getInstance();

    arena_menu = new QMenu(this->windowTitle());
    QAction *close_wnd = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);
    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));

    mwnd->addArenaWidget(this);
    mwnd->addArenaWidgetOnToolbar(this);
    mwnd->mapWidgetOnArena(this);
    setAttribute(Qt::WA_DeleteOnClose);
}

ViewDMD::~ViewDMD()
{
    delete arena_menu;
    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
}

void ViewDMD::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
	retranslateUi(this);
        break;
    default:
        break;
    }
}

QString ViewDMD::getArenaShortTitle()
{
    QString rez;
    int i;
    for (i = fName.size(); fName[i] != QDir::separator(); --i);
    rez = fName.right(fName.size()-i-1);
    if (rez.indexOf(".")!=-1){
	rez = rez.left(rez.indexOf("."));
    }
    return rez;
};

void ViewDMD::closeEvent(QCloseEvent *e)
{
    setAttribute(Qt::WA_DeleteOnClose);

    e->accept();
}

bool createTreeDir(QString dir, Node *n)
{
    for (int i = 0; i<n->children.count(); ++i){
	if (n->children[i]->children.count() != 0){
	    QDir d(dir);
	    if (d.exists(n->children[i]->str1)){
		continue;
	    }
	    if (!d.mkdir(n->children[i]->str1)){
		return false;
	    }
	    if (!createTreeDir(dir+n->children[i]->str1+QDir::separator(), n->children[i])){
		return false;
	    }
	}
    }
    return true;
}

bool initDownload(QHash<QString, QString> aShares, QString dir, Node *n)
{
    QDialog *stat = new QDialog(MainWindow::getInstance());
    stat->setWindowTitle(MainWindow::getInstance()->tr("Copy file"));
    QVBoxLayout *lay = new QVBoxLayout(stat);
    QLabel *l = new QLabel(stat);
    QProgressBar *p = new QProgressBar(stat);
    p->setMinimum(0);
    p->setMaximum(100);
    p->setValue(0);
    stat->setLayout(lay);
    lay->addWidget(l);
    lay->addWidget(p);

    QString renameAll = "ASK";
    QString copyAll = "ASK";
    for (int i = 0; i<n->children.count(); ++i){
	if (n->children[i]->children.count() != 0){
	    if (!initDownload(aShares, dir+n->children[i]->str1+QDir::separator(), n->children[i])){
		return false;
	    }
	} else {
	    try {
		if (QFile::exists(dir+n->children[i]->str1)){
		    QString str = n->children[i]->str1;
                    if (renameAll == "ASK"){
			QMessageBox *d = new QMessageBox(0);//Создаем диалог с вопросом
			d->setIcon(QMessageBox::Question);
			d->addButton(QMessageBox::Yes);
			d->addButton(QMessageBox::YesToAll);
			d->addButton(QMessageBox::No);
			d->addButton(QMessageBox::NoToAll);
                        d->setWindowTitle(QObject::tr("Rename?"));
                        d->setText(QObject::tr("File %1 already exist.\nIf you answer no - the file will be replaced.\nRename?").arg(dir+str));
			d->exec();
			QMessageBox::StandardButton rez = d->standardButton(d->clickedButton());
                        delete d;
			if ((rez == QMessageBox::Yes) ||  //Ответ - копировать или копировать все
			    (rez == QMessageBox::YesToAll)){
			    int x = 1;
			    while (QFile::exists(dir+str)){
				str = n->children[i]->str1;
				if (str.indexOf(".") >= 0){
				    str.insert(str.indexOf("."), QObject::tr("(%1)").arg(x));
				} else {
				    str += QObject::tr("(%1)").arg(x);
				}
				++x;
			    }
			    if (!QFile::rename(dir+n->children[i]->str1, dir+str)){
				QMessageBox::critical(0,
						      QObject::tr("Can't rename"),
						      QObject::tr("Can't rename %1 to %2").arg(n->children[i]->str1, str));
			    }
			    if (rez == QMessageBox::YesToAll){ //Если выбрано - копировать все
                                renameAll = "RENAME"; //Устанавливаем действие - копировать все
			    }
			}
			if (rez == QMessageBox::NoToAll){ //Если выбрано - Игнорировать все
                            renameAll = "DELETE"; //Устанавливаем действие - Игнорировать все
			}
		    } else {
                        if (renameAll == "RENAME"){
			    int x = 1;
			    while (QFile::exists(dir+str)){
				str = n->children[i]->str1;
				if (str.indexOf(".") >= 0){
				    str.insert(str.indexOf("."), QObject::tr("(%1)").arg(x));
				} else {
				    str += QObject::tr("(%1)").arg(x);
				}
				++x;
			    }
			    if (!QFile::rename(dir+n->children[i]->str1, dir+str)){
				QMessageBox::critical(0,
						      QObject::tr("Can't rename"),
						      QObject::tr("Can't rename %1 to %2").arg(n->children[i]->str1, str));
			    }
			} else {
                            if (renameAll == "DELETE"){
                                QFile::remove(dir+n->children[i]->str1);
			    }
			}
		    }
		}
		if (aShares.contains(n->children[i]->str2)){
                    bool copy;
                    if (copyAll == "ASK"){
                        QMessageBox *d = new QMessageBox(0);//Создаем диалог с вопросом
                        d->setIcon(QMessageBox::Question);
                        d->addButton(QMessageBox::Yes);
                        d->addButton(QMessageBox::YesToAll);
                        d->addButton(QMessageBox::No);
                        d->addButton(QMessageBox::NoToAll);
                        d->setWindowTitle(QObject::tr("Copy?"));
                        d->setText(QObject::tr("File\n\n%1\n\nyou already have.\nCopy?").arg(n->children[i]->str1));
                        d->exec();
                        QMessageBox::StandardButton rez = d->standardButton(d->clickedButton());
                        delete d;
                        if (rez == QMessageBox::Yes)
                            copy = true;
                        if (rez == QMessageBox::YesToAll){
                            copy = true;
                            copyAll = "COPY";
                        }
                        if (rez == QMessageBox::No)
                            copy = false;
                        if (rez == QMessageBox::NoToAll){
                            copy = false;
                            copyAll = "IGNORE";
                        }
                    } else {
                        if (copyAll == "COPY"){
                            copy = true;
                        } else {
                            if (copyAll == "IGNORE"){
                                copy = false;
                            }
                        }
                    }
                    if (copy){
                        QFile fIN(aShares[n->children[i]->str2]);
                        QFile fOUT(dir+n->children[i]->str1);
                        fIN.open(QIODevice::ReadOnly);
                        if (!fOUT.open(QIODevice::WriteOnly)){
                            QMessageBox::critical(0,
                                                  QObject::tr("Can't copy"),
                                                  QObject::tr("Can't copy file\n\n%1\n\nto\n\n%2").arg(aShares[n->children[i]->str2], dir+n->children[i]->str1));
                        } else {
                            p->setValue(0);
                            p->setMaximum(fIN.size()/1024);
                            l->setText(QObject::tr("Copy file %1").arg(n->children[i]->str1));
                            stat->show();
                            qApp->processEvents();
                            while (fIN.atEnd()){
                                fOUT.write(fIN.read(1024));
                                p->setValue(p->value()+1);
                                qApp->processEvents();
                            }
                            l->setText(QObject::tr("Copy finished"));
                            qApp->processEvents();
                        }
                    }
                    //QFile::copy(aShares[n->children[i]->str2], dir+n->children[i]->str1);
		} else {
		    QueueManager::getInstance()->add((dir+n->children[i]->str1).toStdString(), n->children[i]->size, TTHValue(n->children[i]->str2.toStdString())/*, UserPtr(), ""*/); //NOTE: core 0.77
		}
	    }
	    catch (const std::exception& e){
                delete stat;
                return false;
	    }
	}
    }
    delete stat;
    return true;
}

void createList(QMultiHash<QString, QString> *aDirs, DirectoryListing::Directory *dir, QHash<QString, QString> *list, QString dirName)
{
    if (!(dir && list))
	return;

    DirectoryListing::Directory::Iter it;
    DirectoryListing::File::Iter iter;
    for (iter = dir->files.begin(); iter != dir->files.end(); ++iter){
	QString name = QString::fromUtf8((*iter)->getName().c_str());
	QString str = dirName + QDir::separator() + name;
	str.remove(0, 1);
	QString psev = str.left(str.indexOf(QDir::separator()));
	str.remove(0, str.indexOf(QDir::separator())+1);
	QList<QString> dirs = aDirs->values(psev);
	for (int j = 0; j < dirs.size(); ++j){
	    QString fullStr = dirs[j]+str;
	    if (HashManager::getInstance()->getFileTTHif(fullStr.toStdString()) != NULL){
		(*list)[QString::fromUtf8((*iter)->getTTH().toBase32().c_str())] = fullStr;
		continue;
	    }
	}
    }


    for (it = dir->directories.begin(); it != dir->directories.end(); ++it)
	createList(aDirs, *it, list, dirName + QDir::separator() + QString::fromUtf8((*it)->getName().c_str()));
}

void ViewDMD::on_pushButton_clicked()
{
    SettingsManager *SM = SettingsManager::getInstance();
    QString downDir = QString::fromStdString(SM->get(SettingsManager::DOWNLOAD_DIRECTORY, false));
    if (((TreeModel *)(this->treeView->model()))->getRoot()->children.count() > 1){
	QString upName = fName.left(fName.length()-12);
	int x;
	for (x = upName.length()-1; upName[x] != QDir::separator(); --x);
	upName.remove(0, x+1);
	QDir d(downDir);
	if (!d.exists(upName)){
	    if (!d.mkdir(upName)){
		QMessageBox::critical(this, tr("Error"), tr("Can't create folders for download"));
		return;
	    }
	}
	downDir += upName + QDir::separator();
    }
    if (!createTreeDir(downDir, ((TreeModel *)(this->treeView->model()))->getRoot())){
	QMessageBox::critical(this, tr("Error"), tr("Can't create folders for download"));
	return;
    }

    StringPairList directories = ShareManager::getInstance()->getDirectories();
    StringPairList::iterator it;

    QHash<QString, QString> shares;
    dcpp::DirectoryListing *listing = new dcpp::DirectoryListing(HintedUser(ClientManager::getInstance()->getMe(),"")); //NOTE: core 0.77
    listing->loadFile(ShareManager::getInstance()->getOwnListFile());
    listing->getRoot()->setName(QString("My Uploads").toStdString());
    ADLSearchManager::getInstance()->matchListing(*listing);

    QMultiHash<QString, QString> dirs;
    for (it = directories.begin(); it != directories.end(); ++it){
	dirs.insert(QString::fromStdString((*it).first), QString::fromStdString((*it).second));
    }

    createList(&dirs, listing->getRoot(), &shares, tr(""));
    delete listing;

    if (!initDownload(shares, downDir, ((TreeModel *)(this->treeView->model()))->getRoot())){
	QMessageBox::critical(this, tr("Error"), tr("Error of add files to download"));
	return;
    }
    QMessageBox::information(this, tr("Ok"), tr("Download to begin in folder\n\n%1").arg(QString::fromStdString(SM->get(SettingsManager::DOWNLOAD_DIRECTORY, false))));
    this->close();
}
