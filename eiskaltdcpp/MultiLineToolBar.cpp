#include "MultiLineToolBar.h"
#include "WulforSettings.h"

#include <QMenu>
#include <QWheelEvent>

MultiLineToolBar::MultiLineToolBar(QWidget *parent) :
    QToolBar(parent)
{
    setObjectName("multiLineTabbar");

    frame = new TabFrame();

    addWidget(frame);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(nextTab()), frame, SLOT(nextTab()));
    connect(this, SIGNAL(prevTab()), frame, SLOT(prevTab()));
    connect(this, SIGNAL(moveTabLeft()), frame, SLOT(moveLeft()));
    connect(this, SIGNAL(moveTabRight()), frame, SLOT(moveRight()));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu()));
}

MultiLineToolBar::~MultiLineToolBar(){
    frame->deleteLater();
}

void MultiLineToolBar::wheelEvent(QWheelEvent *e){
    e->ignore();

    if (e->delta() > 0)
        emit nextTab();
    else if (e->delta() < 0)
        emit prevTab();
}

void MultiLineToolBar::removeWidget(ArenaWidget *awgt){
    frame->removeWidget(awgt);
}

void MultiLineToolBar::insertWidget(ArenaWidget *awgt){
    frame->insertWidget(awgt);
}

bool MultiLineToolBar::hasWidget(ArenaWidget *awgt) const{
    return frame->hasWidget(awgt);
}

void MultiLineToolBar::mapped(ArenaWidget *awgt) {
    frame->mapped(awgt);
}

void MultiLineToolBar::redraw() {
    frame->redraw();
}

void MultiLineToolBar::slotContextMenu(){
    QMenu *m = new QMenu(this);
    QAction *act = new QAction(tr("Show close buttons"), m);

    act->setCheckable(true);
    act->setChecked(WBGET(WB_APP_TBAR_SHOW_CL_BTNS));

    m->addAction(act);

    if (m->exec(QCursor::pos()) != NULL){
        WBSET(WB_APP_TBAR_SHOW_CL_BTNS, act->isChecked());

        redraw();
    }

    m->deleteLater();
}
