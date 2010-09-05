/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef EmoticonDialog_H
#define EmoticonDialog_H

#include <QDialog>
#include <QEvent>
#include <QPixmap>

class QLabel;
class QGridLayout;
class FlowLayout;

class EmoticonDialog : public QDialog {
    Q_OBJECT

public:
    /** construtor */
    EmoticonDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
    /** destructor */
    virtual ~EmoticonDialog();

    QString getEmoticonText() const { return selectedSmile; }

protected:
    /** */
    bool eventFilter(QObject * object, QEvent * event);

private:
    /** */
    FlowLayout * m_pLayout;
    QString selectedSmile;
};

#endif
