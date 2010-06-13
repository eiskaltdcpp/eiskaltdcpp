#include "MultiLineToolBar.h"

MultiLineToolBar::MultiLineToolBar(QWidget *parent) :
    QToolBar(parent)
{
    setObjectName("multiLineTabbar");

    frame = new TabFrame();

    addWidget(frame);

    connect(this, SIGNAL(nextTab()), frame, SLOT(nextTab()));
    connect(this, SIGNAL(prevTab()), frame, SLOT(prevTab()));
}

MultiLineToolBar::~MultiLineToolBar(){
    frame->deleteLater();
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
