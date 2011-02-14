/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef GETNAMEDMD_H
#define GETNAMEDMD_H

#include <QDialog>
#include <QStringList>

namespace Ui {
    class getNameDMD;
}

class getNameDMD : public QDialog {
    Q_OBJECT
public:
    getNameDMD(QString postfix, QStringList folders, QWidget *parent = 0);
    ~getNameDMD();
    bool isOk();
    QString getFileName();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::getNameDMD *ui;
    bool ok;
    QString postfix;

private slots:
    void on_pushButton_CANSEL_clicked();
    void on_pushButton_OK_clicked();
    void on_lineEdit_returnPressed();
    void changeData();
};

#endif // GETNAMEDMD_H
