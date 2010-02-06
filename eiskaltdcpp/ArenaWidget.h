#ifndef ARENAWIDGET_H
#define ARENAWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QAction>

class ArenaWidget
{
public:
    ArenaWidget();
    virtual ~ArenaWidget();

    virtual QWidget *getWidget() = 0;
    virtual QString getArenaTitle() = 0;
    virtual QMenu *getMenu() = 0;

    virtual void setUnload(bool b){ _arenaUnload = b; }
    virtual bool isUnload() const { return _arenaUnload; }

private:
    bool _arenaUnload;
};

#endif // ARENAWIDGET_H
