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

#include "AutoToolTip.h"

#include <QtCore>
#include <QTextDocument>
#include <QToolTip>

AutoToolTipDelegate::AutoToolTipDelegate(QObject *parent): QStyledItemDelegate(parent) {}
AutoToolTipDelegate::~AutoToolTipDelegate() {}

bool AutoToolTipDelegate::helpEvent(QHelpEvent* e, QAbstractItemView* view,
    const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (!e || !view)
        return false;

    if (e->type() == QEvent::ToolTip) {
        QRect rect = view->visualRect(index);
        QSize size = sizeHint(option, index);

        if (rect.width() < size.width()) {
            QVariant tooltip = index.data(Qt::DisplayRole);
            if ( tooltip.canConvert<QString>() ) {
                QToolTip::showText(e->globalPos(),
#if QT_VERSION >= 0x050000
                                   QString("<div>%1</div>").arg(tooltip.toString().toHtmlEscaped()),
#else
                                   QString("<div>%1</div>").arg(Qt::escape(tooltip.toString())),
#endif
                                   view);
                return true;
            }
        }
        if (!QStyledItemDelegate::helpEvent(e, view, option, index))
            QToolTip::hideText();
        return true;
    }

    return QStyledItemDelegate::helpEvent(e, view, option, index);
}
