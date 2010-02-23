#ifndef ARENAWIDGET_H
#define ARENAWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QAction>
#include <QPixmap>

class ArenaWidget
{
public:
    ArenaWidget();
    virtual ~ArenaWidget();

    virtual QWidget *getWidget() = 0;
    virtual QString getArenaTitle() = 0;
    virtual QString getArenaShortTitle() = 0;
    virtual QMenu *getMenu() = 0;
    virtual const QPixmap &getPixmap(){ return _pxmap; }

    virtual void setUnload(bool b){ _arenaUnload = b; }
    virtual bool isUnload() const { return _arenaUnload; }

private:
    bool _arenaUnload;
    QPixmap _pxmap;
};

#endif // ARENAWIDGET_H
