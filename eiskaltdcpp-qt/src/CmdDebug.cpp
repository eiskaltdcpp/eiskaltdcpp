/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QScrollBar>
#include <QStringList>
#include <QTextCursor>
#include <QTextEdit>
#include <QTextDocument>

#include "WulforUtil.h"
#include "CmdDebug.h"

CmdDebug::CmdDebug(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new CmdDebugPrivate())
{
    setupUi(this);
    Q_D(CmdDebug);

    toolButton_HIDE->setIcon(WICON(WulforUtil::eiEDITDELETE));
    searchFrame->hide();

    installEventFilter(this);
    lineEdit_FIND->installEventFilter(this);

    d->maxLines = spinBoxLines->value();
    plainTextEdit_DEBUG->document()->setMaximumBlockCount(d->maxLines);
    plainTextEdit_DEBUG->setReadOnly(true);
    plainTextEdit_DEBUG->setMouseTracking(true);

    connect(this, SIGNAL(coreDebugCommand(const QString&, const QString&)), this, SLOT(addOutput(const QString&, const QString&)), Qt::QueuedConnection);
    connect(spinBoxLines, SIGNAL(valueChanged(int)), this, SLOT(maxLinesChanged(int)));
    connect(pushButton_ClearLog, SIGNAL(clicked(bool)), plainTextEdit_DEBUG, SLOT(clear()));
    connect(toolButton_BACK, SIGNAL(clicked()), this, SLOT(slotFindBackward()));
    connect(toolButton_FORWARD, SIGNAL(clicked()), this, SLOT(slotFindForward()));
    connect(toolButton_HIDE, SIGNAL(clicked()), this, SLOT(slotHideSearchBar()));
    connect(lineEdit_FIND, SIGNAL(textEdited(QString)), this, SLOT(slotFindTextEdited(QString)));
    connect(toolButton_ALL, SIGNAL(clicked()), this, SLOT(slotFindAll()));
    DebugManager::getInstance()->addListener(this);

    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString,QString)), this, SLOT(slotSettingsChanged(QString,QString)));

    ArenaWidget::setState( ArenaWidget::Flags(ArenaWidget::state() | ArenaWidget::Singleton | ArenaWidget::Hidden) );
}

CmdDebug::~CmdDebug()
{
    Q_D(CmdDebug);

    DebugManager::getInstance()->removeListener(this);
    delete d;
}

QWidget *CmdDebug::getWidget() {
    return this;
}

QString CmdDebug::getArenaTitle() {
    return tr("Debug Console");
}

QString CmdDebug::getArenaShortTitle() {
    return getArenaTitle();
}

QMenu *CmdDebug::getMenu() {
    return nullptr;
}

bool CmdDebug::eventFilter(QObject *obj, QEvent *e){
    Q_D(CmdDebug);

    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        const bool keyEnter = (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return);
        const bool shiftModifier = (k_e->modifiers() == Qt::ShiftModifier);

        if (static_cast<QLineEdit*>(obj) == lineEdit_FIND) {
            const bool ret = QWidget::eventFilter(obj, e);

            if (keyEnter) {
                slotFindForward();
            }

            return ret;
        }

    }
    else if (e->type() == QEvent::KeyPress){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        const bool controlModifier = (k_e->modifiers() == Qt::ControlModifier);

        if (qobject_cast<LineEdit*>(obj) == lineEdit_FIND && k_e->key() == Qt::Key_Escape){
            lineEdit_FIND->clear();
            slotHideSearchBar();
            return true;
        }

#if QT_VERSION >= 0x050000
        if (controlModifier) {
            if (k_e->key() == Qt::Key_Equal || k_e->key() == Qt::Key_Plus){
                plainTextEdit_DEBUG->zoomIn();

                return true;
            }
            else if (k_e->key() == Qt::Key_Minus){
                plainTextEdit_DEBUG->zoomOut();

                return true;
            }
        }
#endif
    }
    else if (e->type() == QEvent::MouseButtonPress){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        bool isChat = (static_cast<QWidget*>(obj) == plainTextEdit_DEBUG->viewport());

        if (isChat)
            plainTextEdit_DEBUG->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        if (isChat && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = plainTextEdit_DEBUG->anchorAt(plainTextEdit_DEBUG->mapFromGlobal(QCursor::pos()));

            if (!WulforUtil::getInstance()->openUrl(pressedParagraph)){
                /**
                  Do nothing
                */
            }
        }
    }

    return QWidget::eventFilter(obj, e);
}

void CmdDebug::addOutput(const QString& msg, const QString& url) {
    if (checkBoxFilterIP->isChecked()) {
        const QStringList &&urlList = url.split(":");
        const QStringList &&addresses = lineEditIP->text().split(",", Qt::SkipEmptyParts);
        if (urlList.isEmpty() || addresses.isEmpty())
            return;

        const QString &ipAddress = urlList.at(0);
        bool found = false;
        for (const auto &a : addresses) {
            if (a.trimmed() == ipAddress) {
                found = true;
                break;
            }
        }

        if (!found)
            return;
    }

    QString tmp = msg;
    tmp.replace("\r", "");
    plainTextEdit_DEBUG->appendPlainText(tmp);
}

void CmdDebug::maxLinesChanged(int value){
    Q_D(CmdDebug);
    d->maxLines = value;
    plainTextEdit_DEBUG->document()->setMaximumBlockCount(value);
}

void CmdDebug::slotFindTextEdited(const QString &text){
    if (text.isEmpty()){
        plainTextEdit_DEBUG->verticalScrollBar()->setValue(plainTextEdit_DEBUG->verticalScrollBar()->maximum());
        plainTextEdit_DEBUG->textCursor().movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);

        return;
    }

    QTextCursor c = plainTextEdit_DEBUG->textCursor();

    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    c = plainTextEdit_DEBUG->document()->find(lineEdit_FIND->text(), c, {});
    if (!c.isNull()) {
        plainTextEdit_DEBUG->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        plainTextEdit_DEBUG->setTextCursor(c);
        slotFindAll();
    }
}

void CmdDebug::slotFindAll(){
    if (!toolButton_ALL->isChecked()){
        plainTextEdit_DEBUG->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!lineEdit_FIND->text().isEmpty()) {
        QTextEdit::ExtraSelection selection;

        QColor color;
        color.setNamedColor(WSGET(WS_CHAT_FIND_COLOR));
        color.setAlpha(WIGET(WI_CHAT_FIND_COLOR_ALPHA));

        selection.format.setBackground(color);

        QTextCursor c = plainTextEdit_DEBUG->document()->find(lineEdit_FIND->text(), 0, {});

        while (!c.isNull()) {
            selection.cursor = c;
            extraSelections.append(selection);

            c = plainTextEdit_DEBUG->document()->find(lineEdit_FIND->text(), c, {});
        }
    }
    plainTextEdit_DEBUG->setExtraSelections(extraSelections);
}

void CmdDebug::slotShowSearchBar(){
    searchFrame->setVisible(true);
    QString stext = plainTextEdit_DEBUG->textCursor().selectedText();
    if (!stext.isEmpty())
        lineEdit_FIND->setText(stext);
    lineEdit_FIND->selectAll();
    lineEdit_FIND->setFocus();
}

void CmdDebug::slotHideSearchBar(){
    searchFrame->setVisible(false);
    QTextCursor c = plainTextEdit_DEBUG->textCursor();
    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    plainTextEdit_DEBUG->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    plainTextEdit_DEBUG->setTextCursor(c);
}

void CmdDebug::findText(QTextDocument::FindFlags flag){
    plainTextEdit_DEBUG->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    if (lineEdit_FIND->text().isEmpty())
        return;

    QTextCursor c = plainTextEdit_DEBUG->textCursor();

    const bool ok = plainTextEdit_DEBUG->find(lineEdit_FIND->text(), flag);

    if (flag == QTextDocument::FindBackward && !ok)
        c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor,1);
    else if (!flag && !ok)
        c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor,1);

    c = plainTextEdit_DEBUG->document()->find(lineEdit_FIND->text(), c, flag);
    if (!c.isNull()) {
        plainTextEdit_DEBUG->setTextCursor(c);
        slotFindAll();
    }
}

void CmdDebug::slotSettingsChanged(const QString &key, const QString&){
    if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void CmdDebug::on(DebugManagerListener::DebugDetection, const string &com) noexcept {
    Q_UNUSED(com)
    // NOTE: we do not use this function in core
}

void CmdDebug::on(DebugManagerListener::DebugCommand, const string &mess, int typedir, const string &ip) noexcept {
    QString qmess = _q(mess);

    switch(typedir) {
        case dcpp::DebugManager::HUB_IN :
            if(checkBoxHUB_IN->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Hub: [Incoming] [" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::HUB_OUT :
            if(checkBoxHUB_OUT->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Hub: [Outgoing] [" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::CLIENT_IN:
            if(checkBoxCL_IN->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Client: [Incoming] ["  + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::CLIENT_OUT:
            if(checkBoxCL_OUT->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "Client: [Outgoing] [" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
#ifdef WITH_DHT
        case dcpp::DebugManager::DHT_IN:
            if(checkBoxDHT_IN->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "DHT: [Incoming] ["  + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
        case dcpp::DebugManager::DHT_OUT:
            if(checkBoxDHT_OUT->isChecked())
            {
                QString qip = _q(ip);
                QString msg = "DHT: [Outgoing] [" + qip + "] "+ qmess;
                emit coreDebugCommand(msg, qip);
            }
            break;
#endif
        default: break;
    }
}

