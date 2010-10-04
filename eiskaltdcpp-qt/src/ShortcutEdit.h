/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef SHORTCUTEDIT_H
#define SHORTCUTEDIT_H

#include <QLineEdit>
#include <QEvent>
#include <QKeyEvent>

#include "LineEdit.h"

class ShortcutEdit : public LineEdit
{
    Q_OBJECT
public:
    explicit ShortcutEdit(QWidget *parent = 0);

protected:
    virtual bool eventFilter(QObject *, QEvent *);

private:
    int keyNumber;
    int keys[4];
};

#endif // SHORTCUTEDIT_H
