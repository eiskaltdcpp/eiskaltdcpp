#ifndef PMWindow_H
#define PMWindow_H

#include "ui_PrivateMessage.h"
#include "ArenaWidget.h"
#include "HubFrame.h"

class QKeyEvent;
class QEvent;
class QObject;
class QCloseEvent;
class QMenu;
class QShowEvent;

class PMWindow: public QWidget,
                private Ui::UIPrivateMessage,
                public ArenaWidget
{
    Q_OBJECT

public:
    friend class HubFrame;

    PMWindow(QString cid, QString hubUrl);
    virtual ~PMWindow();

    QString  getArenaTitle();
    QWidget *getWidget();
    QMenu   *getMenu();
    const QPixmap &getPixmap();

private slots:
    void slotHub();
    void slotShare();

signals:
    void privateMessageClosed(QString);

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);

private:
    void sendMessage(QString,bool = true);
    void addStatusMessage(QString);
    void addOutput(QString);

    bool hasMessages;

    QString cid;
    QString hubUrl;
    QMenu *arena_menu;
};

#endif // PMWindow_H
