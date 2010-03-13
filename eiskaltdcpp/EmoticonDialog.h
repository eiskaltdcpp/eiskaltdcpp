/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

class EmoticonDialog : public QDialog {
    Q_OBJECT

public:
    /** construtor */
    EmoticonDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
    /** destructor */
    virtual ~EmoticonDialog();

    /** */
    void SetPixmap(QPixmap & pixmap);
    /** */
    void GetXY(int & x, int & y);

protected:
    /** */
    bool eventFilter(QObject * object, QEvent * event);

private:
    /** */
    QLabel * m_pLabel;
    /** */
    QGridLayout * m_pGridLayout;
    /** */
    int m_nX, m_nY;
};

#endif
