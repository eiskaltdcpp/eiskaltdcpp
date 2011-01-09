/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef LINEEDIT_H
#define LINEEDIT_H

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

    explicit LineEdit(QWidget *parent = 0);
    virtual ~LineEdit();

    virtual void setPixmap(const QPixmap&);
    virtual void setMenu(QMenu*);
    virtual void setMenuRole(LineEdit::MenuRole);

    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy() const;

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

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

#endif // LINEEDIT_H
