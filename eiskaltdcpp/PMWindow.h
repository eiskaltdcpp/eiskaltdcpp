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
                public Ui::UIPrivateMessage,
                public ArenaWidget
{
    Q_OBJECT

public:
    friend class HubFrame;

    PMWindow(QString cid, QString hubUrl);
    virtual ~PMWindow();

    QString  getArenaTitle();
    QString  getArenaShortTitle();
    QWidget *getWidget();
    QMenu   *getMenu();
    const QPixmap &getPixmap();

    void addStatus(QString);

public slots:
    void reloadSomeSettings();
    void nextMsg();
    void prevMsg();

private slots:
    void slotHub();
    void slotShare();
    void slotSmile();

signals:
    void privateMessageClosed(QString);

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);

private:
    void sendMessage(QString,bool = false);
    void addStatusMessage(QString);
    void addOutput(QString);

    bool hasMessages;

    static int unread;

    QString cid;
    QString hubUrl;
    QMenu *arena_menu;

    QStringList out_messages;
    int out_messages_index;
};

#endif // PMWindow_H
