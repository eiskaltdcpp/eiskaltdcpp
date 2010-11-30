#include "MainWindow.h"
#include "createdmd.h"
#include "uploadsframe.h"
#include "viewdmd.h"
#include "qdmd.h"
#include "TransferView.h"
#include "WulforUtil.h"

#include "dcpp/ShareManager.h"
#include "TransferView.h"
#include "HashProgress.h"
#include "dcpp/ClientManager.h"
#include "dcpp/ADLSearch.h"
#include "dcpp/HashManager.h"

#include <QHBoxLayout>
#include <QDebug>
#include <QVector>
#include <QStringList>
#include <QFileInfoList>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QTreeWidgetItem>
#include <QMapIterator>
#include <QList>
#include <QVariant>
#include <QMultiHash>
#include <QProgressBar>


UploadsFrame::UploadsFrame(QWidget *parent) :
	QWidget(parent),
	fName(QObject::tr("Uploads")),
        exist7z(false),
	shareSize(-1)

{
    setUnload(false);
    exist7z = lib7z::check7z();
    if (!exist7z){
        QMessageBox::critical(this, tr("Error"), tr("7z not found"));
    }
    setupUi(this);
    toolBtn = new QAction(tr("Menu"), this);
    this->type_btn = new QTypeContentButton(false, this->frame);
    ((QHBoxLayout *)this->frame->layout())->insertWidget(3, this->type_btn);

/*    types.clear();
    #define SM SearchManager
    #define SL QStringList
    types[SM::TYPE_VIDEO_FOREIGN_FILMS]	    = SL() << tr("Foreign films")	<< tr(".fof_vid_dmd");
    types[SM::TYPE_VIDEO_FOREIGN_SERIALS]   = SL() << tr("Foreign serials")	<< tr(".fos_vid_dmd");
    types[SM::TYPE_VIDEO_FOREIGN_CARTOONS]  = SL() << tr("Foreign cartoons")	<< tr(".foc_vid_dmd");
    types[SM::TYPE_VIDEO_OUR_FILMS]	    = SL() << tr("Our films")		<< tr(".uof_vid_dmd");
    types[SM::TYPE_VIDEO_OUR_SERIALS]	    = SL() << tr("Our serials")		<< tr(".ous_vid_dmd");
    types[SM::TYPE_VIDEO_OUR_CARTOONS]	    = SL() << tr("Our cartoons")	<< tr(".ouc_vid_dmd");
    types[SM::TYPE_VIDEO_TUTORIAL]	    = SL() << tr("Video Tutorial")	<< tr(".tut_vid_dmd");
    types[SM::TYPE_VIDEO_CLIPS]		    = SL() << tr("Clips")		<< tr(".cli_vid_dmd");
    types[SM::TYPE_VIDEO_CONCERTS]	    = SL() << tr("Concerts")		<< tr(".con_vid_dmd");
    types[SM::TYPE_VIDEO_KARAOKE]	    = SL() << tr("Caraoke")		<< tr(".kar_vid_dmd");
    types[SM::TYPE_VIDEO_HUMOR]		    = SL() << tr("Humor video")		<< tr(".hum_vid_dmd");
    types[SM::TYPE_VIDEO_ANIME]		    = SL() << tr("Anime video")		<< tr(".ani_vid_dmd");
    types[SM::TYPE_VIDEO_18]		    = SL() << tr("Video 18+")		<< tr(".ero_vid_dmd");
    types[SM::TYPE_VIDEO_OTHER]		    = SL() << tr("Other video")		<< tr(".oth_vid_dmd");

    types[SM::TYPE_AUDIO_ROCK]		= SL() << tr("Rock")		    << tr(".roc_aud_dmd");
    types[SM::TYPE_AUDIO_POP]		= SL() << tr("Pop")		    << tr(".pop_aud_dmd");
    types[SM::TYPE_AUDIO_JAZZ]		= SL() << tr("Jazz")		    << tr(".jaz_aud_dmd");
    types[SM::TYPE_AUDIO_BLUES]		= SL() << tr("Blues")		    << tr(".blu_aud_dmd");
    types[SM::TYPE_AUDIO_ELECTRONIC]	= SL() << tr("Electronic music")    << tr(".ele_aud_dmd");
    types[SM::TYPE_AUDIO_SOUNDTRACKS]	= SL() << tr("Soundtracks")	    << tr(".sou_aud_dmd");
    types[SM::TYPE_AUDIO_NOTES]		= SL() << tr("Notes")		    << tr(".not_aud_dmd");
    types[SM::TYPE_AUDIO_AUDIOBOOKS]	= SL() << tr("Audiobooks")	    << tr(".aud_aud_dmd");
    types[SM::TYPE_AUDIO_OTHER]		= SL() << tr("Other music")	    << tr(".oth_aud_dmd");

    types[SM::TYPE_IMAGE_AVATARS]	= SL() << tr("Avatars")		<< tr(".ava_img_dmd");
    types[SM::TYPE_IMAGE_WALLPAPERS]	= SL() << tr("Wallpapers")	<< tr(".wal_img_dmd");
    types[SM::TYPE_IMAGE_ANIME]		= SL() << tr("Anime images")	<< tr(".ani_img_dmd");
    types[SM::TYPE_IMAGE_NATURE]	= SL() << tr("Nature")		<< tr(".nat_img_dmd");
    types[SM::TYPE_IMAGE_ANIMALS]	= SL() << tr("Animals")		<< tr(".mal_img_dmd");
    types[SM::TYPE_IMAGE_CARS]		= SL() << tr("Cars and moto")	<< tr(".car_img_dmd");
    types[SM::TYPE_IMAGE_HUMOR]		= SL() << tr("Humor images")	<< tr(".hum_img_dmd");
    types[SM::TYPE_IMAGE_SPORT]		= SL() << tr("Sport")		<< tr(".spo_img_dmd");
    types[SM::TYPE_IMAGE_SCREENSHOTS]	= SL() << tr("Screenshots")	<< tr(".scr_img_dmd");
    types[SM::TYPE_IMAGE_ARTISTIC]	= SL() << tr("Artistic photo")	<< tr(".art_img_dmd");
    types[SM::TYPE_IMAGE_ABSTRACT]	= SL() << tr("Abstract")	<< tr(".abs_img_dmd");
    types[SM::TYPE_IMAGE_18]		= SL() << tr("Images 18+")	<< tr(".ero_img_dmd");
    types[SM::TYPE_IMAGE_OTHER]		= SL() << tr("Other images")	<< tr(".oth_img_dmd");

    types[SM::TYPE_GAMES_ACTION]    = SL() << tr("Action")	    << tr(".act_gam_dmd");
    types[SM::TYPE_GAMES_RPG]	    = SL() << tr("RPG")		    << tr(".rpg_gam_dmd");
    types[SM::TYPE_GAMES_STRATEGY]  = SL() << tr("Strategy")	    << tr(".str_gam_dmd");
    types[SM::TYPE_GAMES_SIM]	    = SL() << tr("Sim")		    << tr(".sim_gam_dmd");
    types[SM::TYPE_GAMES_QUESTS]    = SL() << tr("Quests")	    << tr(".que_gam_dmd");
    types[SM::TYPE_GAMES_ARCADE]    = SL() << tr("Arcade")	    << tr(".arc_gam_dmd");
    types[SM::TYPE_GAMES_MINI]	    = SL() << tr("Mini-games")	    << tr(".min_gam_dmd");
    types[SM::TYPE_GAMES_CONSOLES]  = SL() << tr("For consoles")    << tr(".con_gam_dmd");
    types[SM::TYPE_GAMES_LINUX]	    = SL() << tr("For Linux")	    << tr(".lin_gam_dmd");
    types[SM::TYPE_GAMES_MAC]	    = SL() << tr("For Mac")	    << tr(".mac_gam_dmd");
    types[SM::TYPE_GAMES_ADD_ON]    = SL() << tr("Add-ons")	    << tr(".add_gam_dmd");
    types[SM::TYPE_GAMES_OTHER]	    = SL() << tr("Other games")	    << tr(".oth_gam_dmd");

    types[SM::TYPE_SOFT_SYSTEM]		= SL() << tr("System soft")		<< tr(".sys_sof_dmd");
    types[SM::TYPE_SOFT_ANTI_VIR]	= SL() << tr("Anti-viruses")		<< tr(".avr_sof_dmd");
    types[SM::TYPE_SOFT_AUDIO]		= SL() << tr("Audio soft")		<< tr(".aud_sof_dmd");
    types[SM::TYPE_SOFT_VIDEO]		= SL() << tr("Video soft")		<< tr(".vid_sof_dmd");
    types[SM::TYPE_SOFT_GRAPHIC]	= SL() << tr("Graphic soft")		<< tr(".gra_sof_dmd");
    types[SM::TYPE_SOFT_OFFICE]		= SL() << tr("Office soft")		<< tr(".off_sof_dmd");
    types[SM::TYPE_SOFT_DRIVERS]	= SL() << tr("Drivers and frimware")	<< tr(".drv_sof_dmd");
    types[SM::TYPE_SOFT_DEVELOP]	= SL() << tr("Develop")			<< tr(".dev_sof_dmd");
    types[SM::TYPE_SOFT_DIRECTORIES]	= SL() << tr("Directories and gis")	<< tr(".dir_sof_dmd");
    types[SM::TYPE_SOFT_OTHER]		= SL() << tr("Other soft")		<< tr(".oth_sof_dmd");

    types[SM::TYPE_BOOK_HUMOR]	    = SL() << tr("Humor")	    << tr(".hum_boo_dmd");
    types[SM::TYPE_BOOK_MANGA]	    = SL() << tr("Manga")	    << tr(".man_boo_dmd");
    types[SM::TYPE_BOOK_TUTORIAL]   = SL() << tr("Tutorial")	    << tr(".tut_boo_dmd");
    types[SM::TYPE_BOOK_FEATURE]    = SL() << tr("Feature book")    << tr(".fea_boo_dmd");
    types[SM::TYPE_BOOK_TECHNIKAL]  = SL() << tr("Technikal")	    << tr(".tec_boo_dmd");
    types[SM::TYPE_BOOK_OTHER]	    = SL() << tr("Other books")	    << tr(".oth_boo_dmd");

    types[SM::TYPE_UP_OTHER]	    = SL() << tr("Other uploads")   << tr("oth_dmd");*/

    this->current_type = this->type_btn->getNumber();
    connect(this->type_btn, SIGNAL(typeChanged()), this, SLOT(chandgeType()));
    this->reload();
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(reload()));
    timer->start(1000);
    MainWindow::getInstance()->addArenaWidget(this);

    //connect(TransferView::getInstance(), SIGNAL(loadDMD(QString)), this, SLOT(loadDMD(QString)));
}

UploadsFrame::~UploadsFrame()
{
    MainWindow::getInstance()->remArenaWidget(this);

    delete this->toolBtn;
    delete this->timer;
    delete this->type_btn;
}

void UploadsFrame::changeEvent(QEvent *e)
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

QWidget *UploadsFrame::getWidget()
{
    return this;
}

QString UploadsFrame::getArenaTitle()
{
    return fName;
}

QString UploadsFrame::getArenaShortTitle()
{
    return fName;
}

QMenu *UploadsFrame::getMenu()
{
    return new QMenu();
}

void UploadsFrame::chandgeType()
{
    this->shareSize = -1;
    this->reload();
}

void UploadsFrame::reload()
{
    try {
	if (this->shareSize == ShareManager::getInstance()->getSharedFiles()){
	    return;
	}
	this->shareSize = ShareManager::getInstance()->getSharedFiles();
	dcpp::DirectoryListing *listing = new dcpp::DirectoryListing(HintedUser(ClientManager::getInstance()->getMe(),"")); //NOTE: core 0.77
	listing->loadFile(ShareManager::getInstance()->getOwnListFile());
	listing->getRoot()->setName(QString("My Uploads").toStdString());
	ADLSearchManager::getInstance()->matchListing(*listing);
	QString dirName("");

	QStringList fileList;
	createTree(listing->getRoot(), &fileList, dirName);
	delete listing;

	StringPairList directories = ShareManager::getInstance()->getDirectories();
	StringPairList::iterator it;

	QMultiHash<QString, QString> shares;

	for (it = directories.begin(); it != directories.end(); ++it){
	    shares.insert(QString::fromStdString((*it).first), QString::fromStdString((*it).second));
	}

	this->treeWidget->clear();
	this->treeWidget->setHeaderLabels(QStringList() << tr("Name") << tr("Type") << tr("Size") << tr("Progress"));
	this->treeWidget->setColumnWidth(0, 200);
	this->treeWidget->setColumnWidth(1, 200);
	this->treeWidget->setColumnWidth(2, 100);
	for (int i = 0; i < fileList.size(); ++i){
	    QString tmp = fileList[i];
	    QString psev = tmp.left(tmp.indexOf(QDir::separator()));
	    tmp.remove(0, tmp.indexOf(QDir::separator())+1);
	    QList<QString> dirs = shares.values(psev);
	    for (int j = 0; j < dirs.size(); ++j){
		QString str = dirs[j]+tmp;
		if (HashManager::getInstance()->getFileTTHif(str.toStdString()) != NULL){
		    if (!QDmd::isValid(str)){
			continue;
		    }
                    QList<int> keys = WulforUtil::getInstance()->getTypeKeys();
		    QString type;
                    for (int i = 0; i < keys.count(); ++i){
                        if (WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(keys[i]))[1] == str.right(12)){
                            type = WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(keys[i]))[0];
			    break;
			}
		    }
		    int x;
		    QString n = str.left(str.length()-12);
		    for (x = n.size()-1; n[x]!=QDir::separator(); --x);
		    n = n.right(n.length()-x-1);

		    QTreeWidgetItem *item = new QTreeWidgetItem();
		    item->setText(0, n);
		    item->setText(1, type);
		    item->setText(2, WulforUtil::formatBytes(getSize(str)));

		    item->setData(0, Qt::UserRole, QVariant(str));

		    this->treeWidget->insertTopLevelItem(0, item);

		    QProgressBar *bar = new QProgressBar(this->treeWidget);
		    bar->setMinimum(0);
		    bar->setMaximum(100);
		    bar->setValue(getPersent(str));
		    this->treeWidget->setItemWidget(item, 3, bar);
		}
	    }
	}
    }
    catch (const Exception &e){
	//TODO: add error handling
    }
}

void UploadsFrame::createTree(DirectoryListing::Directory *dir, QStringList *list, QString dirName)
{
    if (!(dir && list))
	return;

    DirectoryListing::Directory::Iter it;
    DirectoryListing::File::Iter iter;
    for (iter = dir->files.begin(); iter != dir->files.end(); ++iter){
	QString name = QString::fromUtf8((*iter)->getName().c_str());
	if (name.length() < 13){
	    continue;
	}
	if (this->type_btn->getNumber() == 999){
            QList<int> keys = WulforUtil::getInstance()->getTypeKeys();
	    bool found = false;
            for (int i = 0; i < keys.count(); ++i){
                if (WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(keys[i]))[1] == name.right(12)){
		    found = true;
		    break;
		}
	    }
	    if (!found)
		continue;
	} else {
            if (WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(this->type_btn->getNumber()))[1] != name.right(12)){
		continue;
	    }
	}
	QString str = dirName + QDir::separator() + name;
	str.remove(0, 1);
	list->append(str);
    }


    for (it = dir->directories.begin(); it != dir->directories.end(); ++it)
	createTree(*it, list, dirName + QDir::separator() + QString::fromUtf8((*it)->getName().c_str()));
}

void UploadsFrame::closeEvent(QCloseEvent *e)
{
    if (isUnload()){
	MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
	MainWindow::getInstance()->remWidgetFromArena(this);
	MainWindow::getInstance()->remArenaWidget(this);

	//setAttribute(Qt::WA_DeleteOnClose);

	e->accept();
    }
    else {
	MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
	MainWindow::getInstance()->remWidgetFromArena(this);

	e->ignore();
    }
}

void UploadsFrame::on_treeWidget_doubleClicked(QModelIndex index)
{
    QModelIndex i = index.sibling(index.row(), 0);
    QString file = i.data(Qt::UserRole).toString();
    if (!QDmd::isValid(file)){
	QMessageBox::critical(this, tr("Error"), tr("Upload is damage"));
	return;
    }
    new ViewDMD(file, MainWindow::getInstance());
}

qint64 UploadsFrame::getSize(QString fileName)
{
    QList<file> list = TreeModel::getList(fileName);
    qint64 allS = 0;
    for (int i = 0; i < list.size(); ++i){
	allS += list.at(i).size;
    }
    return allS;
}

char UploadsFrame::getPersent(QString fileName)
{
    QList<file> list = TreeModel::getList(fileName);
    qint64 s = 0;
    qint64 allS = 0;
    for (int i = 0; i < list.size(); ++i){
	allS += list.at(i).size;
	if (ShareManager::getInstance()->isTTHShared(TTHValue(list[i].tth.toStdString()))){
	    s += list.at(i).size;
	}
    }
    return (char)((s*100)/allS);
}


void UploadsFrame::on_pushButton_clicked()
{
    CreateDMD *c = new CreateDMD(this);
    c->exec();
    if ( HashProgress::getHashStatus() == HashProgress::IDLE ) {
	ShareManager *SM = ShareManager::getInstance();
	SM->setDirty();
	SM->refresh(true);
    }
}

void UploadsFrame::loadDMD(QString file)
{
    if (!QDmd::isValid(file)){
        QMessageBox::critical(this, tr("Error"), tr("Upload is damage"));
        return;
    }
    new ViewDMD(file, MainWindow::getInstance());
}
