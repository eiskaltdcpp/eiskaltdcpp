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
    NickCompletionModel(QObject *parent = nullptr) : QStringListModel(parent)
    {}
    NickCompletionModel(const QStringList &strings, QObject *parent = nullptr) : QStringListModel(strings, parent)
    {}
    ~NickCompletionModel() override
    { }

    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override
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
    ChatEdit(QWidget *parent = nullptr);
    ~ChatEdit() override;

    void setCompleter(QCompleter *, UserListModel *);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;
    void focusInEvent(QFocusEvent *) override;
    void dropEvent(QDropEvent *) override;
    void dragEnterEvent(QDragEnterEvent *e) override;
    void dragMoveEvent(QDragMoveEvent *event) override; // Required to accept drops on win32

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
