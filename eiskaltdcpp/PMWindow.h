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

signals:
    void privateMessageClosed(QString);

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void closeEvent(QCloseEvent *);

private:
    void sendMessage(QString,bool = true);
    void addStatusMessage(QString);
    void addOutput(QString);

    QString cid;
    QString hubUrl;
    QMenu *arena_menu;
};

#endif // PMWindow_H
