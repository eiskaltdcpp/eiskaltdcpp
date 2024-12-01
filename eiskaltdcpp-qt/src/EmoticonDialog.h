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
#include <QEvent>
#include <QPixmap>

class QLabel;
class QGridLayout;
class FlowLayout;

class EmoticonDialog : public QDialog {
    Q_OBJECT

public:
    /** construtor */
    EmoticonDialog(QWidget * parent = nullptr, Qt::WindowFlags f = {});
    /** destructor */
    virtual ~EmoticonDialog();

    QString getEmoticonText() const { return selectedSmile; }

private Q_SLOTS:
    void smileClicked();

private:
    /** */
    FlowLayout * m_pLayout;
    QString selectedSmile;
};
