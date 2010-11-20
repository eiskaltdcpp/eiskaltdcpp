#ifndef QTYPECONTENTBUTTON_H
#define QTYPECONTENTBUTTON_H

#include <QPushButton>
#include <QMenu>
#include <QWidget>
#include <QAction>
#include <QStringList>
#include <QMessageBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QIcon>
#include <QPixmap>

class QTypeContentButton : public QPushButton
{
    Q_OBJECT
public:
    QTypeContentButton(bool all = true, QWidget *parent = 0);
    ~QTypeContentButton(){};

    int getNumber();
    void setDefault();
    void setTTH();
public slots:
    void actionClick();
signals:
    void typeChanged();
private:
     int typeContent;
     bool all;
};

#endif // QTYPECONTENTBUTTON_H
