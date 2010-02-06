#include "ToolBar.h"

#include <QMenu>

#include "ArenaWidget.h"
#include "MainWindow.h"

ToolBar::ToolBar(QWidget *parent):
    QToolBar(parent),
    tabbar(NULL)
{
}

ToolBar::~ToolBar(){
}

void ToolBar::initTabs(){
    tabbar = new QTabBar(this);
    tabbar->setTabsClosable(true);
    tabbar->setMovable(false);
    tabbar->setContextMenuPolicy(Qt::CustomContextMenu);
    tabbar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);

    connect(tabbar, SIGNAL(currentChanged(int)), this, SLOT(slotIndexChanged(int)));
    connect(tabbar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotClose(int)));
    connect(tabbar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

    addWidget(tabbar);
}

void ToolBar::insertWidget(ArenaWidget *awgt){
    if (!awgt || !awgt->getWidget() || map.contains(awgt))
        return;

    int index = tabbar->addTab(awgt->getArenaTitle().left(22)+"...");

    if (index >= 0){
        map.insert(awgt, index);

        if (tabbar->isHidden())
            tabbar->show();

        tabbar->setCurrentIndex(index);
    }
}

void ToolBar::removeWidget(ArenaWidget *awgt){
    if (!awgt || !awgt->getWidget() || !map.contains(awgt))
        return;

    int index = map.value(awgt);

    if (index >= 0){
        map.erase(map.find(awgt));

        rebuildIndexes(index);

        if (map.size() == 0)
            tabbar->hide();

        tabbar->removeTab(index);
    }
}

void ToolBar::slotIndexChanged(int index){
    if (index < 0)
        return;

    ArenaWidget *awgt = findWidgetForIndex(index);

    if (!awgt || !awgt->getWidget())
        return;

    MainWindow::getInstance()->mapWidgetOnArena(awgt);
}

void ToolBar::slotClose(int index){
    if (index < 0)
        return;

    ArenaWidget *awgt = findWidgetForIndex(index);

    if (!awgt || !awgt->getWidget())
        return;

    awgt->getWidget()->close();
}

void ToolBar::slotContextMenu(const QPoint &p){
    int tab = tabbar->tabAt(p);

    if (tab < 0)
        return;

    ArenaWidget *awgt = findWidgetForIndex(tab);

    if (!awgt)
        return;

    QMenu *m = awgt->getMenu();

    if (m)
        m->exec(QCursor::pos());
}

ArenaWidget *ToolBar::findWidgetForIndex(int index){
    if (index < 0)
        return NULL;

    WidgetMap::const_iterator it = map.begin();

    for (; it != map.end(); ++it){
        if (it.value() == index)
            return const_cast<ArenaWidget*>(it.key());
    }

    return NULL;
}

void ToolBar::redraw(){
    WidgetMap::const_iterator it = map.begin();

    for (; it != map.end(); ++it)
        tabbar->setTabText(it.value(), it.key()->getArenaTitle().left(22)+"...");

    ArenaWidget *awgt = findWidgetForIndex(tabbar->currentIndex());

    if (awgt)
        MainWindow::getInstance()->setWindowTitle(awgt->getArenaTitle());
}

void ToolBar::rebuildIndexes(int removed){
    if (removed < 0)
        return;

    WidgetMap::iterator it = map.begin();

    for (; it != map.end(); ++it){
        if (it.value() > removed)
            map[it.key()] = it.value()-1;
    }
}

void ToolBar::mapped(ArenaWidget *awgt){
    blockSignals(true);
    if (map.contains(awgt))
        tabbar->setCurrentIndex(map[awgt]);
    blockSignals(false);
}

bool ToolBar::hasWidget(ArenaWidget *w){
    return map.contains(w);
}

void ToolBar::mapWidget(ArenaWidget *w){
    if (hasWidget(w))
        tabbar->setCurrentIndex(map[w]);
}
