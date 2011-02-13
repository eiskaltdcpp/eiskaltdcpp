/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "createdmd.h"
#include "qdmd.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QTextBrowser>
#include <QComboBox>
#include <QPushButton>
#include <QDropEvent>
#include <QImage>
#include <QProgressBar>

#include "dcpp/HashManager.h"
#include "dcpp/ShareManager.h"

#include "WulforUtil.h"
#include "HashProgress.h"
#include "FileHasher.h"

#include "dcpp/File.h"

#include "getnamedmd.h"

#ifdef Q_WS_WIN
    #define nix_file "'"
#else
    #define nix_file "'file://"
#endif

CreateDMD::CreateDMD(QWidget *parent, Qt::WindowFlags f) :
    QDialog(parent, f)
{
    setupUi(this);
    TreeModel *model = new TreeModel();
    model->setEditable(true);
    delete treeView->model();

    Node *root = new Node("", "", 0, 0);

    Node *gr = new Node(tr("Folder"), "", 0, root);
    root->children.append(gr);

    model->setRootNode(root);
    treeView->setModel(model);
    treeView->setCurrentIndex(model->index(0, 0));

/*    types.clear();
    types[SearchManager::TYPE_VIDEO_FOREIGN_FILMS]	= QStringList() << tr("Foreign films")		<< tr(".fof_vid_dmd");
    types[SearchManager::TYPE_VIDEO_FOREIGN_SERIALS]	= QStringList() << tr("Foreign serials")	<< tr(".fos_vid_dmd");
    types[SearchManager::TYPE_VIDEO_FOREIGN_CARTOONS]	= QStringList() << tr("Foreign cartoons")	<< tr(".foc_vid_dmd");
    types[SearchManager::TYPE_VIDEO_OUR_FILMS]		= QStringList() << tr("Our films")		<< tr(".uof_vid_dmd");
    types[SearchManager::TYPE_VIDEO_OUR_SERIALS]	= QStringList() << tr("Our serials")		<< tr(".ous_vid_dmd");
    types[SearchManager::TYPE_VIDEO_OUR_CARTOONS]	= QStringList() << tr("Our cartoons")		<< tr(".ouc_vid_dmd");
    types[SearchManager::TYPE_VIDEO_TUTORIAL]		= QStringList() << tr("Video Tutorial")		<< tr(".tut_vid_dmd");
    types[SearchManager::TYPE_VIDEO_CLIPS]		= QStringList() << tr("Clips")			<< tr(".cli_vid_dmd");
    types[SearchManager::TYPE_VIDEO_CONCERTS]		= QStringList() << tr("Concerts")		<< tr(".con_vid_dmd");
    types[SearchManager::TYPE_VIDEO_KARAOKE]		= QStringList() << tr("Caraoke")		<< tr(".kar_vid_dmd");
    types[SearchManager::TYPE_VIDEO_HUMOR]		= QStringList() << tr("Humor video")		<< tr(".hum_vid_dmd");
    types[SearchManager::TYPE_VIDEO_ANIME]		= QStringList() << tr("Anime video")		<< tr(".ani_vid_dmd");
    types[SearchManager::TYPE_VIDEO_18]			= QStringList() << tr("Video 18+")		<< tr(".ero_vid_dmd");
    types[SearchManager::TYPE_VIDEO_OTHER]		= QStringList() << tr("Other video")		<< tr(".oth_vid_dmd");

    types[SearchManager::TYPE_AUDIO_ROCK]		= QStringList() << tr("Rock")		    << tr(".roc_aud_dmd");
    types[SearchManager::TYPE_AUDIO_POP]		= QStringList() << tr("Pop")		    << tr(".pop_aud_dmd");
    types[SearchManager::TYPE_AUDIO_JAZZ]		= QStringList() << tr("Jazz")		    << tr(".jaz_aud_dmd");
    types[SearchManager::TYPE_AUDIO_BLUES]		= QStringList() << tr("Blues")		    << tr(".blu_aud_dmd");
    types[SearchManager::TYPE_AUDIO_ELECTRONIC]		= QStringList() << tr("Electronic music")   << tr(".ele_aud_dmd");
    types[SearchManager::TYPE_AUDIO_SOUNDTRACKS]	= QStringList() << tr("Soundtracks")	    << tr(".sou_aud_dmd");
    types[SearchManager::TYPE_AUDIO_NOTES]		= QStringList() << tr("Notes")		    << tr(".not_aud_dmd");
    types[SearchManager::TYPE_AUDIO_AUDIOBOOKS]		= QStringList() << tr("Audiobooks")	    << tr(".aud_aud_dmd");
    types[SearchManager::TYPE_AUDIO_OTHER]		= QStringList() << tr("Other music")	    << tr(".oth_aud_dmd");

    types[SearchManager::TYPE_IMAGE_AVATARS]	    = QStringList() << tr("Avatars")		<< tr(".ava_img_dmd");
    types[SearchManager::TYPE_IMAGE_WALLPAPERS]	    = QStringList() << tr("Wallpapers")		<< tr(".wal_img_dmd");
    types[SearchManager::TYPE_IMAGE_ANIME]	    = QStringList() << tr("Anime images")	<< tr(".ani_img_dmd");
    types[SearchManager::TYPE_IMAGE_NATURE]	    = QStringList() << tr("Nature")		<< tr(".nat_img_dmd");
    types[SearchManager::TYPE_IMAGE_ANIMALS]	    = QStringList() << tr("Animals")		<< tr(".mal_img_dmd");
    types[SearchManager::TYPE_IMAGE_CARS]	    = QStringList() << tr("Cars and moto")	<< tr(".car_img_dmd");
    types[SearchManager::TYPE_IMAGE_HUMOR]	    = QStringList() << tr("Humor images")	<< tr(".hum_img_dmd");
    types[SearchManager::TYPE_IMAGE_SPORT]	    = QStringList() << tr("Sport")		<< tr(".spo_img_dmd");
    types[SearchManager::TYPE_IMAGE_SCREENSHOTS]    = QStringList() << tr("Screenshots")	<< tr(".scr_img_dmd");
    types[SearchManager::TYPE_IMAGE_ARTISTIC]	    = QStringList() << tr("Artistic photo")	<< tr(".art_img_dmd");
    types[SearchManager::TYPE_IMAGE_ABSTRACT]	    = QStringList() << tr("Abstract")		<< tr(".abs_img_dmd");
    types[SearchManager::TYPE_IMAGE_18]		    = QStringList() << tr("Images 18+")		<< tr(".ero_img_dmd");
    types[SearchManager::TYPE_IMAGE_OTHER]	    = QStringList() << tr("Other images")	<< tr(".oth_img_dmd");

    types[SearchManager::TYPE_GAMES_ACTION]	    = QStringList() << tr("Action")	    << tr(".act_gam_dmd");
    types[SearchManager::TYPE_GAMES_RPG]	    = QStringList() << tr("RPG")	    << tr(".rpg_gam_dmd");
    types[SearchManager::TYPE_GAMES_STRATEGY]	    = QStringList() << tr("Strategy")	    << tr(".str_gam_dmd");
    types[SearchManager::TYPE_GAMES_SIM]	    = QStringList() << tr("Sim")	    << tr(".sim_gam_dmd");
    types[SearchManager::TYPE_GAMES_QUESTS]	    = QStringList() << tr("Quests")	    << tr(".que_gam_dmd");
    types[SearchManager::TYPE_GAMES_ARCADE]	    = QStringList() << tr("Arcade")	    << tr(".arc_gam_dmd");
    types[SearchManager::TYPE_GAMES_MINI]	    = QStringList() << tr("Mini-games")	    << tr(".min_gam_dmd");
    types[SearchManager::TYPE_GAMES_CONSOLES]	    = QStringList() << tr("For consoles")   << tr(".con_gam_dmd");
    types[SearchManager::TYPE_GAMES_LINUX]	    = QStringList() << tr("For Linux")	    << tr(".lin_gam_dmd");
    types[SearchManager::TYPE_GAMES_MAC]	    = QStringList() << tr("For Mac")	    << tr(".mac_gam_dmd");
    types[SearchManager::TYPE_GAMES_ADD_ON]	    = QStringList() << tr("Add-ons")	    << tr(".add_gam_dmd");
    types[SearchManager::TYPE_GAMES_OTHER]	    = QStringList() << tr("Other games")    << tr(".oth_gam_dmd");

    types[SearchManager::TYPE_SOFT_SYSTEM]	= QStringList() << tr("System soft")		<< tr(".sys_sof_dmd");
    types[SearchManager::TYPE_SOFT_ANTI_VIR]	= QStringList() << tr("Anti-viruses")		<< tr(".avr_sof_dmd");
    types[SearchManager::TYPE_SOFT_AUDIO]	= QStringList() << tr("Audio soft")		<< tr(".aud_sof_dmd");
    types[SearchManager::TYPE_SOFT_VIDEO]	= QStringList() << tr("Video soft")		<< tr(".vid_sof_dmd");
    types[SearchManager::TYPE_SOFT_GRAPHIC]	= QStringList() << tr("Graphic soft")		<< tr(".gra_sof_dmd");
    types[SearchManager::TYPE_SOFT_OFFICE]	= QStringList() << tr("Office soft")		<< tr(".off_sof_dmd");
    types[SearchManager::TYPE_SOFT_DRIVERS]	= QStringList() << tr("Drivers and frimware")	<< tr(".drv_sof_dmd");
    types[SearchManager::TYPE_SOFT_DEVELOP]	= QStringList() << tr("Develop")		<< tr(".dev_sof_dmd");
    types[SearchManager::TYPE_SOFT_DIRECTORIES]	= QStringList() << tr("Directories and gis")	<< tr(".dir_sof_dmd");
    types[SearchManager::TYPE_SOFT_OTHER]	= QStringList() << tr("Other soft")		<< tr(".oth_sof_dmd");

    types[SearchManager::TYPE_BOOK_HUMOR]	= QStringList() << tr("Humor")		<< tr(".hum_boo_dmd");
    types[SearchManager::TYPE_BOOK_MANGA]	= QStringList() << tr("Manga")		<< tr(".man_boo_dmd");
    types[SearchManager::TYPE_BOOK_TUTORIAL]	= QStringList() << tr("Tutorial")	<< tr(".tut_boo_dmd");
    types[SearchManager::TYPE_BOOK_FEATURE]	= QStringList() << tr("Feature book")   << tr(".fea_boo_dmd");
    types[SearchManager::TYPE_BOOK_TECHNIKAL]	= QStringList() << tr("Technikal")	<< tr(".tec_boo_dmd");
    types[SearchManager::TYPE_BOOK_OTHER]	= QStringList() << tr("Other books")	<< tr(".oth_boo_dmd");

    types[SearchManager::TYPE_UP_OTHER]	    = QStringList() << tr("Other uploads")   << tr("oth_dmd");*/

    this->type_btn = new QTypeContentButton(false, this);
    ((QHBoxLayout *)horizontalLayout_6)->insertWidget(0, this->type_btn);
    listWidget->installEventFilter(this);

    this->pushButton_10->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiBOOKMARK_ADD));
    this->pushButton_11->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE));

    this->pushButton->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWN));
    this->pushButton_2->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiUP));
}

CreateDMD::~CreateDMD()
{
    delete type_btn;
}

void CreateDMD::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
	retranslateUi(this);
        break;
    default:
        break;
    }
}

void CreateDMD::on_pushButton_8_clicked()
{
    if (listWidget->count() == 0){
	QMessageBox::critical(this, tr("Error"), tr("Cover not selected"));
	return;
    }
    QPixmap pix;
    pix.load(listWidget->item(0)->data(Qt::UserRole).toString());

    QString w = QString("%1").arg((pix.width() < 256 ? pix.width() : 256));

    QString html = "<!DOCTYPE HTML PUBLIC  '-//W3C//DTD HTML 4.01 Transitional//EN' 'http://www.w3.org/TR/html4/loose.dtd'>"
		   "<HTML>"
		   "<HEAD>"
		   "<meta http-equiv='content-type' content='text/html'; charset='UTF-8'>"
		   "<meta http-equiv='content-style-type' content='text/css'>"
		   "<title>Upload</title>"
		   "</HEAD>"
		   "<BODY>"
		   //"<table border=0 cellpadding=2 align=right><tr><td><img src="+tr(nix_file)+f1.fileName()+".png' width=256></td></tr></table><p align=justify>";
                   "<img src="+tr(nix_file)+listWidget->item(0)->data(Qt::UserRole).toString()+"' width="+w+" align=right><p align=justify>";
    html += textEdit->toPlainText();
    html += "</BODY>"
	    "</HTML>";

    QDialog v(this);
    v.setMinimumSize(600, 400);
    QVBoxLayout main(&v);

    QTextBrowser browse(&v);
    browse.setHtml(html);

    QDialogButtonBox q(Qt::Horizontal);

    main.addWidget(&browse);
    main.addWidget(&q);

    q.addButton(tr("Ok"), QDialogButtonBox::RejectRole);

    v.setWindowTitle(tr("Preview"));

    QObject::connect(&q, SIGNAL(rejected()), &v, SLOT(reject()));

    v.exec();
}

void CreateDMD::on_pushButton_6_clicked()
{
    if (listWidget->count() == 0){
        QMessageBox::critical(this, tr("Error"), tr("Cover not selected"));
        return;
    }
    if (this->type_btn->getNumber() == 999){
        QMessageBox::information(this, tr("Error"), tr("Select type"));
        return;
    }

    QDmd dmd;
    TreeModel *m = (TreeModel *)treeView->model();

    m->delEmpty();
    if (m->getRoot()->children.count() == 0){
	Node *gr = new Node(tr("Folder"), "", 0, m->getRoot());
	m->getRoot()->children.append(gr);
    }
    treeView->reset();
    treeView->setCurrentIndex(m->index(0, 0));

    if (m->getRoot()->children[0]->children.count() == 0){
	QMessageBox::critical(this, tr("Error"), tr("You don't add files!"));
	return;
    }

    QImage cover;
    if (!cover.load(listWidget->item(0)->data(Qt::UserRole).toString())){
        QMessageBox::critical(this, tr("Error"), tr("Don't load cover file\n%1").arg(listWidget->item(0)->data(Qt::UserRole).toString()));
        return;
    }
    QList<QImage> images;
    QImage img;
    for (int i = 1; i < listWidget->count(); ++i){
        if (!img.load(listWidget->item(i)->data(Qt::UserRole).toString())){
            QMessageBox::critical(this, tr("Error"), tr("Don't load cover file\n%1").arg(listWidget->item(i)->data(Qt::UserRole).toString()));
            return;
        }
        images << img;
    }
    dmd.create(textEdit->toPlainText(), m, cover, images);

    QStringList shares;
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    StringPairList::iterator it;

    for (it = directories.begin(); it != directories.end(); ++it){
	shares << QString::fromStdString((*it).second);
    }

    if (shares.count() == 0){
	QMessageBox::critical(this, tr("Error"), tr("You don't have shared foldels"));
	return;
    }
    getNameDMD *d = new getNameDMD(WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(this->type_btn->getNumber()))[1], shares, this);
    d->exec();
    if (!d->isOk())
	return;
    QString fileName = d->getFileName();

    if (fileName.right(12) != WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(this->type_btn->getNumber()))[1]){
        fileName += WulforUtil::getInstance()->getTypeData(static_cast<dcpp::SearchManager::TypeModes>(this->type_btn->getNumber()))[1];
    }
    dmd.saveFile(fileName);
    this->close();
}

//Кнопка "Добавить папку"
void CreateDMD::on_pushButton_9_clicked()
{
    static int folderName;
    ++folderName;
    QModelIndex index = treeView->currentIndex();
    index.sibling(index.row(), 0);
    ((TreeModel *)treeView->model())->addElement(index, tr("New folder %1").arg(folderName), "", 0);
    treeView->reset();
    treeView->setCurrentIndex(index);
    treeView->expand(index);
    while (index.parent().isValid()){
	treeView->expand(index);
	index = index.parent();
    }
}

void CreateDMD::on_pushButton_5_clicked()
{
    QModelIndex index = treeView->currentIndex();
    QModelIndex pIndex = index.parent();
    index.sibling(index.row(), 0);
    if (((TreeModel *)treeView->model())->delElement(index)){
	treeView->reset();
	index = pIndex;
	treeView->setCurrentIndex(index);
	treeView->expand(index);
	while (index.parent().isValid()){
	    treeView->expand(index);
	    index = index.parent();
	}
    }
}

void CreateDMD::on_pushButton_4_clicked()
{
    QDialog stat(this);
    stat.setWindowTitle(tr("Copy file to shares"));
    QVBoxLayout lay(&stat);
    QLabel l(&stat);
    QProgressBar p(&stat);
    p.setMinimum(0);
    p.setMaximum(100);
    p.setValue(0);
    stat.setLayout(&lay);
    lay.addWidget(&l);
    lay.addWidget(&p);

    QStringList shares;
    StringPairList directories = ShareManager::getInstance()->getDirectories();
    StringPairList::iterator it;

    for (it = directories.begin(); it != directories.end(); ++it){
	shares << QString::fromStdString((*it).second);
    }

    if (shares.count() == 0){
	QMessageBox::critical(this, tr("Error"), tr("You don't have shared foldels"));
	return;
    }

    SettingsManager *SM = SettingsManager::getInstance();
    QString downDir = QString::fromStdString(SM->get(SettingsManager::DOWNLOAD_DIRECTORY, false));
    QStringList str = QFileDialog::getOpenFileNames(this, tr("Add file"), downDir, "All files (*.*)");
    if (str.count() == 0){
	return;
    }

    QString allCopy = "ASK";
    QString copyTo = "";

    for (int i = 0; i < str.count(); ++i){
        QFileInfo fi(str[i]);
        str[i] = QDir::toNativeSeparators(fi.absoluteFilePath());
        const TTHValue *tth = HashManager::getInstance()->getFileTTHif(str[i].toStdString());
	if (tth == NULL){ //Если файл не найден в расшаренных пробуем преобразовать пути
            str[i] = QDir::toNativeSeparators( fi.canonicalFilePath() ); // try to follow symlinks
            tth = HashManager::getInstance()->getFileTTHif(str[i].toStdString());
        }
	if ( tth != NULL ) { //Если файл всеже найден - добавляем
            QModelIndex index = treeView->currentIndex();
            index = index.sibling(index.row(), 1);
            if (index.data(Qt::DisplayRole).toString().length() != 0){
                index = index.parent();
            }
            index = index.sibling(index.row(), 0);
            ((TreeModel *)treeView->model())->addElement(index, fi.fileName(), _q(tth->toBase32()), fi.size());
	} else { //А если не расшарен
            stat.show();
	    QString newName = fi.fileName();
	    if (allCopy == "ASK"){//Установлено действие - спросить
                QMessageBox *d = new QMessageBox(this);//Создаем диалог с вопросом
		d->setIcon(QMessageBox::Question);
		d->addButton(QMessageBox::Yes);
		d->addButton(QMessageBox::YesToAll);
		d->addButton(QMessageBox::No);
		d->addButton(QMessageBox::NoToAll);
		d->setWindowTitle(tr("Copy?"));
		d->setText(tr("File\n\n%1\n\nis not shared. Copy to shared folder?").arg(fi.absoluteFilePath()));
		d->exec();
		QMessageBox::StandardButton rez = d->standardButton(d->clickedButton());
		delete d;
		if ((rez == QMessageBox::Yes) ||  //Ответ - копировать или копировать все
		    (rez == QMessageBox::YesToAll)){
		    if (shares.count() == 1){ //Определяем папку для копирования
			copyTo = shares[0]; //Если папка одна - выбираем ее
		    } else {
			QDialog *f = new QDialog(this); //Если несколько - выводим диалог выбора
			f->setWindowTitle(tr("Shares"));
			f->setMaximumWidth(400);
			QVBoxLayout *mLay = new QVBoxLayout(f);

			QLabel *l = new QLabel(f);
			l->setText(tr("Select shares for copy"));

			QComboBox *box = new QComboBox(f);
			box->addItems(shares);
			box->setCurrentIndex(0);

			QHBoxLayout *bLay = new QHBoxLayout(f);
			QSpacerItem *s = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			QPushButton *p = new QPushButton(f);
			p->setText(tr("Ok"));
			connect(p, SIGNAL(clicked()), f, SLOT(accept()));

			bLay->addSpacerItem(s);
			bLay->addWidget(p);

			mLay->addWidget(l);
			mLay->addWidget(box);
			mLay->addLayout(bLay);

			f->exec();
			copyTo = box->currentText(); //И запиминаем выбранную папку
			delete f;
		    }
                    if (!QDir(copyTo).exists()){
                        QMessageBox::critical(this, tr("Error"), tr("Folder for copy not exists!"));
                        return;
                    }
		    int n = 1;    //При переименовании файла (если потребуется) ищем точку в имени
		    int pos = fi.fileName().indexOf('.');
		    if (pos == -1){ //Если её нет - устанавливаем в конец файла
			pos = fi.fileName().length();
		    }
		    while (QFile::exists(copyTo+newName)){ //Ищем свободное имя для файла
			newName = fi.fileName();
			newName.insert(pos, QString("(%1)").arg(n));
			n++;
		    }
		    qApp->processEvents();
		    QString sTTH = "";
		    HashManager *HM = HashManager::getInstance();
		    const TTHValue *tth= HM->getFileTTHif(_tq(fi.absoluteFilePath()));
		    if (tth != NULL) {
			sTTH = _q(tth->toBase32());
		    } else {
			quint64 MIN_BLOCK_SIZE = 64 * 1024;
			size_t BUF_SIZE = 64*1024;

			char TTH[40] = {0};
			char *buf = new char[BUF_SIZE];

			memset(buf, 0, BUF_SIZE);

			try {
			    File f(fi.absoluteFilePath().toStdString(), File::READ, File::OPEN);
			    File fOUT((copyTo+newName).toStdString(), File::WRITE, File::CREATE);
			    TigerTree tth(max(TigerTree::calcBlockSize(f.getSize(), 10), static_cast<int64_t>(MIN_BLOCK_SIZE)));
			    if(f.getSize() > 0) {
                                p.setValue(0);
                                p.setMaximum(f.getSize());
                                l.setText(tr("Copy and calculated hash '%1'").arg(fi.fileName()));
                                stat.show();
				size_t n = BUF_SIZE;
				int x = 0;
				while( (n = f.read(&buf[0], n)) > 0) {
				    if (x % 10 == 0){
                                        p.setValue(f.getPos());
					qApp->processEvents();
				    }
				    ++x;
				    tth.update(&buf[0], n);
				    fOUT.write(&buf[0], n);
				    n = BUF_SIZE;
				}
                                stat.hide();
			    } else {
				tth.update("", 0);
			    }
			    tth.finalize();
			    strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
			    sTTH = _q(TTH);
			    fOUT.close();
			    f.close();
                            l.setText(tr("Copyeng complite"));

                            //И вставляем ссылку на файл
                            QModelIndex index = treeView->currentIndex();
                            index = index.sibling(index.row(), 1);
                            if (index.data(Qt::DisplayRole).toString().length() != 0){
                                index = index.parent();
                            }
                            index = index.sibling(index.row(), 0);
                            ((TreeModel *)treeView->model())->addElement(index, fi.fileName(), sTTH, fi.size());
                        } catch (...) {
			    File::deleteFile((copyTo+newName).toStdString());
                            QMessageBox::critical(this, tr("Error"), tr("Can't copy file \n'%1'").arg(fi.fileName()));
			}
                        p.setValue(0);

			delete buf;
		    }

		    if (rez == QMessageBox::YesToAll){ //Если выбрано - копировать все
			allCopy = "COPY"; //Устанавливаем действие - копировать все
		    }
		}
		if (rez == QMessageBox::NoToAll){ //Если выбрано - Игнорировать все
		    allCopy = "IGNORE"; //Устанавливаем действие - Игнорировать все
		}
		continue;
	    }
	    //Если задано действие - копировать все
	    if (allCopy == "COPY"){
		int n = 1; //Копируем файлв в выбранную ранее директорию, хешируем и добавляем ссылку
		int pos = -1;
		for (int i = 0; i < fi.fileName().length(); ++i){
		    if (fi.fileName()[i] == QChar('.')){
			pos = i;
		    }
		}
		if (pos == -1){
		    pos = fi.fileName().length();
		}
		while (QFile::exists(copyTo+newName)){
		    newName = fi.fileName();
		    newName.insert(pos, QString("(%1)").arg(n));
		    n++;
		}
		qApp->processEvents();
		QString sTTH = "";
		HashManager *HM = HashManager::getInstance();
		const TTHValue *tth= HM->getFileTTHif(_tq(fi.absoluteFilePath()));
		if (tth != NULL) {
		    sTTH = _q(tth->toBase32());
		} else {
		    quint64 MIN_BLOCK_SIZE = 64 * 1024;
		    size_t BUF_SIZE = 64*1024;

		    char TTH[40] = {0};
		    char *buf = new char[BUF_SIZE];

		    memset(buf, 0, BUF_SIZE);

		    try {
			File f(fi.absoluteFilePath().toStdString(), File::READ, File::OPEN);
			File fOUT((copyTo+newName).toStdString(), File::WRITE, File::CREATE);
			TigerTree tth(max(TigerTree::calcBlockSize(f.getSize(), 10), static_cast<int64_t>(MIN_BLOCK_SIZE)));
			if(f.getSize() > 0) {
                            p.setValue(0);
                            p.setMaximum(f.getSize());
                            l.setText(tr("Copy and calculated hash '%1'").arg(fi.fileName()));
                            stat.show();
			    size_t n = BUF_SIZE;
			    int x = 0;
			    while( (n = f.read(&buf[0], n)) > 0) {
				if (x % 10 == 0){
                                    p.setValue(f.getPos());
				    qApp->processEvents();
				}
				tth.update(&buf[0], n);
				fOUT.write(&buf[0], n);
				n = BUF_SIZE;
			    }
			} else {
			    tth.update("", 0);
			}
			tth.finalize();
			strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
			sTTH = _q(TTH);
			fOUT.close();
			f.close();
                        l.setText(tr("Copyeng complite"));
                        delete buf;
                        QModelIndex index = treeView->currentIndex();
                        index = index.sibling(index.row(), 1);
                        if (index.data(Qt::DisplayRole).toString().length() != 0){
                            index = index.parent();
                        }
                        index = index.sibling(index.row(), 0);
                        ((TreeModel *)treeView->model())->addElement(index, fi.fileName(), sTTH, fi.size());
                    } catch (...) {
			File::deleteFile((copyTo+newName).toStdString());
                        QMessageBox::critical(this, tr("Error"), tr("Can't copy file \n'%1'").arg(fi.fileName()));
		    }

                    p.setValue(0);
		}
	    }
	}
    }
    //Перерисовываем дерево.
    QModelIndex index = treeView->currentIndex();
    index = index.sibling(index.row(), 1);
    if (index.data(Qt::DisplayRole).toString().length() != 0){
        index = index.parent();
    }
    index = index.sibling(index.row(), 0);
    treeView->reset();
    treeView->setCurrentIndex(index);
    treeView->expand(index);
    while (index.parent().isValid()){
        treeView->expand(index);
        index = index.parent();
    }
}

void CreateDMD::on_pushButton_10_clicked()
{
    QStringList str;
    str = QFileDialog::getOpenFileNames(this, tr("Choose image"), "", tr("Images (*.jpg *.jpeg *.png)"));
    if (str.count() == 0){
        return;
    }
    QStringList errors;
    for (int i = 0; i< str.count(); ++i){
        QFileInfo finf(str[i]);
        QPixmap pix;
        if (!pix.load(str[i])){
            errors << finf.fileName();
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(listWidget);
        item->setText(finf.fileName());
        item->setIcon(QIcon(pix.scaled(64, 64)));
        item->setData(Qt::UserRole, finf.absoluteFilePath());

        this->listWidget->addItem(item);
	if (listWidget->count() >= 4){
	    this->pushButton_10->setEnabled(false);
	    break;
	}
    }
    if (errors.count() != 0){
        QMessageBox::critical(this, tr("Error"), tr("Error load images\n%1").arg(errors.join("\n")));
    }
}

void CreateDMD::on_pushButton_11_clicked()
{
    delete listWidget->item(listWidget->currentRow());
    this->pushButton_10->setEnabled(listWidget->count() < 4);
}

void CreateDMD::on_pushButton_2_clicked()
{
    int row = listWidget->currentRow();
    if (row == 0){
        return;
    }
    QString str = listWidget->item(row)->text();
    QString pix = listWidget->item(row)->data(Qt::UserRole).toString();
    QIcon icon = listWidget->item(row)->icon();

    listWidget->item(row)->setText(listWidget->item(row-1)->text());
    listWidget->item(row)->setData(Qt::UserRole, listWidget->item(row-1)->data(Qt::UserRole).toString());
    listWidget->item(row)->setIcon(listWidget->item(row-1)->icon());

    listWidget->item(row-1)->setText(str);
    listWidget->item(row-1)->setData(Qt::UserRole, pix);
    listWidget->item(row-1)->setIcon(icon);

    listWidget->setCurrentRow(row-1);
}

void CreateDMD::on_pushButton_clicked()
{
    int row = listWidget->currentRow();
    if (row == listWidget->count()-1){
        return;
    }
    QString str = listWidget->item(row)->text();
    QString pix = listWidget->item(row)->data(Qt::UserRole).toString();
    QIcon icon = listWidget->item(row)->icon();

    listWidget->item(row)->setText(listWidget->item(row+1)->text());
    listWidget->item(row)->setData(Qt::UserRole, listWidget->item(row+1)->data(Qt::UserRole).toString());
    listWidget->item(row)->setIcon(listWidget->item(row+1)->icon());

    listWidget->item(row+1)->setText(str);
    listWidget->item(row+1)->setData(Qt::UserRole, pix);
    listWidget->item(row+1)->setIcon(icon);

    listWidget->setCurrentRow(row+1);
}
