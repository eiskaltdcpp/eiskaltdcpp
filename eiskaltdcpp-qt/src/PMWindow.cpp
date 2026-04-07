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

#include "PMWindow.h"
#include "QtContextAware.h"
#include "QtContext.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "HubManager.h"
#include "MainWindow.h"
#include "Notification.h"
#include "EmoticonFactory.h"
#include "EmoticonDialog.h"
#include "FlowLayout.h"
#include "ArenaWidgetManager.h"

#include "dcpp/stdinc.h"
#include "dcpp/ClientManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/User.h"
#include "dcpp/DCPlusPlus.h"

#include <QTextBlock>
#include <QTextDocument>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QScrollBar>

using namespace dcpp;

static inline void clearLayout(QLayout *l){
    if (!l)
        return;

    for (QLayoutItem *item = l->takeAt(0); item; item = l->takeAt(0)) {
        l->removeWidget(item->widget());
        item->widget()->deleteLater();
        delete item;
    }

    l->invalidate();
}

PMWindow::PMWindow(const QString &cid_, const QString &hubUrl_):
        hasMessages(false),
        hasHighlightMessages(false),
        cid(cid_),
        hubUrl(hubUrl_),
        arena_menu(nullptr)
{
    setupUi(this);


    frame_SMILES->setLayout(new FlowLayout(frame_SMILES));
    frame_SMILES->setVisible(false);

    QSize sz;
    Q_UNUSED(sz);

    if (qtCtx()->emoticonFactory())
        qtCtx()->emoticonFactory()->fillLayout(frame_SMILES->layout(), sz);

    for (const auto &l : frame_SMILES->findChildren<EmoticonLabel*>())
        connect(l, &EmoticonLabel::clicked, this, &PMWindow::slotSmileClicked);

    setAttribute(Qt::WA_DeleteOnClose);

    lineEdit_FIND->installEventFilter(this);

    plainTextEdit_INPUT->setWordWrapMode(QTextOption::NoWrap);
    plainTextEdit_INPUT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plainTextEdit_INPUT->setContextMenuPolicy(Qt::CustomContextMenu);
    plainTextEdit_INPUT->installEventFilter(this);
    plainTextEdit_INPUT->setAcceptRichText(false);

    textEdit_CHAT->viewport()->installEventFilter(this);
    textEdit_CHAT->viewport()->setMouseTracking(true);
    textEdit_CHAT->document()->setMaximumBlockCount(qtCtx()->settings()->getInt(WI_CHAT_MAXPARAGRAPHS));
    textEdit_CHAT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit_CHAT->setTabStopDistance(40);

    frame_2->setVisible(false);

    updateStyles();

    if (qtCtx()->settings()->getBool(WB_APP_ENABLE_EMOTICON) && qtCtx()->emoticonFactory())
        qtCtx()->emoticonFactory()->addEmoticons(textEdit_CHAT->document());

    toolButton_SMILE->setVisible(qtCtx()->settings()->getBool(WB_APP_ENABLE_EMOTICON) && qtCtx()->emoticonFactory());
    toolButton_SMILE->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEMOTICON));
    toolButton_SMILE->setContextMenuPolicy(Qt::CustomContextMenu);

    toolButton_ALL->setCheckable(true);

    toolButton_HIDE->setIcon(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiEDITDELETE));

    arena_menu = new QMenu(tr("Private message"));
    QAction *close_wnd = new QAction(qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    if (!qtCtx()->settings()->getStr("hubframe/chat-background-color", "").isEmpty()){
        QPalette p = textEdit_CHAT->palette();
        QColor clr = p.color(QPalette::Active, QPalette::Base);

        clr.setNamedColor(qtCtx()->settings()->getStr("hubframe/chat-background-color"));

        if (clr.isValid()){
            p.setColor(QPalette::Base, clr);

            textEdit_CHAT->setPalette(p);
        }
    }

    connect(close_wnd, &QAction::triggered, this, &PMWindow::slotClose);
    connect(pushButton_HUB, &QPushButton::clicked, this, &PMWindow::slotHub);
    connect(pushButton_SHARE, &QPushButton::clicked, this, &PMWindow::slotShare);
    connect(toolButton_SMILE, &QToolButton::clicked, this, &PMWindow::slotSmile);
    connect(toolButton_SMILE, &QToolButton::customContextMenuRequested, this, &PMWindow::slotSmileContextMenu);
    connect(plainTextEdit_INPUT, &QTextEdit::textChanged, this, &PMWindow::inputTextChanged);
    connect(plainTextEdit_INPUT, &QWidget::customContextMenuRequested, this, &PMWindow::inputTextMenu);
    connect(qtCtx()->settings(), &WulforSettings::strValueChanged, this, &PMWindow::slotSettingChanged);
    connect(lineEdit_FIND, &QLineEdit::textChanged, this, &PMWindow::slotFindTextEdited);
    connect(toolButton_HIDE, &QToolButton::clicked, this, &PMWindow::slotHideSearchBar);
    connect(toolButton_BACK, &QToolButton::clicked, this, &PMWindow::slotFindBackward);
    connect(toolButton_FORWARD, &QToolButton::clicked, this, &PMWindow::slotFindForward);
    connect(toolButton_ALL, &QToolButton::clicked, this, &PMWindow::slotFindAll);

    out_messages_index = 0;
    out_messages_unsent = false;
    
    setState(state() & ~ArenaWidget::RaiseOnStart);// Do not allow PMWindow to be automatically showed by default
}

void PMWindow::setCompleter(QCompleter *completer, UserListModel *model) {
    plainTextEdit_INPUT->setCompleter(completer, model);
}

PMWindow::~PMWindow(){
    arena_menu->deleteLater();
}

void PMWindow::slotClose() {
    qtCtx()->arenaWidgetManager()->rem(this);
}

bool PMWindow::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) &&
            (k_e->modifiers() == Qt::NoModifier))
        {
            return true;
        }
        else if (static_cast<LineEdit*>(obj) == lineEdit_FIND && k_e->key() == Qt::Key_Escape){
            lineEdit_FIND->clear();
            slotHideSearchBar();
            return true;
        }
    }
    else if (e->type() == QEvent::KeyPress){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        const bool controlModifier = (k_e->modifiers() == Qt::ControlModifier);

        if (static_cast<QTextEdit*>(obj) == plainTextEdit_INPUT)
        {
            const bool useCtrlEnter = qtCtx()->settings()->getBool(WB_USE_CTRL_ENTER);
            const bool keyEnter = (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return);
            const bool shiftModifier = (k_e->modifiers() == Qt::ShiftModifier);

            if ((useCtrlEnter && keyEnter && controlModifier) ||
                (!useCtrlEnter && keyEnter && !controlModifier && !shiftModifier))
            {
                const QString msg = plainTextEdit_INPUT->toPlainText();

                HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->getHub(hubUrl));

                if (fr) {
                    if (!fr->parseForCmd(msg, this))
                        sendMessage(msg, false, false);
                }
                else {
                    sendMessage(msg, false, false);
                }

                plainTextEdit_INPUT->setPlainText("");

                return true;
            }
        }

        if (controlModifier){
            if (k_e->key() == Qt::Key_Equal || k_e->key() == Qt::Key_Plus){
                textEdit_CHAT->zoomIn();

                return true;
            }
            else if (k_e->key() == Qt::Key_Minus){
                textEdit_CHAT->zoomOut();

                return true;
            }
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if ((static_cast<QWidget*>(obj) == textEdit_CHAT->viewport()) && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

            qtCtx()->wulforUtil()->openUrl(pressedParagraph);
        }
    }
    else if (e->type() == QEvent::MouseMove && (static_cast<QWidget*>(obj) == textEdit_CHAT->viewport())){
        QString str = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

        if (!str.isEmpty())
            textEdit_CHAT->viewport()->setCursor(Qt::PointingHandCursor);
        else
            textEdit_CHAT->viewport()->setCursor(Qt::IBeamCursor);
    }
    else if (e->type() == QEvent::MouseButtonDblClick){
        HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->getHub(hubUrl));
        bool cursoratnick = false;
        QString nick = "",nickstatus="",nickmessage="";
        QString cid = "";
        QTextCursor cursor = textEdit_CHAT->textCursor();

        QString pressedParagraph = cursor.block().text();
        int positionCursor = cursor.columnNumber();
        int l = pressedParagraph.indexOf(" <");
        int r = pressedParagraph.indexOf("> ");
        if (l < r)
            nickmessage = pressedParagraph.mid(l+2, r-l-2);
        else {
            int l1 = pressedParagraph.indexOf(" * ");
            //qDebug() << positionCursor << " " << l1 << " " << l << " " << r;
            if (l1 > -1 ) {
                QString pressedParagraphstatus = pressedParagraph.remove(0,l1+3).simplified();
                //qDebug() << pressedParagraphstatus;
                int r1 = pressedParagraphstatus.indexOf(" ");
                //qDebug() << r1;
                nickstatus = pressedParagraphstatus.mid(0, r1);
                //qDebug() << nickstatus;
            }
        }
        if ((!nickmessage.isEmpty() || !nickstatus.isEmpty())&& fr){
            //qDebug() << nickstatus;
            //qDebug() << nickmessage;
            nick = nickmessage + nickstatus;
            //qDebug() << nick;
            cid = fr->getCIDforNick(nick);
        }
        if ((positionCursor < r) && (positionCursor > l))
            cursoratnick = true;

        if (!cid.isEmpty()){
            if (qtCtx()->settings()->getInt(WI_CHAT_DBLCLICK_ACT) == 1 && fr && cursoratnick)
                    fr->browseUserFiles(cid, false);
            else if (qtCtx()->settings()->getInt(WI_CHAT_DBLCLICK_ACT) == 2 && fr && cursoratnick)
                    fr->addPM(cid, "");
            else if (textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos())).startsWith("user://")){
                if(!plainTextEdit_INPUT->textCursor().position())
                    plainTextEdit_INPUT->textCursor().insertText(nick + qtCtx()->settings()->getStr(WS_CHAT_SEPARATOR) + " ");
                else
                    plainTextEdit_INPUT->textCursor().insertText(nick + " ");

                plainTextEdit_INPUT->setFocus();
            }
        }
    }

    return QWidget::eventFilter(obj, e);
}

void PMWindow::closeEvent(QCloseEvent *c_e){
    emit privateMessageClosed(cid);

    hasMessages = false;
    hasHighlightMessages = false;

    c_e->accept();
}

void PMWindow::showEvent(QShowEvent *e){
    e->accept();

    if (isVisible()){
        hasMessages = false;
        hasHighlightMessages = false;
        qtCtx()->mainWindow()->redrawToolPanel();
    }
}

void PMWindow::slotActivate(){
    plainTextEdit_INPUT->setFocus();
}

QString PMWindow::getArenaTitle(){
    QString nick = (cid.length() > 24)? qtCtx()->wulforUtil()->getNickViaOnlineUser(cid, hubUrl) : cid;

    nick_ = nick.isEmpty()? nick_ : nick;

    return (tr("%1 on hub %2").arg(nick_).arg(hubUrl));
}

QString PMWindow::getArenaShortTitle(){
    QString nick = (cid.length() > 24)? qtCtx()->wulforUtil()->getNickViaOnlineUser(cid, hubUrl) : cid;

    nick_ = nick.isEmpty()? nick_ : nick;

    return nick_;
}

QWidget *PMWindow::getWidget(){
    return this;
}

QMenu *PMWindow::getMenu(){
    return arena_menu;
}

const QPixmap &PMWindow::getPixmap(){
    if (hasHighlightMessages)
        return qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiMESSAGE);
    else if (hasMessages)
        return qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiPMMSG);
    else
        return qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiUSERS);
}

ArenaWidget::Role PMWindow::role() const {
    return ArenaWidget::PrivateMessage;
}

void PMWindow::requestClear(){
    clearChat();
}

void PMWindow::requestFilter() {
    slotShowSearchBar();
}

void PMWindow::requestFocus() {
    plainTextEdit_INPUT->setFocus();
}

void PMWindow::clearChat(){
    textEdit_CHAT->setHtml("");
    addStatus(tr("Chat cleared."));

    updateStyles();

    if (qtCtx()->settings()->getBool(WB_APP_ENABLE_EMOTICON) && qtCtx()->emoticonFactory())
        qtCtx()->emoticonFactory()->addEmoticons(textEdit_CHAT->document());
}

void PMWindow::updateStyles(){
    QString custom_font_desc = qtCtx()->settings()->getStr(WS_CHAT_PM_FONT);
    QFont custom_font;

    if (!custom_font_desc.isEmpty() && custom_font.fromString(custom_font_desc)){
        textEdit_CHAT->document()->setDefaultStyleSheet(
                                                        QString("pre { margin:0px; white-space:pre-wrap; font-family:'%1'; font-size: %2pt; }")
                                                        .arg(custom_font.family()).arg(custom_font.pointSize())
                                                       );
    }
    else {
        textEdit_CHAT->document()->setDefaultStyleSheet(
                                                        QString("pre { margin:0px; white-space:pre-wrap; font-family:'%1' }")
                                                        .arg(QApplication::font().family())
                                                       );
    }
}

void PMWindow::addStatusMessage(const QString &msg){
    QString status = " * ";

    QString nick = "";
    QString time = "";

    if (!qtCtx()->settings()->getStr(WS_CHAT_TIMESTAMP).isEmpty())
        time = "[" + QDateTime::currentDateTime().toString(qtCtx()->settings()->getStr(WS_CHAT_TIMESTAMP)) + "]";

    status = time + status;
    status += "<font color=\"" + qtCtx()->settings()->getStr(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>: ";
    status += msg;

    addOutput(status);
}

void PMWindow::addStatus(QString msg){
    QString status = "";
    QString nick    = " * ";

    qtCtx()->wulforUtil()->textToHtml(msg, true);
    qtCtx()->wulforUtil()->textToHtml(nick, true);

    msg             = "<font color=\"" + qtCtx()->settings()->getStr(WS_CHAT_MSG_COLOR) + "\">" + msg + "</font>";
    QString time    = "";

    if (!qtCtx()->settings()->getStr(WS_CHAT_TIMESTAMP).isEmpty())
        time = "<font color=\""+qtCtx()->settings()->getStr(WS_CHAT_TIME_COLOR)+">["+QDateTime::currentDateTime().toString(qtCtx()->settings()->getStr(WS_CHAT_TIMESTAMP))+"]</font>";

    status = time + "<font color=\"" + qtCtx()->settings()->getStr(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";
    status += msg;

    qtCtx()->wulforUtil()->textToHtml(status, false);

    addOutput(status);
}

void PMWindow::addOutput(QString msg){
    msg.replace("\r", "");
    msg = "<pre>" + msg + "</pre>";
    textEdit_CHAT->append(msg);

    if (!isVisible()) {
        hasMessages = true;
        qtCtx()->mainWindow()->redrawToolPanel();
    }
}

void PMWindow::addUserData(const QString &nick){
    QTextDocument *chatDoc = textEdit_CHAT->document();
    for (QTextBlock itu = chatDoc->lastBlock(); itu.isValid(); itu = itu.previous()){
        if (!itu.userData())
            itu.setUserData(new UserListUserData(nick));
        else
            break;
    }
}

void PMWindow::sendMessage(QString msg, const bool thirdPerson, const bool stripNewLines){
    UserPtr user = qtCtx()->dcCtx().getClientManager()->findUser(CID(cid.toStdString()));

    if (user && user->isOnline()){

        if (stripNewLines)
            msg.replace("\n", "");

        if (msg.isEmpty() || msg == "\n")
            return;

        qtCtx()->dcCtx().getClientManager()->privateMessage(HintedUser(user, _tq(hubUrl)), _tq(msg), thirdPerson);
    }
    else {
        addStatusMessage(tr("User went offline"));
    }

    if (out_messages_unsent){
        out_messages.removeLast();
        out_messages_unsent = false;
    }

    out_messages << msg;

    if (out_messages.size() > qtCtx()->settings()->getInt(WI_OUT_IN_HIST))
        out_messages.removeFirst();

    out_messages_index = out_messages.size()-1;
}

QWidget *PMWindow::inputWidget() const {
    return plainTextEdit_INPUT;
}

void PMWindow::setHasHighlightMessages(bool h) {
    hasHighlightMessages = (h && !isVisible());
}

bool PMWindow::hasNewMessages() {
    return (hasMessages || hasHighlightMessages);
}

void PMWindow::nextMsg(){
    if (!plainTextEdit_INPUT->hasFocus())
        return;

    if (out_messages_index < 0 ||
        out_messages_index+1 > out_messages.size()-1 ||
        out_messages.isEmpty())
        return;

    if (out_messages.at(out_messages_index) != plainTextEdit_INPUT->toPlainText())
        out_messages[out_messages_index] = plainTextEdit_INPUT->toPlainText();

    if (out_messages_index+1 <= out_messages.size()-1)
        out_messages_index++;

    plainTextEdit_INPUT->setPlainText(out_messages.at(out_messages_index));

    if (out_messages_unsent && out_messages_index == out_messages.size()-1){
        out_messages.removeLast();
        out_messages_unsent = false;
        out_messages_index = out_messages.size()-1;
    }
}

void PMWindow::prevMsg(){
    if (!plainTextEdit_INPUT->hasFocus())
        return;

    if (out_messages_index < 1 ||
        out_messages_index-1 > out_messages.size()-1 ||
        out_messages.isEmpty())
        return;

    if (!out_messages_unsent && out_messages_index == out_messages.size()-1){
        out_messages << plainTextEdit_INPUT->toPlainText();
        out_messages_unsent = true;
        out_messages_index++;
    }

    if (out_messages.at(out_messages_index) != plainTextEdit_INPUT->toPlainText())
        out_messages[out_messages_index] = plainTextEdit_INPUT->toPlainText();

    if (out_messages_index >= 1)
        out_messages_index--;

    plainTextEdit_INPUT->setPlainText(out_messages.at(out_messages_index));
}

void PMWindow::slotHub(){
    HubFrame *fr = qobject_cast<HubFrame*>(qtCtx()->hubManager()->getHub(hubUrl));

    if (fr)
        qtCtx()->arenaWidgetManager()->activate(fr);
}

void PMWindow::slotShare(){
    string cid = this->cid.toStdString();

    if (!cid.empty()){
        try{
            UserPtr user = qtCtx()->dcCtx().getClientManager()->findUser(CID(cid));

            if (user){
                if (user == qtCtx()->dcCtx().getClientManager()->getMe())
                    qtCtx()->mainWindow()->browseOwnFiles();
                else
                    qtCtx()->dcCtx().getQueueManager()->addList(HintedUser(user, _tq(hubUrl)), QueueItem::FLAG_CLIENT_VIEW, "");
            }
        }
        catch (const Exception &e){}
    }
}

void PMWindow::slotSmile(){
    if (!(qtCtx()->settings()->getBool(WB_APP_ENABLE_EMOTICON) && qtCtx()->emoticonFactory()))
        return;

    if (qtCtx()->settings()->getBool(WB_CHAT_USE_SMILE_PANEL)){
        frame_SMILES->setVisible(!frame_SMILES->isVisible());
    }
    else {
        EmoticonDialog *dialog = new EmoticonDialog(this);

        if (dialog->exec() == QDialog::Accepted) {

            QString smiley = dialog->getEmoticonText();

            if (!smiley.isEmpty()) {

                smiley.replace("&lt;", "<");
                smiley.replace("&gt;", ">");
                smiley.replace("&amp;", "&");
                smiley.replace("&apos;", "\'");
                smiley.replace("&quot;", "\"");

                smiley += " ";

                plainTextEdit_INPUT->textCursor().insertText(smiley);
                plainTextEdit_INPUT->setFocus();
            }
        }

        delete dialog;
    }
}

void PMWindow::slotSmileClicked(){
    EmoticonLabel *lbl = qobject_cast<EmoticonLabel* >(sender());

    if (!lbl)
        return;

    QString smiley = lbl->toolTip();

    if (!smiley.isEmpty()) {

        smiley.replace("&lt;", "<");
        smiley.replace("&gt;", ">");
        smiley.replace("&amp;", "&");
        smiley.replace("&apos;", "\'");
        smiley.replace("&quot;", "\"");

        smiley += " ";

        plainTextEdit_INPUT->textCursor().insertText(smiley);
        plainTextEdit_INPUT->setFocus();
    }

    if (qtCtx()->settings()->getBool(WB_CHAT_HIDE_SMILE_PANEL))
        frame_SMILES->setVisible(false);
}


void PMWindow::slotSmileContextMenu(){
    QMenu *m = new QMenu(this);
    QAction * a = nullptr;

    for (const auto &f : QDir(qtCtx()->wulforUtil()->getEmoticonsPath())
                              .entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot)){
        if (!f.isEmpty()){
            QAction * act = m->addAction(f);
            act->setCheckable(true);

            if (f == qtCtx()->settings()->getStr(WS_APP_EMOTICON_THEME))
                act->setChecked(true);
        }
    }

    a = m->exec(QCursor::pos());

    if (a && a->isChecked())
        qtCtx()->settings()->setStr(WS_APP_EMOTICON_THEME, a->text());
}

void PMWindow::slotSettingChanged(const QString &key, const QString &value){
    Q_UNUSED(value);

    if (key == WS_CHAT_PM_FONT)
        updateStyles();
    else if (key == WS_APP_EMOTICON_THEME){
        if (qtCtx()->emoticonFactory()){
            qtCtx()->emoticonFactory()->load();

            frame_SMILES->setVisible(false);

            clearLayout(frame_SMILES->layout());

            QSize sz;
            Q_UNUSED(sz);

            qtCtx()->emoticonFactory()->fillLayout(frame_SMILES->layout(), sz);

            for (const auto &l : frame_SMILES->findChildren<EmoticonLabel*>())
                connect(l, &EmoticonLabel::clicked, this, &PMWindow::slotSmileClicked);
        }

        toolButton_SMILE->setVisible(!value.isEmpty() && qtCtx()->settings()->getBool(WB_APP_ENABLE_EMOTICON) && qtCtx()->emoticonFactory());
    }
    else if (key == "hubframe/chat-background-color"){
        QPalette p = textEdit_CHAT->palette();
        QColor clr = p.color(QPalette::Active, QPalette::Base);

        clr.setNamedColor(value);

        if (clr.isValid()){
            p.setColor(QPalette::Base, clr);

            textEdit_CHAT->setPalette(p);
        }
    }
    else if (key == WS_TRANSLATION_FILE)
        retranslateUi(this);
}

void PMWindow::slotShowSearchBar(){
    frame_2->setVisible(true);
    QString stext = textEdit_CHAT->textCursor().selectedText();
    if (!stext.isEmpty())
        lineEdit_FIND->setText(stext);
    lineEdit_FIND->selectAll();
    lineEdit_FIND->setFocus();
}

void PMWindow::slotHideSearchBar(){
    frame_2->setVisible(false);
    QTextCursor c = textEdit_CHAT->textCursor();
    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());
    textEdit_CHAT->setTextCursor(c);
}

void PMWindow::slotFindTextEdited(const QString & text){
    if (text.isEmpty()){
        textEdit_CHAT->verticalScrollBar()->setValue(textEdit_CHAT->verticalScrollBar()->maximum());
        textEdit_CHAT->textCursor().movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);

        return;
    }

    QTextCursor c = textEdit_CHAT->textCursor();

    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), c, QTextDocument::FindFlags());
    if (!c.isNull()) {
        textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());
        textEdit_CHAT->setTextCursor(c);
        slotFindAll();
    }
}

void PMWindow::slotFindAll(){
    if (!toolButton_ALL->isChecked()){
        textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!lineEdit_FIND->text().isEmpty()) {
        QTextEdit::ExtraSelection selection;

        QColor color;
        color.setNamedColor(qtCtx()->settings()->getStr(WS_CHAT_FIND_COLOR));
        color.setAlpha(qtCtx()->settings()->getInt(WI_CHAT_FIND_COLOR_ALPHA));

        selection.format.setBackground(color);

        QTextCursor c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), 0, QTextDocument::FindFlags());

        while (!c.isNull()){
            selection.cursor = c;
            extraSelections.append(selection);

            c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), c, QTextDocument::FindFlags());
        }
    }

    textEdit_CHAT->setExtraSelections(extraSelections);
}

void PMWindow::findText(QTextDocument::FindFlags flag){
    textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    if (lineEdit_FIND->text().isEmpty())
        return;

    QTextCursor c = textEdit_CHAT->textCursor();

    bool ok = textEdit_CHAT->find(lineEdit_FIND->text(), flag);

    if (flag == QTextDocument::FindBackward && !ok)
        c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor,1);
    else if (!flag && !ok)
        c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor,1);

    c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), c, flag);

    if (!c.isNull()) {
        textEdit_CHAT->setTextCursor(c);
        slotFindAll();
    }
}
