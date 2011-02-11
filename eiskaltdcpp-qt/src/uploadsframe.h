#ifndef UPLOADSFRAME_H
#define UPLOADSFRAME_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QThread>
#include <QTimer>


#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "dcpp/Singleton.h"
#include "dcpp/DirectoryListing.h"

#include "qtypecontentbutton.h"
#include "ui_UiUploadsFrame.h"

class UploadsFrame : public QWidget,
    public ArenaWidget,
    public dcpp::Singleton<UploadsFrame>,
    private Ui_UploadsFrame{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)
public:
    UploadsFrame(QWidget *parent = 0);
    ~UploadsFrame();

    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    QAction *toolButton() { return toolBtn; }
    void  setToolButton(QAction *btn) { if (btn) toolBtn = btn; }
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiDOWN); }

    ArenaWidget::Role role() const { return ArenaWidget::MyUploads; }

    void DEL_pressed() {}
    void CTRL_F_pressed() {}


protected:
    void changeEvent(QEvent *e);
    virtual void closeEvent(QCloseEvent *);

public slots:
    void chandgeType();
    void reload();
    void loadDMD(QString);

private:
    QString fName;
    bool _arenaUnload;
    QAction *toolBtn;
    QPixmap _pxmap;
    QTypeContentButton *type_btn;
//    QMap<int, QStringList> types;
    int current_type;
    QTimer *timer;
    int shareSize;

    void createTree(dcpp::DirectoryListing::Directory*, QStringList*, QString);

    bool exist7z;

    char getPersent(QString fileName);
    qint64 getSize(QString fileName);

private slots:
    void on_pushButton_clicked();
    void on_treeWidget_doubleClicked(QModelIndex index);
};

#endif // UPLOADSFRAME_H
