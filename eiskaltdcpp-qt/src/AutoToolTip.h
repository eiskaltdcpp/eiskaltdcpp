/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/***
* Origin: http://www.mimec.org/node/337
*/

#pragma once

#include <QStyledItemDelegate>
#include <QHelpEvent>
#include <QAbstractItemView>

class AutoToolTipDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    AutoToolTipDelegate(QObject* parent);
    ~AutoToolTipDelegate() override;

public slots:
    bool helpEvent(QHelpEvent* e, QAbstractItemView* view, const QStyleOptionViewItem& option, const QModelIndex& index) override;
};
