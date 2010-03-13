/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "EmoticonDialog.h"

#include <QLabel>
#include <QLayout>
#include <QMouseEvent>

/** */
EmoticonDialog::EmoticonDialog(QWidget * parent, Qt::WindowFlags f)
: QDialog(parent, f) {
    m_pGridLayout = new QGridLayout(this);

    // Q3GridLayout is QGridLayout with margin and spacing set to zero
    m_pGridLayout->setMargin(0);
    m_pGridLayout->setSpacing(0);

    m_pLabel = new QLabel(this);
    m_pGridLayout->addWidget(m_pLabel, 0, 0);
    m_pLabel->show();

    m_nX = m_nY = 0;

    m_pLabel->installEventFilter(this);
}

/** */
EmoticonDialog::~EmoticonDialog() {
    delete m_pGridLayout;
    delete m_pLabel;
}

/** */
void EmoticonDialog::SetPixmap(QPixmap & pixmap) {
    m_pLabel->setPixmap(pixmap);
    resize(pixmap.width(), pixmap.height());
}

/** */
void EmoticonDialog::GetXY(int & x, int & y) {
    x = m_nX;
    y = m_nY;
}

/** event filter */
bool EmoticonDialog::eventFilter(QObject * object, QEvent * event) {
    if ((event->type() == QEvent::MouseButtonPress) && ((QLabel*) object == m_pLabel)) {
        QMouseEvent * e = (QMouseEvent*) event;

        m_nX = e->x();
        m_nY = e->y();

        //printf("EVENT %d %d!\n",e->x(),e->y());

        accept();
    }

    return QWidget::eventFilter(object, event); // standard event processing
}
