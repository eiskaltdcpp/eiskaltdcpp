/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QDialog>

class QLineEdit;

class ShortcutGetter : public QDialog
{
    Q_OBJECT

public:
    ShortcutGetter(QWidget *parent = 0);

    QString exec(const QString& s);

protected slots:
    void setCaptureKeyboard(bool b);

protected:
        bool captureKeyboard() { return capture; }

    bool event(QEvent *e);
    bool eventFilter(QObject *o, QEvent *e);
    void setText();

private:
    bool bStop;
    QLineEdit *leKey;
    QStringList lKeys;
    bool capture;
};
