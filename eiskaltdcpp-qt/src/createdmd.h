#ifndef CREATEDMD_H
#define CREATEDMD_H

#include <QDialog>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QPixmap>

#include "qtypecontentbutton.h"
#include "ui_createdmd.h"

#include "WulforUtil.h"
#include "dcpp/SearchManager.h"

class CreateDMD : public QDialog,
		  private Ui_CreateDMD
{
    Q_OBJECT
public:
    CreateDMD(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~CreateDMD();

protected:
    void changeEvent(QEvent *e);

private:

    QTypeContentButton *type_btn;
    /*QMap<int, QStringList> types;
    void on_listWidget_dragMoveEvent(QDragMoveEvent *event);
    void on_listWidget_dragEnterEvent(QDragEnterEvent *e);
    void on_listWidget_dropEvent(QDropEvent *event);*/

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_11_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_5_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_8_clicked();
};

#endif // CREATEDMD_H
