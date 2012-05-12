/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QTextEdit>
#include <QAbstractTextDocumentLayout>
#include <QStringListModel>

#include <QDebug>

#include "UserListModel.h"

class QCompleter;

class NickCompletionModel: public QStringListModel
{
    Q_OBJECT

public:
    NickCompletionModel(QObject *parent = 0) : QStringListModel(parent)
    {}
    NickCompletionModel(const QStringList &strings, QObject *parent = 0) : QStringListModel(strings, parent)
    {}
    virtual ~NickCompletionModel()
    { }

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const
    {
        QVariant result = QStringListModel::data(index, role);

        if (result.isValid() && role == Qt::EditRole) {
            int end = -1;
            QString nick = result.toString();
            if (nick.startsWith("[") && (end = nick.lastIndexOf("]")) > 0 && nick.length() > ++end) {
                nick.remove(0, end);
                result = nick;
            }

        }
        return result;
    }
};

class ChatEdit : public QTextEdit
{
    Q_OBJECT

public:
    ChatEdit(QWidget *parent = 0);
    virtual ~ChatEdit();

    void setCompleter(QCompleter *, UserListModel *);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent *);
    void keyReleaseEvent(QKeyEvent *);
    void focusInEvent(QFocusEvent *);
    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent *e);
    void dragMoveEvent(QDragMoveEvent *event); // Required to accept drops on win32

private Q_SLOTS:
    void insertCompletion(const QModelIndex &);
    void recalculateGeometry() { updateGeometry(); updateScrollBar(); }
    void updateScrollBar();

private:
    QString textUnderCursor() const;
    void insertToPos(const QString &, int);
    void complete();

    UserListModel *cc_model;
    QCompleter *cc;
};
