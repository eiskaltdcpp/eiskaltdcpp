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
