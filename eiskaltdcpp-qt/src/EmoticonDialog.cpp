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

#include "EmoticonFactory.h"
#include "FlowLayout.h"

/** */
EmoticonDialog::EmoticonDialog(QWidget * parent, Qt::WindowFlags f)
: QDialog(parent, f) {
    m_pLayout = new FlowLayout(this);

    m_pLayout->setMargin(0);
    m_pLayout->setSpacing(0);

    setWindowTitle(tr("Select emoticon"));

    QSize s;
    EmoticonFactory::getInstance()->fillLayout(m_pLayout, s);

    resize(s);

    foreach(QLabel *l, findChildren<QLabel*>())
        l->installEventFilter(this);
}

/** */
EmoticonDialog::~EmoticonDialog() {
    delete m_pLayout;
}

/** event filter */
bool EmoticonDialog::eventFilter(QObject * object, QEvent * event) {
    if ((event->type() == QEvent::MouseButtonPress) && qobject_cast<QLabel*>(object)) {
        QLabel *l = qobject_cast<QLabel*>(object);

        selectedSmile = l->toolTip();

        accept();
    }

    return QWidget::eventFilter(object, event); // standard event processing
}
