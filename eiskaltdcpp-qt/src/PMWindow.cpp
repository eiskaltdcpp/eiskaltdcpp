#include "PMWindow.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "HubManager.h"
#include "MainWindow.h"
#include "Notification.h"
#include "EmoticonFactory.h"
#include "EmoticonDialog.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/User.h"

#include <QTextBlock>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>

using namespace dcpp;

PMWindow::PMWindow(QString cid, QString hubUrl):
        cid(cid),
        hubUrl(hubUrl),
        arena_menu(NULL),
        hasMessages(false),
        hasHighlightMessages(false)
{
    setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    plainTextEdit_INPUT->setWordWrapMode(QTextOption::NoWrap);
    plainTextEdit_INPUT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plainTextEdit_INPUT->setContextMenuPolicy(Qt::CustomContextMenu);
    plainTextEdit_INPUT->installEventFilter(this);
    plainTextEdit_INPUT->setAcceptRichText(false);

    textEdit_CHAT->viewport()->installEventFilter(this);
    textEdit_CHAT->viewport()->setMouseTracking(true);

    textEdit_CHAT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit_CHAT->setTabStopWidth(40);

    updateStyles();

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_CHAT->document());

    toolButton_SMILE->setVisible(WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance());
    toolButton_SMILE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEMOTICON));

    arena_menu = new QMenu(tr("Private message"));
    QAction *close_wnd = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));
    connect(pushButton_HUB, SIGNAL(clicked()), this, SLOT(slotHub()));
    connect(pushButton_SHARE, SIGNAL(clicked()), this, SLOT(slotShare()));
    connect(toolButton_SMILE, SIGNAL(clicked()), this, SLOT(slotSmile()));
    connect(plainTextEdit_INPUT, SIGNAL(textChanged()), this, SIGNAL(inputTextChanged()));
    connect(plainTextEdit_INPUT, SIGNAL(customContextMenuRequested(QPoint)), this, SIGNAL(inputTextMenu()));
    connect(WulforSettings::getInstance(), SIGNAL(fontChanged(QString,QString)), this, SLOT(slotFontChanged(QString,QString)));

    out_messages_index = 0;

    reloadSomeSettings();
}

void PMWindow::setCompleter(QCompleter *completer, UserListModel *model) {
    plainTextEdit_INPUT->setCompleter(completer, model);
}

PMWindow::~PMWindow(){
    delete arena_menu;
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
    }
    else if (e->type() == QEvent::KeyPress){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (!WBGET(WB_USE_CTRL_ENTER) || k_e->modifiers() == Qt::ControlModifier) &&
            ((k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) && k_e->modifiers() != Qt::ShiftModifier) ||
            (k_e->key() == Qt::Key_Enter && k_e->modifiers() == Qt::KeypadModifier))
        {
            QString msg = plainTextEdit_INPUT->toPlainText();

            HubFrame *fr = HubManager::getInstance()->getHub(hubUrl);

            if (fr){
                if (!fr->parseForCmd(msg, this))
                    sendMessage(msg, false, false);
            }
            else
                sendMessage(msg, false, false);

            plainTextEdit_INPUT->setPlainText("");

            return true;
        }

        if (k_e->modifiers() == Qt::ControlModifier){
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

            WulforUtil::getInstance()->openUrl(pressedParagraph);
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
        HubFrame *fr = HubManager::getInstance()->getHub(hubUrl);
        bool cursoratnick;
        QString nick = "";
        QString cid = "";
        QTextCursor cursor = textEdit_CHAT->textCursor();

        QString pressedParagraph = cursor.block().text();
        int positionCursor = cursor.columnNumber();
        int l = pressedParagraph.indexOf(" <");
        int r = pressedParagraph.indexOf("> ");

        if ((l < r) && fr){
            nick = pressedParagraph.mid(l+2, r-l-2);
            cid = fr->getCIDforNick(nick);
        }
        if ((positionCursor < r) && (positionCursor > l))
            cursoratnick = true;
        else
            cursoratnick = false;

        if (!cid.isEmpty()){
            if (WIGET(WI_CHAT_DBLCLICK_ACT) == 1 && fr){
                if (cursoratnick)
                    fr->browseUserFiles(cid, false);
            }
            else if (WIGET(WI_CHAT_DBLCLICK_ACT) == 2 && fr){
                if (cursoratnick)
                    fr->addPM(cid, "");
            }
            else if (textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos())).startsWith("user://")){
                if (plainTextEdit_INPUT->textCursor().position() == 0)
                    plainTextEdit_INPUT->textCursor().insertText(nick + WSGET(WS_CHAT_SEPARATOR) + " ");
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

    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
    MainWindow::getInstance()->remArenaWidget(this);

    hasMessages = false;
    hasHighlightMessages = false;
    MainWindow::getInstance()->redrawToolPanel();

    c_e->accept();
}

void PMWindow::showEvent(QShowEvent *e){
    e->accept();

    if (isVisible()){
        hasMessages = false;
        hasHighlightMessages = false;
        MainWindow::getInstance()->redrawToolPanel();
    }
}

void PMWindow::slotActivate(){
    plainTextEdit_INPUT->setFocus();
}

void PMWindow::reloadSomeSettings(){
}

QString PMWindow::getArenaTitle(){
    QString nick = (cid.length() > 24)? WulforUtil::getInstance()->getNicks(CID(cid.toStdString())) : cid;

    nick_ = nick.isEmpty()? nick_ : nick;

    return (tr("%1 on hub %2").arg(nick_).arg(hubUrl));
}

QString PMWindow::getArenaShortTitle(){
    QString nick = (cid.length() > 24)? WulforUtil::getInstance()->getNicks(CID(cid.toStdString())) : cid;

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
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiMESSAGE);
    else if (hasMessages)
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiPMMSG);
    else
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiUSERS);
}

void PMWindow::clearChat(){
    textEdit_CHAT->setHtml("");
    addStatus(tr("Chat cleared."));

    updateStyles();

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_CHAT->document());
}

void PMWindow::updateStyles(){
    QString custom_font_desc = WSGET(WS_CHAT_PM_FONT);
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

void PMWindow::addStatusMessage(QString msg){
    QString status = " * ";

    QString nick = "DC-CORE";
    QString time = "";

    if (!WSGET(WS_CHAT_TIMESTAMP).isEmpty())
        time = "[" + QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP)) + "]";

    status = time + status;
    status += "<font color=\"" + WulforSettings::getInstance()->getStr(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>: ";
    status += msg;

    addOutput(status);
}

void PMWindow::addStatus(QString msg){
    QString status = "";
    QString nick    = " * ";

    WulforUtil::getInstance()->textToHtml(msg, true);
    WulforUtil::getInstance()->textToHtml(nick, true);

    msg             = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + msg + "</font>";
    QString time    = "";

    if (!WSGET(WS_CHAT_TIMESTAMP).isEmpty())
        time = "<font color=\""+WSGET(WS_CHAT_TIME_COLOR)+">["+QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP))+"]</font>";

    status = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";
    status += msg;

    WulforUtil::getInstance()->textToHtml(status, false);

    addOutput(status);
}

void PMWindow::addOutput(QString msg){
    msg.replace("\r", "");
    msg = "<pre>" + msg + "</pre>";
    textEdit_CHAT->append(msg);

    if (!isVisible()){
        hasMessages = true;
        MainWindow::getInstance()->redrawToolPanel();
    }
}

void PMWindow::sendMessage(QString msg, bool thirdPerson, bool stripNewLines){
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

    if (user && user->isOnline()){

        if (stripNewLines)
            msg.replace("\n", "");

        if (msg.isEmpty() || msg == "\n")
            return;

        ClientManager::getInstance()->privateMessage(user, msg.toStdString(), thirdPerson, hubUrl.toStdString());
    }
    else {
        addStatusMessage(tr("User went offline"));
    }

    out_messages << msg;

    if (out_messages.size() >= WIGET(WI_OUT_IN_HIST))
        out_messages.removeAt(0);

    out_messages_index = out_messages.size()-1;
}

void PMWindow::nextMsg(){
    if (!plainTextEdit_INPUT->hasFocus())
        return;

    if (out_messages_index < 0 ||
        out_messages.size()-1 < out_messages_index+1 ||
        out_messages.size() == 0)
        return;

    plainTextEdit_INPUT->setPlainText(out_messages.at(out_messages_index+1));

    if (out_messages_index < out_messages.size()-1)
        out_messages_index++;
    else
        out_messages_index = out_messages.size()-1;
}

void PMWindow::prevMsg(){
    if (!plainTextEdit_INPUT->hasFocus())
        return;

    if (out_messages_index < 0 ||
        out_messages.size()-1 < out_messages_index ||
        out_messages.size() == 0)
        return;

    plainTextEdit_INPUT->setPlainText(out_messages.at(out_messages_index));

    if (out_messages_index >= 1)
        out_messages_index--;
}

void PMWindow::slotHub(){
    HubFrame *fr = HubManager::getInstance()->getHub(hubUrl);

    if (fr)
        MainWindow::getInstance()->mapWidgetOnArena(fr);
}

void PMWindow::slotShare(){
    string cid = this->cid.toStdString();

    if (!cid.empty()){
        try{
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

            if (user){
                if (user == ClientManager::getInstance()->getMe())
                    MainWindow::getInstance()->browseOwnFiles();
                else
                    QueueManager::getInstance()->addList(user, _tq(hubUrl), QueueItem::FLAG_CLIENT_VIEW);
            }
        }
        catch (const Exception &e){}
    }
}

void PMWindow::slotSmile(){
    if (!(WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance()))
        return;

    int x, y;
    EmoticonDialog *dialog = new EmoticonDialog(this);
    QPixmap p = QPixmap::fromImage(EmoticonFactory::getInstance()->getImage());
    dialog->SetPixmap(p);

    if (dialog->exec() == QDialog::Accepted) {

        dialog->GetXY(x, y);

        QString smiley = EmoticonFactory::getInstance()->textForPos(x, y);

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

void PMWindow::slotFontChanged(const QString &key, const QString &value){
    Q_UNUSED(value);

    if (key == WS_CHAT_PM_FONT)
        updateStyles();
}
