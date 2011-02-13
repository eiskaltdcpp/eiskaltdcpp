/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef VIEWDMD_H
#define VIEWDMD_H

#include <QFile>
#include <QString>
#include <QTemporaryFile>
#include <QImage>
#include <QPixmap>

#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "qdmd.h"

#include "ui_viewdmd.h"

class ViewDMD : public QWidget,
		public ArenaWidget,
		private Ui_ViewDMD
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)
public:
    ViewDMD(QString fileName, QWidget *parent = 0);
    ~ViewDMD();

    QString getError(){return error;};

    QWidget *getWidget(){return this;};
    QString getArenaTitle(){return tr("Upload - ") + fName;};
    QString getArenaShortTitle();
    QMenu *getMenu(){return arena_menu;};
    const QPixmap &getPixmap(){return WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWNLIST);};
    ArenaWidget::Role role() const { return ArenaWidget::UploadView; };
protected:
    virtual void closeEvent(QCloseEvent*);
    void changeEvent(QEvent *e);

private:
    QString error;
    QString fName;
    QMenu *arena_menu;

    QPixmap src1, src2;



private slots:
    void on_pushButton_clicked();
};

#endif // VIEWDMD_H
