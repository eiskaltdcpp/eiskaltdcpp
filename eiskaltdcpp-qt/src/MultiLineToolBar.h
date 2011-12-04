/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef MULTILINETOOLBAR_H
#define MULTILINETOOLBAR_H

#include <QToolBar>

#include "ArenaWidget.h"
#include "TabFrame.h"

class QWheelEvent;

/** Wrapper for TabFrame */
class MultiLineToolBar : public QToolBar
{
Q_OBJECT
public:
    explicit MultiLineToolBar(QWidget *parent = 0);
    virtual ~MultiLineToolBar();

protected:
    virtual void wheelEvent(QWheelEvent *);

signals:
    void nextTab();
    void prevTab();
    void moveTabRight();
    void moveTabLeft();

private Q_SLOTS:
    void slotContextMenu();

private:
    TabFrame *frame;
};

#endif // MULTILINETOOLBAR_H
