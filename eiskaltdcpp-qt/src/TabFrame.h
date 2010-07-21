#ifndef TabFrame_H
#define TabFrame_H

#include <QFrame>
#include <QResizeEvent>
#include <QMap>
#include <QList>
#include <QShortcut>

#include "ArenaWidget.h"
#include "ArenaWidgetContainer.h"

class FlowLayout;
class TabButton;

class TabFrame :
        public QFrame,
        public ArenaWidgetContainer
{
Q_OBJECT
public:
    explicit TabFrame(QWidget *parent = 0);
    ~TabFrame();

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    virtual void removeWidget(ArenaWidget *awgt);
    virtual void insertWidget(ArenaWidget *awgt);
    virtual bool hasWidget(ArenaWidget *awgt) const;
    virtual void redraw();

public Q_SLOTS:
    virtual void mapped(ArenaWidget *awgt);
    virtual void nextTab();
    virtual void prevTab();
    void moveRight();
    void moveLeft();

private Q_SLOTS:
    void buttonClicked();
    void closeRequsted();
    void slotShorcuts();
    void slotContextMenu();
    void slotDropped(TabButton*);

protected:
    virtual void resizeEvent(QResizeEvent *e);

private:
    void historyPush(ArenaWidget*);
    void historyPurge(ArenaWidget*);
    void historyPop();

    FlowLayout *fr_layout;

    QList<ArenaWidget*> history;
    QList<QShortcut*> shortcuts;
    QMap<ArenaWidget*, TabButton*> awgt_map;
    QMap<TabButton*, ArenaWidget*> tbtn_map;
};

#endif // TabFrame_H
