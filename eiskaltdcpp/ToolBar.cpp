#include "ToolBar.h"

#include <QMenu>
#include <QMouseEvent>

#include "ArenaWidget.h"
#include "MainWindow.h"

ToolBar::ToolBar(QWidget *parent):
    QToolBar(parent),
    tabbar(NULL)
{
}

ToolBar::~ToolBar(){
}

bool ToolBar::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::MouseButtonRelease){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if (reinterpret_cast<QTabBar*>(obj) == tabbar && m_e->button() == Qt::MidButton){
            QPoint p = tabbar->mapFromGlobal(QCursor::pos());
            int index = tabbar->tabAt(p);

            if (index >= 0)
                slotClose(index);
        }
    }

    return QToolBar::eventFilter(obj, e);
}

void ToolBar::showEvent(QShowEvent *e){
    e->accept();

    if (tabbar && e->spontaneous()){
        tabbar->hide();// I know, this is crap, but tabbar->repaint() doesn't fit all tabs in tabbar properly when
        tabbar->show();// MainWindow becomes visible (restoring from system tray)
    }
}

void ToolBar::initTabs(){
    tabbar = new QTabBar(parentWidget());
    tabbar->setObjectName("arenaTabbar");
    tabbar->setTabsClosable(true);
    tabbar->setDocumentMode(true);
    tabbar->setMovable(true);
    tabbar->setContextMenuPolicy(Qt::CustomContextMenu);
    tabbar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);

    tabbar->installEventFilter(this);

    connect(tabbar, SIGNAL(currentChanged(int)), this, SLOT(slotIndexChanged(int)));
    connect(tabbar, SIGNAL(tabMoved(int,int)), this, SLOT(slotTabMoved(int,int)));
    connect(tabbar, SIGNAL(tabCloseRequested(int)), this, SLOT(slotClose(int)));
    connect(tabbar, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotContextMenu(QPoint)));

    addWidget(tabbar);
}

void ToolBar::insertWidget(ArenaWidget *awgt, bool keepFocus){
    if (!awgt || !awgt->getWidget() || map.contains(awgt))
        return;

    int index = tabbar->addTab(awgt->getPixmap(), awgt->getArenaShortTitle().left(32));

    if (index >= 0){
        map.insert(awgt, index);

        if (tabbar->isHidden())
            tabbar->show();

        if (!keepFocus)
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

void ToolBar::slotTabMoved(int from, int to){
    ArenaWidget *from_wgt = NULL;
    ArenaWidget *to_wgt   = NULL;

    WidgetMap::iterator it = map.begin();

    for (; it != map.end(); ++it){
        if (it.value() == from){
            from_wgt = it.key();
        }
        else if (it.value() == to)
            to_wgt = it.key();

        if (to_wgt && from_wgt){
            map[to_wgt] = from;
            map[from_wgt] = to;

            slotIndexChanged(tabbar->currentIndex());

            return;
        }
    }
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

    for (; it != map.end(); ++it){
        tabbar->setTabText(it.value(), it.key()->getArenaShortTitle().left(32));
        tabbar->setTabToolTip(it.value(), compactToolTipText(it.key()->getArenaTitle()));
        tabbar->setTabIcon(it.value(), it.key()->getPixmap());
    }

    tabbar->repaint();

    ArenaWidget *awgt = findWidgetForIndex(tabbar->currentIndex());

    if (awgt)
        MainWindow::getInstance()->setWindowTitle(awgt->getArenaTitle());
}

void ToolBar::nextTab(){
    if (!tabbar)
        return;

    if (tabbar->currentIndex()+1 < tabbar->count())
        tabbar->setCurrentIndex(tabbar->currentIndex()+1);
    else
        tabbar->setCurrentIndex(0);
}

void ToolBar::prevTab(){
    if (!tabbar)
        return;

    if (tabbar->currentIndex()-1 >= 0)
        tabbar->setCurrentIndex(tabbar->currentIndex()-1);
    else
        tabbar->setCurrentIndex(tabbar->count()-1);
}

QString ToolBar::compactToolTipText(QString text)
{
    int maxlen = 60;

    int len = text.size();

    if (len <= maxlen)
        return text;

    int n = 0;
    int k = maxlen;

    while((len-k) > 0){
        if(text.at(k) == ' ' || (k == n))
        {
            if(k == n)
                k += maxlen;

            text.insert(k+1,'\n');

            len++;
            k += maxlen + 1;
            n += maxlen + 1;
        }
        else k--;
    }

    return text;
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

    redraw();

    blockSignals(false);
}

bool ToolBar::hasWidget(ArenaWidget *w){
    return map.contains(w);
}

void ToolBar::mapWidget(ArenaWidget *w){
    if (hasWidget(w))
        tabbar->setCurrentIndex(map[w]);
}
