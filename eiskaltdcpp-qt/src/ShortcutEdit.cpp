/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ShortcutEdit.h"

#include <QKeySequence>

static int translateModifiers(Qt::KeyboardModifiers state, const QString &text) {
    int result = 0;
    if ((state & Qt::ShiftModifier) && (text.size() == 0
                                        || !text.at(0).isPrint()
                                        || text.at(0).isLetter()
                                        || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;

    return result;
}

ShortcutEdit::ShortcutEdit(QWidget *parent) :
        LineEdit(parent), keyNumber(0)
{
    installEventFilter(this);

    keys[0] = keys[1] = keys[2] = keys[3] = 0;
}

bool ShortcutEdit::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease || e->type() == QEvent::Shortcut){
        if (keyNumber < 3){
            for (int i = keyNumber; i < 3; i++)
                keys[i] = 0;
        }

        keyNumber = 0;
        return true;
    }
    else if (e->type() == QEvent::ShortcutOverride){
        e->accept();

        return true;
    }

    if (e->type() != QEvent::KeyPress)
        return LineEdit::eventFilter(obj, e);

    QKeyEvent *k_e = reinterpret_cast<QKeyEvent* >(e);

    if (!k_e)
        return true;

    int nextKey = k_e->key();
    if ( keyNumber > 3 ||
         nextKey == Qt::Key_Control ||
         nextKey == Qt::Key_Shift ||
         nextKey == Qt::Key_Meta ||
         nextKey == Qt::Key_Alt )
        return true;

    nextKey |= translateModifiers(k_e->modifiers(), k_e->text());
    switch (keyNumber) {
    case 0:
        keys[0] = nextKey;
        break;
    case 1:
        keys[1] = nextKey;
        break;
    case 2:
        keys[2] = nextKey;
        break;
    case 3:
        keys[3] = nextKey;
        break;
    default:
        break;
    }
    keyNumber++;
    QKeySequence ks(keys[0], keys[1], keys[2], keys[3]);

    setText(ks.toString());

    return true;
}
