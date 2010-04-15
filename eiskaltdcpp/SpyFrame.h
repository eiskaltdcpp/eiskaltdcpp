#ifndef SPYFRAME_H
#define SPYFRAME_H

#include <QWidget>
#include <QCloseEvent>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/Singleton.h"

#include "WulforUtil.h"
#include "ArenaWidget.h"
#include "ui_UISpy.h"
#include "Func.h"

class SpyModel;

class SpyFrameCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1206);

    SpyFrameCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~SpyFrameCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class SpyFrame :
        public QWidget,
        public ArenaWidget,
        public dcpp::Singleton<SpyFrame>,
        private dcpp::ClientManagerListener,
        private Ui::UISpy
{
Q_OBJECT
Q_INTERFACES(ArenaWidget)

friend class dcpp::Singleton<SpyFrame>;

public:

    QString getArenaShortTitle() { return tr("Search Spy"); }
    QString getArenaTitle() {return getArenaShortTitle(); }
    QMenu *getMenu() {return NULL; }
    QWidget *getWidget() { return this; }
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiSPY); }

protected:
    virtual void closeEvent(QCloseEvent *);
    virtual void customEvent(QEvent *);

private slots:
    void slotStartStop();
    void slotClear();
    void contextMenu();

private:
    explicit SpyFrame(QWidget *parent = 0);
    ~SpyFrame();

    SpyModel *model;

    virtual void on(dcpp::ClientManagerListener::IncomingSearch, const std::string& s) throw();
};

#endif // SPYFRAME_H
