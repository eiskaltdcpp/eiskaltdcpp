#ifndef MULTILINETOOLBAR_H
#define MULTILINETOOLBAR_H

#include <QToolBar>

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"
#include "TabFrame.h"

class QWheelEvent;

/** Wrapper for TabFrame */
class MultiLineToolBar :
        public QToolBar,
        public ArenaWidgetContainer
{
Q_OBJECT
public:
    explicit MultiLineToolBar(QWidget *parent = 0);
    virtual ~MultiLineToolBar();

    virtual void removeWidget(ArenaWidget *awgt);
    virtual void insertWidget(ArenaWidget *awgt);
    virtual bool hasWidget(ArenaWidget *awgt) const;
    virtual void redraw();

protected:
    virtual void wheelEvent(QWheelEvent *);

signals:
    void nextTab();
    void prevTab();
    void moveTabRight();
    void moveTabLeft();

public Q_SLOTS:
    virtual void mapped(ArenaWidget *awgt);

private Q_SLOTS:
    void slotContextMenu();

private:
    TabFrame *frame;
};

#endif // MULTILINETOOLBAR_H
