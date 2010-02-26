#include "PMWindow.h"
#include "WulforSettings.h"
#include "WulforUtil.h"
#include "HubManager.h"
#include "MainWindow.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/QueueManager.h"
#include "dcpp/User.h"

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
        hasMessages(false)
{
    setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    plainTextEdit_INPUT->installEventFilter(this);
    textEdit_CHAT->viewport()->installEventFilter(this);

    arena_menu = new QMenu(tr("Private message"));
    QAction *close_wnd = new QAction(WulforUtil::getInstance()->getPixmap(WulforUtil::eiFILECLOSE), tr("Close"), arena_menu);
    arena_menu->addAction(close_wnd);

    connect(close_wnd, SIGNAL(triggered()), this, SLOT(close()));
    connect(pushButton_HUB, SIGNAL(clicked()), this, SLOT(slotHub()));
    connect(pushButton_SHARE, SIGNAL(clicked()), this, SLOT(slotShare()));
}

PMWindow::~PMWindow(){
    delete arena_menu;
}

bool PMWindow::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) &&
            (k_e->modifiers() == Qt::NoModifier)){
            sendMessage(plainTextEdit_INPUT->toPlainText());

            plainTextEdit_INPUT->setPlainText("");

            return false;
        }
    }
    else if (e->type() == QEvent::MouseButtonRelease){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if ((static_cast<QWidget*>(obj) == textEdit_CHAT->viewport()) && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

            WulforUtil::getInstance()->openUrl(pressedParagraph);
        }
    }

    return QWidget::eventFilter(obj, e);
}

void PMWindow::closeEvent(QCloseEvent *c_e){
    emit privateMessageClosed(cid);

    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);
    MainWindow::getInstance()->remArenaWidget(this);

    c_e->accept();
}

void PMWindow::showEvent(QShowEvent *e){
    e->accept();

    if (isVisible()){
        hasMessages = false;
        MainWindow::getInstance()->redrawToolPanel();
    }
}

QString PMWindow::getArenaTitle(){
    return WulforUtil::getInstance()->getNicks(CID(cid.toStdString())) + tr(" on hub ") + hubUrl;
}

QString PMWindow::getArenaShortTitle(){
    return WulforUtil::getInstance()->getNicks(CID(cid.toStdString()));
}

QWidget *PMWindow::getWidget(){
    return this;
}

QMenu *PMWindow::getMenu(){
    return arena_menu;
}

const QPixmap &PMWindow::getPixmap(){
    if (hasMessages)
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiMESSAGE);
    else
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiSERVER);
}

void PMWindow::addStatusMessage(QString msg){
    QString status = " * ";

    QString nick = "DC-CORE";
    QString time = "[" + QString::fromStdString(Util::getTimeString().c_str()) + "]";

    status = time + status;
    status += "<font color=\"" + WulforSettings::getInstance()->getStr(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>: ";
    status += msg;

    addOutput(status);
}

void PMWindow::addOutput(QString msg){

    /* This is temporary block. Later we must make it more wise. */
    msg.replace("\r", "");
    msg.replace("\n", "\n<br>");
    msg.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

    textEdit_CHAT->append(msg);

    if (!isVisible()){
        hasMessages = true;
        MainWindow::getInstance()->redrawToolPanel();
    }
}

void PMWindow::sendMessage(QString msg, bool stripNewLines){
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

    if (user && user->isOnline()){

        if (stripNewLines)
            msg.replace("\n", "");

        if (msg.isEmpty() || msg == "\n")
            return;

        ClientManager::getInstance()->privateMessage(user, msg.toStdString(), false, hubUrl.toStdString());
    }
    else {
        addStatusMessage(tr("User went offline"));
    }
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
