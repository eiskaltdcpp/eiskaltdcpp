/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QPushButton>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QDragMoveEvent>
#include <QMouseEvent>

class QResizeEvent;
class QPaintEvent;
class QLabel;
class QEvent;

class TabButton : public QPushButton
{
Q_OBJECT
public:
    explicit TabButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    void setWidgetIcon(const QPixmap &px);
    void resetGeometry() { updateGeometry(); }
    int normalWidth() const;
    int normalHeight() const;

protected:
    void resizeEvent(QResizeEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dragMoveEvent(QDragMoveEvent *) override;
    void dropEvent(QDropEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

signals:
    void closeRequest();
    void dropped(TabButton*);

private:
    void updateStyles();
    void updateGeometry();

    QLabel *label;
    QLabel *px_label;
    int parentHeight;
    bool isLeftBtnHold;
};
