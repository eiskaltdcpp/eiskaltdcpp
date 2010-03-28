#ifndef MAGNET_H
#define MAGNET_H

#include <QDialog>
#include <QEvent>
#include <QTimer>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/SearchResult.h"
#include "dcpp/SearchManager.h"

#include "ui_UIMagnet.h"
#include "Func.h"

class MagnetCustomEvent: public QEvent{
public:
    static const QEvent::Type Event = static_cast<QEvent::Type>(1210);

    MagnetCustomEvent(FuncBase *f = NULL): QEvent(Event), f(f)
    {}
    virtual ~MagnetCustomEvent(){ delete f; }

    FuncBase *func() { return f; }
private:
    FuncBase *f;
};

class Magnet :
        public QDialog,
        private Ui::UIMagnet
{
Q_OBJECT
public:
    explicit Magnet(QWidget *parent = 0);
    virtual ~Magnet();

    void setLink(const QString&);

protected:
    virtual void customEvent(QEvent *);

private slots:
    void search();
    void download();
    void timeout();
        void slotBrowse();

private:
    QTimer *t;
};

#endif // MAGNET_H
