/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QLineEdit>
#include <QLabel>
#include <QResizeEvent>
#include <QEvent>
#include <QPixmap>
#include <QMenu>
#include <QFocusEvent>

class LineEdit : public QLineEdit
{
Q_OBJECT
public:
    enum MenuRole{
        InsertText=0,
        EmitSignal
    };

    explicit LineEdit(QWidget *parent = nullptr);
    ~LineEdit() override;

    virtual void setPixmap(const QPixmap&);
    virtual void setMenu(QMenu*);
    virtual void setMenuRole(LineEdit::MenuRole);

    QSize sizeHint() const override;
    virtual QSizePolicy sizePolicy() const;

protected:
    void resizeEvent(QResizeEvent *) override;
    void focusInEvent(QFocusEvent *) override;
    void focusOutEvent(QFocusEvent *) override;
    bool eventFilter(QObject *, QEvent *) override;

signals:
    void clearEdit();
    void menuAction(QAction*);

private slots:
    void slotTextChanged();

private:
    void updateStyles();
    void updateGeometry();

    QLabel *label;
    QPixmap pxm;

    QMenu *menu;
    MenuRole role;

    int parentHeight;
};
