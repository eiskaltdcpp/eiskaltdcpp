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
    virtual QAction *toolButton() { return toolBtn; }
    virtual void  setToolButton(QAction *btn) { if (btn) toolBtn = btn; }
    virtual const QPixmap &getPixmap(){ return _pxmap; }

    virtual void DEL_pressed() {}
    virtual void CTRL_F_pressed() {}

    virtual void setUnload(bool b){ _arenaUnload = b; }
    virtual bool isUnload() const { return _arenaUnload; }

private:
    bool _arenaUnload;
    QAction *toolBtn;
    QPixmap _pxmap;
};

Q_DECLARE_INTERFACE (ArenaWidget, "com.NegatiV.EiskaltDCPP.ArenaWidget/1.0")
#endif // ARENAWIDGET_H
