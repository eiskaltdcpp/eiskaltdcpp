/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qtypecontentbutton.h"
#include "WulforUtil.h"
//#include <QtDebug>

QTypeContentButton::QTypeContentButton(bool all, QWidget *parent):QPushButton(parent),all(all)
{
    QList<WulforUtil::Icons> icons;
    icons   << WulforUtil::eiFILETYPE_UNKNOWN  << WulforUtil::eiFILETYPE_MP3         << WulforUtil::eiFILETYPE_ARCHIVE
            << WulforUtil::eiFILETYPE_DOCUMENT << WulforUtil::eiFILETYPE_APPLICATION << WulforUtil::eiFILETYPE_PICTURE
            << WulforUtil::eiFILETYPE_VIDEO    << WulforUtil::eiFOLDER_BLUE          << WulforUtil::eiFIND;

    QMenu *menuMain = new QMenu(this);
    QStringList list;
    QAction *act;

    QMenu *uploads;
    if (all){
	uploads= new QMenu(menuMain);
	uploads->setTitle(tr("Uploads"));		//Раздачи
	uploads->menuAction()->setIconVisibleInMenu(true);
	uploads->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWN)));
	menuMain->addMenu(uploads);
    } else {
	act = new QAction(menuMain);
	act->setText(tr("All uploads"));		//Все раздачи
	act->setData(999);
	act->setIconVisibleInMenu(true);
	act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILETYPE_UNKNOWN)));
	menuMain->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
	this->setText(act->text());
	this->setIcon(act->icon());
	this->typeContent = act->data().toInt();
    }

    QMenu *menuVideo;
    if (all){
	menuVideo = new QMenu(uploads);
    } else {
	menuVideo = new QMenu(menuMain);
    }
    menuVideo->setTitle(tr("Video"));		//Видео
    menuVideo->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(6))));
    menuVideo->menuAction()->setIconVisibleInMenu(true);
    list.clear();
    //list.append(tr("All video"));		//Всё видео
    list.append(tr("Foreign films"));		//Зарубежные фильмы
    list.append(tr("Foreign serials"));		//Зарубежные сериалы
    list.append(tr("Foreign cartoons"));	//Зарубежные мультфильмы
    list.append(tr("Our films"));		//Наши фильмы
    list.append(tr("Our serials"));		//Наши сериалы
    list.append(tr("Our cartoons"));		//Наши мультфильмы
    list.append(tr("Video Tutorial"));		//Видеоучебники
    list.append(tr("Clips"));			//Клипы
    list.append(tr("Concerts"));		//Концерты
    list.append(tr("Karaoke"));			//Караоке
    list.append(tr("Humor video"));		//Юмор - видео
    list.append(tr("Anime video"));		//Аниме видео

    for (int i = 0; i<list.count(); ++i){
	act = new QAction(menuVideo);
	act->setText(list[i]);
        act->setData(i+102);
        act->setIconVisibleInMenu(true);
        act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(6))));
	menuVideo->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
    }
    act = new QAction(menuVideo);
    act->setText(tr("Video 18+"));		//Видео 18+
    act->setData(198);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(6))));
    menuVideo->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    act = new QAction(menuVideo);
    act->setText(tr("Other video"));		//Другое видео
    act->setData(199);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(6))));
    menuVideo->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	uploads->addMenu(menuVideo);
    } else {
	menuMain->addMenu(menuVideo);
    }


    QMenu *menuAudio;
    if (all){
	menuAudio = new QMenu(uploads);
    } else {
	menuAudio = new QMenu(menuMain);
    }
    menuAudio->setTitle(tr("Music"));		//Музыка
    menuAudio->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(1))));
    menuAudio->menuAction()->setIconVisibleInMenu(true);
    list.clear();
    //list.append(tr("All music"));		//Вся музыка
    list.append(tr("Rock"));			//Рок
    list.append(tr("Pop"));			//Поп
    list.append(tr("Jazz"));			//Джаз
    list.append(tr("Blues"));			//Блюз
    list.append(tr("Electronic music"));	//Электронная музыка
    list.append(tr("Soundtracks"));		//Саундтреки
    list.append(tr("Notes"));			//Ноты
    list.append(tr("Audiobooks"));		//Аудиокниги

    for (int i = 0; i<list.count(); ++i){
	act = new QAction(menuAudio);
	act->setText(list[i]);
        act->setData(i+202);
        act->setIconVisibleInMenu(true);
        act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(1))));
	menuAudio->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
    }
    act = new QAction(menuAudio);
    act->setText(tr("Other music"));		//Другая музыка
    act->setData(299);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(1))));
    menuAudio->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	uploads->addMenu(menuAudio);
    } else {
	menuMain->addMenu(menuAudio);
    }


    QMenu *menuImages;
    if (all){
	menuImages = new QMenu(uploads);
    } else {
	menuImages = new QMenu(menuMain);
    }
    menuImages->setTitle(tr("Images"));		//Картинки
    menuImages->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(5))));
    menuImages->menuAction()->setIconVisibleInMenu(true);
    list.clear();
    //list.append(tr("All images"));		//Всё картинки
    list.append(tr("Avatars"));			//Аватары
    list.append(tr("Wallpapers"));		//Обои
    list.append(tr("Anime images"));		//Аниме картинки
    list.append(tr("Nature"));			//Природа
    list.append(tr("Animals"));			//Животные
    list.append(tr("Cars and moto"));		//Машины и мотоциклы
    list.append(tr("Humor images"));		//Юмор - катинки
    list.append(tr("Sport"));			//Спорт
    list.append(tr("Screenshots"));		//Скриншоты
    list.append(tr("Artistic photo"));		//Художественное фото
    list.append(tr("Abstract"));		//Абстрактные

    for (int i = 0; i<list.count(); ++i){
	act = new QAction(menuImages);
	act->setText(list[i]);
        act->setData(i+302);
        act->setIconVisibleInMenu(true);
        act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(5))));
	menuImages->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
    }
    act = new QAction(menuImages);
    act->setText(tr("Images 18+"));		//Картинки 18+
    act->setData(398);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(5))));
    menuImages->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    act = new QAction(menuImages);
    act->setText(tr("Other images"));		//Другие картинки
    act->setData(399);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(5))));
    menuImages->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	uploads->addMenu(menuImages);
    } else {
	menuMain->addMenu(menuImages);
    }


    QMenu *menuGames;
    if (all){
	menuGames = new QMenu(uploads);
    } else {
	menuGames = new QMenu(menuMain);
    }
    menuGames->setTitle(tr("Games"));		//Игры
    menuGames->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(4))));
    menuGames->menuAction()->setIconVisibleInMenu(true);
    list.clear();
    //list.append(tr("All games"));		//Все игры
    list.append(tr("Action"));			//Экшн
    list.append(tr("RPG"));			//РПГ
    list.append(tr("Strategy"));		//Стратегии
    list.append(tr("Sim"));			//Симуляторы
    list.append(tr("Quests"));			//Квесты
    list.append(tr("Arcade"));			//Аркады
    list.append(tr("Mini-games"));		//Мини-игры
    list.append(tr("For consoles"));		//Для консолей
    list.append(tr("For Linux"));		//Для Linux
    list.append(tr("For Mac"));			//Для Mac
    list.append(tr("Add-ons"));			//Дополнения

    for (int i = 0; i<list.count(); ++i){
	act = new QAction(menuGames);
	act->setText(list[i]);
        act->setData(i+402);
        act->setIconVisibleInMenu(true);
        act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(4))));
	menuGames->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
    }
    act = new QAction(menuGames);
    act->setText(tr("Other games"));		//Другие игры
    act->setData(499);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(4))));
    menuGames->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	uploads->addMenu(menuGames);
    } else {
	menuMain->addMenu(menuGames);
    }


    QMenu *menuSoft;
    if (all){
	menuSoft = new QMenu(uploads);
    } else {
	menuSoft = new QMenu(menuMain);
    }
    menuSoft->setTitle(tr("Soft"));		//Программы
    menuSoft->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(4))));
    menuSoft->menuAction()->setIconVisibleInMenu(true);
    list.clear();
    //list.append(tr("All soft"));		//Все программы
    list.append(tr("System soft"));		//Системный софт
    list.append(tr("Anti-viruses"));		//Антивирусы
    list.append(tr("Audio soft"));		//Аудио софт
    list.append(tr("Video soft"));		//Видео софт
    list.append(tr("Graphic soft"));		//Работа с графикой
    list.append(tr("Office soft"));		//Офисный софт
    list.append(tr("Drivers and frimware"));	//Драйверы и прошивки
    list.append(tr("Develop"));			//Программирование
    list.append(tr("Directories and gis"));	//Справочники и карты

    for (int i = 0; i<list.count(); ++i){
	act = new QAction(menuSoft);
	act->setText(list[i]);
        act->setData(i+502);
        act->setIconVisibleInMenu(true);
        act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(4))));
        menuSoft->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
    }
    act = new QAction(menuSoft);
    act->setText(tr("Other soft"));		//Другие программы
    act->setData(599);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(4))));
    menuSoft->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	uploads->addMenu(menuSoft);
    } else {
	menuMain->addMenu(menuSoft);
    }


    QMenu *menuBook;
    if (all){
	menuBook = new QMenu(uploads);
    } else {
	menuBook = new QMenu(menuMain);
    }
    menuBook->setTitle(tr("Books"));		//Книги
    menuBook->menuAction()->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(3))));
    menuBook->menuAction()->setIconVisibleInMenu(true);
    list.clear();
    //list.append(tr("All books"));		//Все книги
    list.append(tr("Humor"));			//Юмор - книги
    list.append(tr("Manga"));			//Манга
    list.append(tr("Tutorial"));		//Учебники
    list.append(tr("Feature book"));		//Художественные
    list.append(tr("Technikal"));		//Технические

    for (int i = 0; i<list.count(); ++i){
	act = new QAction(menuBook);
	act->setText(list[i]);
        act->setData(i+602);
        act->setIconVisibleInMenu(true);
        act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(3))));
	menuBook->addAction(act);
	connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
    }
    act = new QAction(menuBook);
    act->setText(tr("Other books"));		//Другие книги
    act->setData(699);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(3))));
    menuBook->addAction(act);
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	uploads->addMenu(menuBook);
    } else {
	menuMain->addMenu(menuBook);
    }

    if (all){
	act = new QAction(uploads);
    } else {
	act = new QAction(menuMain);
    }
    act->setText(tr("Other uploads"));		//Разные раздачи
    act->setData(998);
    act->setIconVisibleInMenu(true);
    act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(0))));
    if (all){
	uploads->addAction(act);
    } else {
	menuMain->addAction(act);
    }
    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));

    if (all){
	list.clear();
	list.append(tr("Any"));			//Любой
	list.append(tr("Audio"));			//Аудио
	list.append(tr("Archive"));			//Архив
	list.append(tr("Document"));		//Документ
	list.append(tr("Executable"));		//Исполняемый
	list.append(tr("Images"));			//Изображение
	list.append(tr("Video"));			//Видео
	list.append(tr("Folder"));
	list.append(tr("TTH"));
	list.append(tr("CD Image"));

	for (int i = 0; i<list.count(); ++i){
	    act = new QAction(menuMain);
	    act->setIconVisibleInMenu(true);
	    act->setIconText(list[i]);
	    //qDebug() << i ;
	    if (i != 9)
		act->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(i))));
	    act->setData(i);
	    menuMain->addAction(act);
	    connect(act, SIGNAL(triggered(bool)), this, SLOT(actionClick()));
	}
    }

    this->setMenu(menuMain);


    if (all){
	this->setText(list[0]);
	this->setIcon(QIcon(WulforUtil::getInstance()->getPixmap(icons.at(0))));
	this->typeContent = 0;
    }
    this->setFixedWidth(300);

}

int QTypeContentButton::getNumber()
{
    return this->typeContent;
}

void QTypeContentButton::actionClick()
{
    QAction *action = qobject_cast<QAction *>(sender());
    int type = action->data().toInt();
    if ((type % 100 == 98) && (type < 998) && all){
	QMessageBox d;
	d.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	d.setIcon(QMessageBox::Warning);
	d.setText(tr("Built by hands whose type you choose, consider\n"
		     "that the submissions are not intended for persons\n"
		     "under 18 years.\n\n"
		     "You turned 18?"));
	if (d.exec() == QMessageBox::No){
	    return;
	}
    }
    this->setText(action->text());
    this->setIcon(action->icon());
    this->typeContent = type;
    emit typeChanged();
}

void QTypeContentButton::setDefault()
{
    this->setText(tr("Any"));
    this->typeContent = 0;
}

void QTypeContentButton::setTTH()
{
    this->setText(tr("TTH"));
    this->typeContent = 8;
}
