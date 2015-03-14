/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ChatEdit.h"
#include "WulforUtil.h"

#include "dcpp/HashManager.h"

#include <QCompleter>
#include <QKeyEvent>
#include <QScrollBar>
#include <QTextBlock>
#include <QUrl>
#include <QFileInfo>
#include <QDir>
#include <QMimeData>

ChatEdit::ChatEdit(QWidget *parent) : QTextEdit(parent), cc(NULL)
{
    setMinimumHeight(10);

    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, SIGNAL(textChanged()), this, SLOT(recalculateGeometry()));
}

ChatEdit::~ChatEdit()
{}

void ChatEdit::setCompleter(QCompleter *completer, UserListModel *model)
{
    if (cc)
        QObject::disconnect(cc, 0, this, 0);

    cc = completer;

    if (!cc || !model)
        return;

    cc->setWidget(this);
    cc->setWrapAround(false);
    cc->setCaseSensitivity(Qt::CaseInsensitive);
    cc->setCompletionMode(QCompleter::PopupCompletion);

    cc_model = model;

    QObject::connect(cc, SIGNAL(activated(const QModelIndex&)),
                     this, SLOT(insertCompletion(const QModelIndex&)));
}

QSize ChatEdit::minimumSizeHint() const{
    QSize sh = QTextEdit::minimumSizeHint();
    sh.setHeight(fontMetrics().height() + 1);
    sh += QSize(0, QFrame::lineWidth() * 2);
    return sh;
}

QSize ChatEdit::sizeHint() const{
    QSize sh = QTextEdit::sizeHint();
    sh.setHeight(int(document()->documentLayout()->documentSize().height()));
    sh += QSize(0, QFrame::lineWidth() * 2);
    ((QTextEdit*)this)->setMaximumHeight(sh.height());
    return sh;
}

void ChatEdit::insertCompletion(const QModelIndex & index)
{
    if (cc->widget() != this || !index.isValid())
        return;

    QString nick = cc->completionModel()->index(index.row(), index.column()).data().toString();
    int begin = textCursor().position() - cc->completionPrefix().length();

    insertToPos(nick, begin);
}

void ChatEdit::insertToPos(const QString & completeText, int begin)
{
    if (completeText.isEmpty())
        return;

    if (begin < 0)
        begin = 0;

    QTextCursor cursor = textCursor();
    int end = cursor.position();
    cursor.setPosition(begin);
    cursor.setPosition(end, QTextCursor::KeepAnchor);

    if (!begin)
        cursor.insertText(completeText + ": ");
    else
        cursor.insertText(completeText + " ");

    setTextCursor(cursor);
}

QString ChatEdit::textUnderCursor() const
{
    QTextCursor cursor = textCursor();

    int curpos = cursor.position();
    QString text = cursor.block().text().left(curpos);

    QStringList wordList = text.split(QRegExp("\\s"));

    if (wordList.isEmpty())
        return QString();

    return wordList.last();
}

void ChatEdit::focusInEvent(QFocusEvent *e)
{
    if (cc)
        cc->setWidget(this);

    QTextEdit::focusInEvent(e);
}

void ChatEdit::keyPressEvent(QKeyEvent *e)
{
    const bool ctrlOrShift = e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    bool hasModifier = (e->modifiers() != Qt::NoModifier) &&
                       (e->modifiers() != Qt::KeypadModifier) &&
                       !ctrlOrShift;

    if (e->key() == Qt::Key_Tab) {
        if (!toPlainText().isEmpty()) {
            if (cc && cc->popup()->isVisible()) {
                int row = cc->popup()->currentIndex().row() + 1;
                if (cc->completionModel()->rowCount() == row)
                    row = 0;
                cc->popup()->setCurrentIndex(cc->completionModel()->index(row, 0));
            }
            e->accept();
        } else {
            e->ignore();
        }
        return;
    }

    if (cc && cc->popup()->isVisible()) {
        switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Escape:
        case Qt::Key_Backtab:
            e->ignore();
            return;
        default:
            break;
        }
    }

    if (!cc || !cc->popup()->isVisible() || !hasModifier)
        QTextEdit::keyPressEvent(e);

    if (ctrlOrShift && e->text().isEmpty())
        return;

    if (cc->popup()->isVisible() && (hasModifier || e->text().isEmpty())) {
        cc->popup()->hide();
        return;
    }

    if (cc->popup()->isVisible())
        complete();
}

void ChatEdit::keyReleaseEvent(QKeyEvent *e)
{
    bool hasModifier = (e->modifiers() != Qt::NoModifier);

    switch (e->key()) {
    case Qt::Key_Tab:
        if (cc && !hasModifier && !cc->popup()->isVisible())
            complete();

    case Qt::Key_Enter:
    case Qt::Key_Return:
        e->ignore();
        return;
    default:
        break;
    }
}

void ChatEdit::complete()
{
    QString completionPrefix = textUnderCursor();

    if (completionPrefix.isEmpty()) {
        if (cc->popup()->isVisible())
            cc->popup()->hide();

        return;
    }

    if (!cc->popup()->isVisible() || completionPrefix.length() < cc->completionPrefix().length()) {
        QString pattern = QString("(\\[.*\\])?%1.*").arg( QRegExp::escape(completionPrefix) );
        QStringList nicks = cc_model->findItems(pattern, Qt::MatchRegExp, 0);

        if (nicks.isEmpty())
            return;

        if (nicks.count() == 1) {
            insertToPos(nicks.last(), textCursor().position() - completionPrefix.length());
            return;
        }

        NickCompletionModel *tmpModel = new NickCompletionModel(nicks, cc);
        cc->setModel(tmpModel);
    }

    if (completionPrefix != cc->completionPrefix()) {
        cc->setCompletionPrefix(completionPrefix);
        cc->popup()->setCurrentIndex(cc->completionModel()->index(0, 0));
    }

    QRect cr = cursorRect();
    cr.setWidth(cc->popup()->sizeHintForColumn(0)
                + cc->popup()->verticalScrollBar()->sizeHint().width());

    cc->complete(cr);
}

void ChatEdit::dragMoveEvent(QDragMoveEvent *event) {
    event->accept();
}

void ChatEdit::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls() || e->mimeData()->hasText()) {
        e->acceptProposedAction();
    } else {
        e->ignore();
    }
}

void ChatEdit::dropEvent(QDropEvent *e)
{
    if (e->mimeData()->hasUrls()) {

        e->setDropAction(Qt::IgnoreAction);

        QStringList fileNames;
        for (const auto url : e->mimeData()->urls()) {
            QString urlStr = url.toString();
            if (url.scheme().toLower() == "file") {
                QFileInfo fi( url.toLocalFile() );
                QString str = QDir::toNativeSeparators( fi.absoluteFilePath() );

                if ( fi.exists() && fi.isFile() && !str.isEmpty() ) {
                    const TTHValue *tth = HashManager::getInstance()->getFileTTHif(str.toStdString());
                    if ( !tth ) {
                        str = QDir::toNativeSeparators( fi.canonicalFilePath() ); // try to follow symlinks
                        tth = HashManager::getInstance()->getFileTTHif(str.toStdString());
                    }
                    if (tth)
                        urlStr = WulforUtil::getInstance()->makeMagnet(fi.fileName(), fi.size(), _q(tth->toBase32()));
                }
            };

            if (!urlStr.isEmpty())
                fileNames << urlStr;
        }

        if (!fileNames.isEmpty()) {

            QString dropText = (fileNames.count() == 1) ? fileNames.last() : "\n" + fileNames.join("\n");

            QMimeData mime;
            mime.setText(dropText);
            QDropEvent drop(e->pos(), Qt::CopyAction, &mime, e->mouseButtons(),
                            e->keyboardModifiers(), e->type());

            QTextEdit::dropEvent(&drop);
            return;
        }
    }
    QTextEdit::dropEvent(e);
}

void ChatEdit::updateScrollBar(){
    setVerticalScrollBarPolicy(sizeHint().height() > height() ? Qt::ScrollBarAlwaysOn : Qt::ScrollBarAlwaysOff);
    ensureCursorVisible();
}
