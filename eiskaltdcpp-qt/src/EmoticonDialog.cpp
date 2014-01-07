/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

    for (const auto &l : findChildren<EmoticonLabel*>())
        connect(l, SIGNAL(clicked()), this, SLOT(smileClicked()));
}

/** */
EmoticonDialog::~EmoticonDialog() {
    m_pLayout->deleteLater();
}

void EmoticonDialog::smileClicked(){
    EmoticonLabel *lbl = qobject_cast<EmoticonLabel* >(sender());

    if (!lbl)
        return;

    selectedSmile = lbl->toolTip();

    accept();
}
