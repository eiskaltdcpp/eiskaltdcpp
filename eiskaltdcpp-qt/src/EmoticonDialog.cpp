/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "EmoticonDialog.h"
#include "QtContextAware.h"
#include "QtContext.h"

#include <QLabel>
#include <QLayout>
#include <QMouseEvent>

#include "EmoticonFactory.h"
#include "FlowLayout.h"

/** */
EmoticonDialog::EmoticonDialog(QWidget * parent, Qt::WindowFlags f)
: QDialog(parent, f) {
    m_pLayout = new FlowLayout(this);

    m_pLayout->setContentsMargins(0, 0, 0, 0);
    m_pLayout->setSpacing(0);

    setWindowTitle(tr("Select emoticon"));

    QSize s;
    qtCtx()->emoticonFactory()->fillLayout(m_pLayout, s);

    setMinimumSize(s);
    resize(s);

    for (const auto &l : findChildren<EmoticonLabel*>())
        connect(l, &EmoticonLabel::clicked, this, &EmoticonDialog::smileClicked);
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
