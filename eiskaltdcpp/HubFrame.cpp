#include "HubFrame.h"
#include "MainWindow.h"
#include "PMWindow.h"
#include "Func.h"
#include "WulforUtil.h"
#include "Antispam.h"
#include "HubManager.h"
#include "Notification.h"
#include "ShellCommandRunner.h"
#include "EmoticonDialog.h"
#include "WulforSettings.h"
#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif

#include "UserListModel.h"

#include "dcpp/LogManager.h"
#include "dcpp/User.h"
#include "dcpp/UserCommand.h"
#include "dcpp/CID.h"

#if HAVE_MALLOC_TRIM
#include <malloc.h>
#endif

#include <QMouseEvent>
#include <QTextCodec>
#include <QItemSelectionModel>
#include <QMenu>
#include <QClipboard>
#include <QInputDialog>
#include <QTextCursor>
#include <QTextBlock>
#include <QUrl>
#include <QCloseEvent>
#include <QThread>
#include <QRegExp>
#include <QScrollBar>
#include <QShortcut>

#include <QtDebug>

#include <exception>

QStringList HubFrame::LinkParser::link_types = QString("http://,https://,ftp://,dchub://,adc://,adcs://,magnet:,www.").split(",");
HubFrame::Menu *HubFrame::Menu::instance = NULL;
unsigned HubFrame::Menu::counter = 0;

void HubFrame::Menu::newInstance(){
    delete instance;

    instance = new Menu();
}

void HubFrame::Menu::deleteInstance(){
    delete instance;

    instance = NULL;
}

HubFrame::Menu *HubFrame::Menu::getInstance(){
    return instance;
}

HubFrame::Menu::Menu(){
    menu = new QMenu();
    WulforUtil *WU = WulforUtil::getInstance();

    last_user_cmd = "";

    //Userlist actions
    QAction *copy_text   = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy"), NULL);
    QAction *copy_nick   = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy nick"), NULL);
    QAction *find        = new QAction(WU->getPixmap(WulforUtil::eiFIND), tr("Show in list"), NULL);
    QAction *browse      = new QAction(WU->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Browse files"), NULL);
    QAction *match_queue = new QAction(WU->getPixmap(WulforUtil::eiDOWN), tr("Match Queue"), NULL);
    QAction *private_msg = new QAction(WU->getPixmap(WulforUtil::eiMESSAGE), tr("Private Message"), NULL);
    QAction *fav_add     = new QAction(WU->getPixmap(WulforUtil::eiFAVADD), tr("Add to Favorites"), NULL);
    QAction *fav_del     = new QAction(WU->getPixmap(WulforUtil::eiFAVREM), tr("Remove from Favorites"), NULL);
    QAction *grant_slot  = new QAction(WU->getPixmap(WulforUtil::eiEDITADD), tr("Grant slot"), NULL);
    QAction *rem_queue   = new QAction(WU->getPixmap(WulforUtil::eiEDITDELETE), tr("Remove from Queue"), NULL);

    //Chat actions
    QAction *sep1        = new QAction(NULL);
    QAction *clear_chat  = new QAction(WU->getPixmap(WulforUtil::eiCLEAR), tr("Clear chat"), NULL);
    QAction *find_in_chat= new QAction(WU->getPixmap(WulforUtil::eiFIND), tr("Find in chat"), NULL);
    QAction *dis_chat    = new QAction(WU->getPixmap(WulforUtil::eiFILECLOSE), tr("Disable/Enable chat"), NULL);
    QAction *sep2        = new QAction(NULL);
    QAction *select_all  = new QAction(tr("Select all"), NULL);
    QAction *sep3        = new QAction(NULL);
    QAction *zoom_in     = new QAction(WU->getPixmap(WulforUtil::eiZOOM_IN), tr("Zoom In"), NULL);
    QAction *zoom_out    = new QAction(WU->getPixmap(WulforUtil::eiZOOM_OUT), tr("Zoom Out"), NULL);

    sep1->setSeparator(true), sep2->setSeparator(true), sep3->setSeparator(true);

    actions << copy_text
            << copy_nick
            << find
            << browse
            << match_queue
            << private_msg
            << fav_add
            << fav_del
            << grant_slot
            << rem_queue;

    pm_actions << copy_text
            << copy_nick
            << browse
            << match_queue
            << private_msg
            << fav_add
            << fav_del
            << grant_slot
            << rem_queue;

    ul_actions << browse
            << private_msg
            << fav_add
            << fav_del
            << grant_slot
            << copy_text
            << copy_nick
            << match_queue
            << rem_queue;

    chat_actions << sep1
                 << clear_chat
                 << find_in_chat
                 << dis_chat
                 << sep2
                 << select_all
                 << sep3
                 << zoom_in
                 << zoom_out;

    pm_chat_actions << sep1
                 << clear_chat
                 << sep2
                 << select_all
                 << sep3
                 << zoom_in
                 << zoom_out;

    chat_actions_map.insert(clear_chat, ClearChat);
    chat_actions_map.insert(find_in_chat, FindInChat);
    chat_actions_map.insert(dis_chat, DisableChat);
    chat_actions_map.insert(select_all, SelectAllChat);
    chat_actions_map.insert(zoom_in, ZoomInChat);
    chat_actions_map.insert(zoom_out, ZoomOutChat);
}


HubFrame::Menu::~Menu(){
    delete menu;

    qDeleteAll(chat_actions);
    qDeleteAll(actions);
}

HubFrame::Menu::Action HubFrame::Menu::execUserMenu(Client *client, const QString &cid = QString()){
    if (!client)
        return None;

    menu->clear();

    menu->setTitle("");

    menu->addActions(ul_actions);

    QMenu *user_menu = NULL;

    if (!cid.isEmpty()){
        user_menu = buildUserCmdMenu(_q(client->getHubUrl()));
        menu->addMenu(user_menu);
    }

    QAction *res = menu->exec(QCursor::pos());

    if (actions.contains(res)){
        delete user_menu;

        return static_cast<Action>(actions.indexOf(res));
    }
    else if (res && !res->toolTip().isEmpty()){//User command{
        last_user_cmd = res->toolTip();
        QString cmd_name = res->statusTip();
        QString hub = res->data().toString();

        delete user_menu;

        int id = FavoriteManager::getInstance()->findUserCommand(cmd_name.toStdString(), hub.toStdString());
        UserCommand uc;

        if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
            return None;

        StringMap params;

        if (WulforUtil::getInstance()->getUserCommandParams(last_user_cmd, params)){
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

            if (user)
                ClientManager::getInstance()->userCommand(user, uc, params, true);

            return UserCommands;
        }
        else
            return None;
    }
    else{
        delete user_menu;

        return None;
    }
}

QString HubFrame::Menu::getLastUserCmd() const{
    return last_user_cmd;
}

HubFrame::Menu::Action HubFrame::Menu::execChatMenu(Client *client, const QString &cid = QString(), bool pmw = false){
    if (!client)
        return None;

    menu->clear();

    QAction *title = new QAction(WulforUtil::getInstance()->getNicks(cid), menu);
    QFont f;
    f.setBold(true);
    title->setFont(f);
    title->setEnabled(false);

    menu->addAction(title);

    if(pmw){
        menu->addActions(pm_actions);
        menu->addActions(pm_chat_actions);
    }
    else{
        menu->addActions(actions);
        menu->addActions(chat_actions);
    }

    QMenu *user_menu = NULL;

    if (!cid.isEmpty() && !pmw){
        user_menu = buildUserCmdMenu(_q(client->getHubUrl()));
        menu->addMenu(user_menu);
    }

    QAction *res = menu->exec(QCursor::pos());

    title->deleteLater();

    if (actions.contains(res)){
        delete user_menu;
        return static_cast<Action>(actions.indexOf(res));
    }
    else if (chat_actions_map.contains(res)){
        delete user_menu;

        return chat_actions_map[res];
    }
    else if (res && !res->toolTip().isEmpty()){//User command
        last_user_cmd = res->toolTip();
        QString cmd_name = res->statusTip();
        QString hub = res->data().toString();

        delete user_menu;

        int id = FavoriteManager::getInstance()->findUserCommand(cmd_name.toStdString(), hub.toStdString());
        UserCommand uc;

        if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
            return None;

        StringMap params;

        if (WulforUtil::getInstance()->getUserCommandParams(last_user_cmd, params)){
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid.toStdString()));

            if (user)
                ClientManager::getInstance()->userCommand(user, uc, params, true);

            return UserCommands;
        }
        else
            return None;
    }
    else{
        delete user_menu;
        return None;
    }
}

QMenu *HubFrame::Menu::buildUserCmdMenu(const QString &hub){
    if (hub.isEmpty())
        return NULL;

    QMenu *menu = new QMenu();
    menu->setTitle(tr("Commands"));

    QMenu *tmp = WulforUtil::getInstance()->buildUserCmdMenu(QStringList() << hub, UserCommand::CONTEXT_CHAT);

    if (tmp)
        menu->addMenu(tmp);

    return menu;
}

QString HubFrame::LinkParser::parseForLinks(QString input, bool use_emot){
    if (input.isEmpty() || input.isNull())
        return input;

    input.replace("<a href=","&lt;a href=");
    input.replace("</a>","&lt;/a&gt;");
    input.replace("<img alt=","&lt;img alt=");
    input.replace("\" />","\" /&gt;");

    QString output = "";

    while (!input.isEmpty()){
        for (int j = 0; j < link_types.size(); j++){
            QString linktype = link_types.at(j);

            if (input.startsWith(linktype)){
                int l_pos = linktype.length();

                while (l_pos < input.size()){
                    QChar ch = input.at(l_pos);

                    if (ch.isSpace() || ch == '\n'  ||
                        ch == '>'    || ch == '<'){
                        break;
                    }
                    else
                        l_pos++;
                }

                QString link = input.left(l_pos);
                QString toshow = link;

                if (linktype == "http://"  || linktype == "https://" || linktype == "ftp://")
                    toshow = QUrl::fromEncoded(link.toAscii()).toString();

                if (linktype == "magnet:"){
                    QUrl url;
                    toshow = link;
                    toshow.replace("+", "%20");
                    toshow.replace("!", "%21");
                    url.setEncodedUrl(toshow.toAscii());

                    if (url.hasQueryItem("dn")) {
                        toshow = url.queryItemValue("dn");

                        if (url.hasQueryItem("xl"))
                            toshow += " (" + WulforUtil::formatBytes(url.queryItemValue("xl").toULongLong()) + ")";
                    }
                }

                if (linktype == "www.")
                    toshow.prepend("http://");

                QString html_link = "";

                if (linktype != "magnet:")
                    html_link = QString("<a href=\"%1\" title=\"%1\" style=\"cursor: hand\">%1</a>").arg(toshow);
                else
                    html_link = QString("<a href=\"%1\" title=\"%2\" style=\"cursor: hand\">%2</a>").arg(link).arg(toshow);

                output += html_link;
                input.remove(0, l_pos);
            }

            if (input.isEmpty())
                break;
        }

        if (input.isEmpty())
            break;

        output += input.at(0);

        input.remove(0, 1);
    }

    if (use_emot && WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        output = EmoticonFactory::getInstance()->convertEmoticons(output);

    QString out = "";
    QString buf = output;

    while (!buf.isEmpty()){
        if (buf.startsWith("<a href=") && buf.indexOf("</a>") > 0){
            QString add = buf.left(buf.indexOf("</a>")) + "</a>";

            out += add;
            buf.remove(0, add.length());
        }
        else if (buf.startsWith("<img alt=") && buf.indexOf("\" />") > 0){
            QString add = buf.left(buf.indexOf("\" />")) + "\" />";

            out += add;
            buf.remove(0, add.length());
        }
        else if (buf.startsWith("&lt;a href=")){
            out += "&lt;a href=";
            buf.remove(0, QString("&lt;a href=").length());
        }
        else if (buf.startsWith("&lt;/a&gt;")){
            out += "&lt;/a&gt;";
            buf.remove(0, QString("&lt;/a&gt;").length());
        }
        else if (buf.startsWith("&lt;img alt=")){
            out += "&lt;img alt=";
            buf.remove(0, QString("&lt;img alt=").length());
        }
        else if (buf.startsWith("\" /&gt;")){
            out += "\" /&gt;";
            buf.remove(0, QString("\" /&gt;").length());
        }
        else if (buf.startsWith(";")){
            out += "&#59;";
            buf.remove(0, 1);
        }
        else if (buf.startsWith("<")){
            out += "&lt;";
            buf.remove(0, 1);
        }
        else if (buf.startsWith(">")){
            out += "&gt;";
            buf.remove(0, 1);
        }
        else if (buf.startsWith(' ')){
            if (out.endsWith(" "))
                out += "&nbsp;";
            else
                out += ' ';

            buf.remove(0, 1);
        }
        else{
            out += buf.at(0);
            buf.remove(0, 1);
        }
    }

    output = out;

    return output;
}

HubFrame::HubFrame(QWidget *parent=NULL, QString hub="", QString encoding=""):
        QWidget(parent),
        total_shared(0),
        arenaMenu(NULL),
        codec(NULL),
        chatDisabled(false),
        hasMessages(false),
        client(NULL)
{
    setupUi(this);

    if (!Menu::getInstance())
        Menu::newInstance();

    Menu::counter++;

    client = ClientManager::getInstance()->getClient(hub.toStdString());
    client->addListener(this);

    QString enc = WulforUtil::getInstance()->qtEnc2DcEnc(encoding);

    if (enc.isEmpty())
        enc = WulforUtil::getInstance()->qtEnc2DcEnc(WSGET(WS_DEFAULT_LOCALE));

    if (enc.indexOf(" ") > 0){
        enc = enc.left(enc.indexOf(" "));
        enc.replace(" ", "");
    }

    client->setEncoding(enc.toStdString());

    codec = WulforUtil::getInstance()->codecForEncoding(encoding);

    init();

    FavoriteHubEntry* entry = FavoriteManager::getInstance()->getFavoriteHubEntry(_tq(hub));

    if (entry && entry->getDisableChat())
        disableChat();

    client->connect();

    setAttribute(Qt::WA_DeleteOnClose);

    out_messages_index = 0;
}


HubFrame::~HubFrame(){
    Menu::counter--;

    if (!Menu::counter)
        Menu::deleteInstance();

    delete model;
    delete proxy;

    delete updater;

#if HAVE_MALLOC_TRIM
    malloc_trim(0);
#endif
}

bool HubFrame::eventFilter(QObject *obj, QEvent *e){
    if (e->type() == QEvent::KeyRelease){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) &&
            (k_e->modifiers() != Qt::ShiftModifier))
        {
            return true;
        }
        else if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) && k_e->key() == Qt::Key_Tab){
            nickCompletion();

            e->accept();

            return true;
        }
        else if (static_cast<QLineEdit*>(obj) == lineEdit_FIND){
            bool ret = QWidget::eventFilter(obj, e);

            if (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return)
                slotFindForward();

            return ret;
        }

    }
    else if (e->type() == QEvent::KeyPress){
        QKeyEvent *k_e = reinterpret_cast<QKeyEvent*>(e);

        if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (!WBGET(WB_USE_CTRL_ENTER) || k_e->modifiers() == Qt::ControlModifier) &&
            ((k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) && k_e->modifiers() != Qt::ShiftModifier) ||
             (k_e->key() == Qt::Key_Enter && k_e->modifiers() == Qt::KeypadModifier))
        {
            sendChat(plainTextEdit_INPUT->toPlainText(), false, false);

            plainTextEdit_INPUT->setPlainText("");

            return true;
        }
        else if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) && k_e->key() == Qt::Key_Tab)
            return true;

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
    else if (e->type() == QEvent::MouseButtonPress){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        bool isChat = (static_cast<QWidget*>(obj) == textEdit_CHAT->viewport());
        bool isUserList = (static_cast<QWidget*>(obj) == treeView_USERS->viewport());

        if (isChat)
            textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        if (isChat && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

            if (!WulforUtil::getInstance()->openUrl(pressedParagraph)){
                /**
                  Do nothing
                */
            }
        }
        else if ((isChat || isUserList) && m_e->button() == Qt::MidButton)
        {
            QString nick = "";
            QString cid = "";

            if (isChat){
                QTextCursor cursor = textEdit_CHAT->textCursor();
                QString pressedParagraph = cursor.block().text();

                int l = pressedParagraph.indexOf("<");
                int r = pressedParagraph.indexOf(">");

                if (l < r){
                    nick = pressedParagraph.mid(l+1, r-l-1);
                    cid = model->CIDforNick(nick);
                }
            }
            else if (isUserList){
                QModelIndex index = treeView_USERS->indexAt(treeView_USERS->viewport()->mapFromGlobal(QCursor::pos()));

                if (treeView_USERS->model() == proxy)
                    index = proxy->mapToSource(index);

                if (index.isValid()){
                    UserListItem *i = reinterpret_cast<UserListItem*>(index.internalPointer());

                    nick = i->nick;
                    cid = i->cid;
                }
            }

            if (!cid.isEmpty()){
                if (WIGET(WI_CHAT_MDLCLICK_ACT) == 0){
                    if (plainTextEdit_INPUT->textCursor().position() == 0)
                        plainTextEdit_INPUT->textCursor().insertText(nick+ ": ");
                    else
                        plainTextEdit_INPUT->textCursor().insertText(nick+ " ");

                    plainTextEdit_INPUT->setFocus();
                }
                else if (WIGET(WI_CHAT_DBLCLICK_ACT) == 2)
                    addPM(cid, "");
                else
                    browseUserFiles(cid, false);
            }
        }
    }
    else if (e->type() == QEvent::MouseButtonDblClick){
        bool isChat = (static_cast<QWidget*>(obj) == textEdit_CHAT->viewport());
        bool isUserList = (static_cast<QWidget*>(obj) == treeView_USERS->viewport());

        if (isChat || isUserList){
            QString nick = "";
            QString cid = "";
            QTextCursor cursor = textEdit_CHAT->textCursor();

            if (isChat){
                QString pressedParagraph = cursor.block().text();

                int l = pressedParagraph.indexOf("<");
                int r = pressedParagraph.indexOf(">");

                if (l < r){
                    nick = pressedParagraph.mid(l+1, r-l-1);
                    cid = model->CIDforNick(nick);
                }
            }
            else if (isUserList){
                QModelIndex index = treeView_USERS->indexAt(treeView_USERS->viewport()->mapFromGlobal(QCursor::pos()));

                if (treeView_USERS->model() == proxy)
                    index = proxy->mapToSource(index);

                if (index.isValid()){
                    UserListItem *i = reinterpret_cast<UserListItem*>(index.internalPointer());

                    nick = i->nick;
                    cid = i->cid;
                }
            }

            if (!cid.isEmpty()){
                if (WIGET(WI_CHAT_DBLCLICK_ACT) == 1)
                    browseUserFiles(cid, false);
                else if (WIGET(WI_CHAT_DBLCLICK_ACT) == 2)
                    addPM(cid, "");
                else if (textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos())).startsWith("user://") || isUserList){//may be dbl click on user nick
                    if (plainTextEdit_INPUT->textCursor().position() == 0)
                        plainTextEdit_INPUT->textCursor().insertText(nick+ ": ");
                    else
                        plainTextEdit_INPUT->textCursor().insertText(nick+ " ");

                    plainTextEdit_INPUT->setFocus();
                }
            }
        }
    }
    else if (e->type() == QEvent::MouseMove && (static_cast<QWidget*>(obj) == textEdit_CHAT->viewport())){
        if (!textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos())).isEmpty())
            textEdit_CHAT->viewport()->setCursor(Qt::PointingHandCursor);
        else
            textEdit_CHAT->viewport()->setCursor(Qt::IBeamCursor);
    }

    return QWidget::eventFilter(obj, e);
}

void HubFrame::customEvent(QEvent *e){
    if (e->type() == UserUpdatedEvent::Event){
        UserUpdatedEvent *u_e = reinterpret_cast<UserUpdatedEvent*>(e);

        on_userUpdated(u_e->getMap(), u_e->getUser(), u_e->getJoin());
    }
    else if (e->type() == UserRemovedEvent::Event){
        UserRemovedEvent *u_e = reinterpret_cast<UserRemovedEvent*>(e);

        total_shared -= u_e->getShare();

        QString cid = _q(u_e->getUser()->getCID().toBase32());
        if (pm.contains(cid)){
            pmUserOffline(cid);

            QString nick = model->itemForPtr(u_e->getUser())->nick;
            PMWindow *pmw = pm[cid];

            pm.insert(nick, pmw);

            pmw->cid = nick;
            pmw->plainTextEdit_INPUT->setEnabled(false);//we need interface function

            pm.remove(cid);
        }

        model->removeUser(u_e->getUser());
    }
    else if (e->type() == UserCustomEvent::Event){
        UserCustomEvent *u_e = reinterpret_cast<UserCustomEvent*>(e);

        u_e->func()->call();
    }

    e->accept();
}

void HubFrame::closeEvent(QCloseEvent *e){
    MainWindow *MW = MainWindow::getInstance();

    MW->remArenaWidgetFromToolbar(this);
    MW->remWidgetFromArena(this);
    MW->remArenaWidget(this);

    client->removeListener(this);
    HubManager::getInstance()->unregisterHubUrl(_q(client->getHubUrl()));
    client->disconnect(true);
    ClientManager::getInstance()->putClient(client);

    updater->stop();

    save();

    blockSignals(true);

    PMMap::const_iterator it = pm.constBegin();

    for (; it != pm.constEnd(); ++it){
        PMWindow *w = const_cast<PMWindow*>(it.value());

        disconnect(w, SIGNAL(privateMessageClosed(QString)), this, SLOT(slotPMClosed(QString)));

        w->close();
    }

    pm.clear();

    foreach (ShellCommandRunner *r, shell_list){
        r->cancel();
        r->exit(0);

        r->wait(100);

        if (r->isRunning())
            r->terminate();

        delete r;
    }

    blockSignals(false);

    if (isVisible())
        HubManager::getInstance()->setActiveHub(NULL);

    setAttribute(Qt::WA_DeleteOnClose);

    e->accept();
}

void HubFrame::showEvent(QShowEvent *e){
    e->accept();

    HubManager::getInstance()->setActiveHub(this);

    hasMessages = false;
    MainWindow::getInstance()->redrawToolPanel();
}

void HubFrame::hideEvent(QHideEvent *e){
    e->accept();

    if (!isVisible())
        HubManager::getInstance()->setActiveHub(NULL);
}

void HubFrame::init(){
    updater = new QTimer();
    updater->setInterval(1000);
    updater->setSingleShot(false);

    model = new UserListModel(this);
    proxy = NULL;

    treeView_USERS->setModel(model);
    treeView_USERS->setSortingEnabled(true);
    treeView_USERS->setItemsExpandable(false);
    treeView_USERS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_USERS->header()->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_USERS->viewport()->installEventFilter(this);

    installEventFilter(this);
    lineEdit_FILTER->installEventFilter(this);
    lineEdit_FIND->installEventFilter(this);

    textEdit_CHAT->document()->setMaximumBlockCount(WIGET(WI_CHAT_MAXPARAGRAPHS));
    textEdit_CHAT->setContextMenuPolicy(Qt::CustomContextMenu);
    textEdit_CHAT->setReadOnly(true);
    textEdit_CHAT->setAutoFormatting(QTextEdit::AutoNone);
    textEdit_CHAT->viewport()->installEventFilter(this);//QTextEdit don't receive all mouse events
    textEdit_CHAT->setMouseTracking(true);

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_CHAT->document());

    frame->setVisible(false);

    for (int i = 0; i < model->columnCount(); i++)
        comboBox_COLUMNS->addItem(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());

    toolButton_SMILE->setVisible(WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance());
    toolButton_SMILE->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEMOTICON));

    toolButton_CLEAR_FILTER->setIcon(WulforUtil::getInstance()->getPixmap(WulforUtil::eiERASER));

    connect(label_LAST_STATUS, SIGNAL(linkActivated(QString)), this, SLOT(slotStatusLinkOpen(QString)));
    connect(treeView_USERS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotUserListMenu(QPoint)));
    connect(treeView_USERS->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(updater, SIGNAL(timeout()), this, SLOT(slotUsersUpdated()));
    connect(textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));
    connect(toolButton_BACK, SIGNAL(clicked()), this, SLOT(slotFindBackward()));
    connect(toolButton_FORWARD, SIGNAL(clicked()), this, SLOT(slotFindForward()));
    connect(toolButton_HIDE, SIGNAL(clicked()), this, SLOT(slotHideFindFrame()));
    connect(lineEdit_FIND, SIGNAL(textEdited(QString)), this, SLOT(slotFindTextEdited(QString)));
    connect(lineEdit_FILTER, SIGNAL(textChanged(QString)), this, SLOT(slotFilterTextChanged()));
    connect(comboBox_COLUMNS, SIGNAL(activated(int)), this, SLOT(slotFilterTextChanged()));
    connect(toolButton_SMILE, SIGNAL(clicked()), this, SLOT(slotSmile()));
    connect(pushButton_ALL, SIGNAL(clicked()), this, SLOT(slotFindAll()));

#ifdef USE_ASPELL
    connect(plainTextEdit_INPUT, SIGNAL(textChanged()), this, SLOT(slotInputTextChanged()));
    connect(plainTextEdit_INPUT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotInputContextMenu()));

    plainTextEdit_INPUT->setContextMenuPolicy(Qt::CustomContextMenu);
#endif

    plainTextEdit_INPUT->setWordWrapMode(QTextOption::NoWrap);
    plainTextEdit_INPUT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    plainTextEdit_INPUT->installEventFilter(this);

    initMenu();

    load();

    updater->start();
}

void HubFrame::initMenu(){
    WulforUtil *WU = WulforUtil::getInstance();

    delete arenaMenu;

    arenaMenu = new QMenu(tr("Hub menu"), this);

    QAction *reconnect = new QAction(WU->getPixmap(WulforUtil::eiRECONNECT), tr("Reconnect"), arenaMenu);
    QAction *show_wnd  = new QAction(WU->getPixmap(WulforUtil::eiCHAT), tr("Show widget"), arenaMenu);
    QAction *sep       = new QAction(arenaMenu);
    sep->setSeparator(true);
    QAction *close_wnd = new QAction(WU->getPixmap(WulforUtil::eiEXIT), tr("Close"), arenaMenu);

    arenaMenu->addActions(QList<QAction*>() << reconnect
                                            << show_wnd
                         );

    if (client && client->isConnected()){
        QMenu *u_c = WulforUtil::getInstance()->buildUserCmdMenu(QList<QString>() << _q(client->getHubUrl()), UserCommand::CONTEXT_HUB);

        if (u_c){
            u_c->setTitle(tr("Hub Menu"));

            arenaMenu->addMenu(u_c);

            connect(u_c, SIGNAL(triggered(QAction*)), this, SLOT(slotHubMenu(QAction*)));
        }
    }

    arenaMenu->addActions(QList<QAction*>() << sep << close_wnd);

    connect(reconnect, SIGNAL(triggered()), this, SLOT(slotReconnect()));
    connect(show_wnd, SIGNAL(triggered()), this, SLOT(slotShowWnd()));
    connect(close_wnd, SIGNAL(triggered()), this, SLOT(slotClose()));
}


void HubFrame::save(){
    WSSET(WS_CHAT_USERLIST_STATE, treeView_USERS->header()->saveState().toBase64());
    WISET(WI_CHAT_WIDTH, textEdit_CHAT->width());
    WISET(WI_CHAT_USERLIST_WIDTH, treeView_USERS->width());
    WISET(WI_CHAT_SORT_COLUMN, model->getSortColumn());
    WISET(WI_CHAT_SORT_ORDER, WulforUtil::getInstance()->sortOrderToInt(model->getSortOrder()));
}

void HubFrame::load(){
    int w_chat = WIGET(WI_CHAT_WIDTH), w_ulist = WIGET(WI_CHAT_USERLIST_WIDTH);

    QString ustate = WSGET(WS_CHAT_USERLIST_STATE);

    if (!ustate.isEmpty())
        treeView_USERS->header()->restoreState(QByteArray::fromBase64(ustate.toAscii()));

    if (w_chat >= 0 && w_ulist >= 0){
        QList<int> frames;

        frames << w_chat << w_ulist;

        splitter_2->setSizes(frames);
    }

    treeView_USERS->sortByColumn(WIGET(WI_CHAT_SORT_COLUMN), WulforUtil::getInstance()->intToSortOrder(WIGET(WI_CHAT_SORT_ORDER)));

    reloadSomeSettings();
}

void HubFrame::reloadSomeSettings(){
    plainTextEdit_INPUT->setMaximumHeight(WIGET(WI_TEXT_EDIT_HEIGHT));

    label_USERSTATE->setVisible(WBGET(WB_USERS_STATISTICS));

    label_LAST_STATUS->setVisible(WBGET(WB_LAST_STATUS));
}

QWidget *HubFrame::getWidget(){
    return this;
}

QString HubFrame::getArenaTitle(){
    QString ret = tr("Not connected");

    if (client && client->isConnected()){
        ret  = QString("%1 - %2 [%3]").arg(QString(client->getHubName().c_str()))
                                      .arg(QString(client->getHubDescription().c_str()))
                                      .arg(QString(client->getIp().c_str()));
        QString prefix = QString("[+%1] ").arg(client->isSecure()? ("S") : (client->isTrusted()? ("T"): ("")));

        ret.prepend(prefix);
    }
    else if (client){
        ret = QString("[-] %1").arg(client->getHubUrl().c_str());
    }

    return ret;
}

QString HubFrame::getArenaShortTitle(){
    QString ret = tr("Not connected");

    if (client && client->isConnected()){
        ret = QString("[+] %1").arg(QString(client->getHubName().c_str()));
    }
    else if (client){
        ret = QString("[-] %1").arg(client->getHubUrl().c_str());
    }

    return ret;
}

QMenu *HubFrame::getMenu(){
    initMenu();

    return arenaMenu;
}

const QPixmap &HubFrame::getPixmap(){
    if (hasMessages)
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiMESSAGE);
    else
        return WulforUtil::getInstance()->getPixmap(WulforUtil::eiSERVER);
}

void HubFrame::clearChat(){
    textEdit_CHAT->setHtml("");

    addStatus(tr("Chat cleared."));

    if (WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->addEmoticons(textEdit_CHAT->document());
}

void HubFrame::disableChat(){
    if (!chatDisabled){
        addStatus(tr("Chat disabled."));

        chatDisabled = true;
    }
    else{
        chatDisabled = false;

        addStatus(tr("Chat enabled."));
    }

    plainTextEdit_INPUT->setEnabled(!chatDisabled);
    frame_INPUT->setVisible(!chatDisabled);
}


QString HubFrame::getUserInfo(UserListItem *item){
    QString ttip = "";

    ttip += model->headerData(COLUMN_NICK, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->nick + "\n";
    ttip += model->headerData(COLUMN_COMMENT, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->comm + "\n";
    ttip += model->headerData(COLUMN_EMAIL, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->email + "\n";
    ttip += model->headerData(COLUMN_IP, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->ip + "\n";
    ttip += model->headerData(COLUMN_SHARE, Qt::Horizontal, Qt::DisplayRole).toString() + ": " +
            WulforUtil::formatBytes(item->share) + "\n";
    ttip += model->headerData(COLUMN_TAG, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->tag + "\n";
    ttip += model->headerData(COLUMN_CONN, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->conn + "\n";

    if (item->isOp)
        ttip += tr("Hub role: Operator");
    else
        ttip += tr("Hub role: User");

    if (FavoriteManager::getInstance()->isFavoriteUser(item->ptr))
        ttip += tr("\nFavorite user");

    return ttip;
}

void HubFrame::sendChat(QString msg, bool thirdPerson, bool stripNewLines){
    if (!client || !client->isConnected() || msg.isEmpty() || msg.isNull())
        return;

    if (stripNewLines)
        msg.replace("\n", "");

    if (msg.trimmed().isEmpty())
        return;

    if (msg.endsWith("\n"))
        msg = msg.left(msg.lastIndexOf("\n"));

    if (!parseForCmd(msg, this))
        client->hubMessage(Text::toUtf8(msg.toStdString()), thirdPerson);

    if (!thirdPerson){
        out_messages << msg;

        if (out_messages.size() >= WIGET(WI_OUT_IN_HIST))
            out_messages.removeAt(0);

        out_messages_index = out_messages.size()-1;
    }
}

bool HubFrame::parseForCmd(QString line, QWidget *wg){
    HubFrame *fr = qobject_cast<HubFrame *>(wg);
    PMWindow *pm = qobject_cast<PMWindow *>(wg);

    QStringList list = line.split(" ", QString::SkipEmptyParts);

    if (list.size() == 0)
        return false;

    if (!line.startsWith("/"))
        return false;

    QString cmd = list.at(0);
    QString param = "";
    bool emptyParam = true;

    if (list.size() > 1){
        param = list.at(1);
        emptyParam = false;
    }

    if (cmd == "/away"){
        if (Util::getAway() && emptyParam){
            Util::setAway(false);
            Util::setManualAway(false);

            if (fr == this)
                addStatus(tr("Away mode off"));
            else if (pm)
                pm->addStatus(tr("Away mode off"));
        }
        else {
            Util::setAway(true);
            Util::setManualAway(true);

            if (!emptyParam){
                line.remove(0, 6);
                Util::setAwayMessage(line.toStdString());
            }

            if (fr == this)
                addStatus(tr("Away mode on: ") + QString::fromStdString(Util::getAwayMessage()));
            else if (pm)
                pm->addStatus(tr("Away mode on: ") + QString::fromStdString(Util::getAwayMessage()));
        }
    }
    else if (cmd == "/alias" && !emptyParam){
        QStringList lex = line.split(" ", QString::SkipEmptyParts);

        if (lex.size() >= 2){
            QString aliases = QByteArray::fromBase64(WSGET(WS_CHAT_CMD_ALIASES).toAscii());

            if (lex.at(1) == "list"){
                if (!aliases.isEmpty()){
                    if (fr == this)
                        addStatus("\n"+aliases);
                    else if (pm)
                        pm->addStatus("\n"+aliases);
                }
                else{
                    if (fr == this)
                        addStatus(tr("Aliases not found."));
                    else if (pm)
                        pm->addStatus(tr("Aliases not found."));
                }
            }
            else if (lex.at(1) == "purge" && lex.size() == 3){
                QString alias = lex.at(2);
                QStringList alias_list = aliases.split('\n', QString::SkipEmptyParts);

                foreach(QString line, alias_list){
                    QStringList cmds = line.split('\t', QString::SkipEmptyParts);

                    if (cmds.size() == 2 && alias == cmds.at(0)){
                        alias_list.removeAt(alias_list.indexOf(line));

                        QString new_aliases;
                        foreach (QString line, alias_list)
                            new_aliases += line + "\n";

                        WSSET(WS_CHAT_CMD_ALIASES, new_aliases.toAscii().toBase64());

                        if (fr == this)
                            addStatus(tr("Alias removed."));
                        else if (pm)
                            pm->addStatus(tr("Alias removed."));
                    }
                }
            }
            else if (lex.size() >= 2){
                QString raw = line;

                raw.remove(0, raw.indexOf(" ")+1);

                if (raw.indexOf("::") <= 0){
                    if (fr == this)
                        addStatus(tr("Invalid alias syntax."));
                    else if (pm)
                        pm->addStatus(tr("Invalid alias syntax."));
                }
                else {
                    QStringList new_cmd = raw.split("::", QString::SkipEmptyParts);

                    if (new_cmd.size() < 2 || new_cmd.at(1).isEmpty()){
                        if (fr == this)
                            addStatus(tr("Invalid alias syntax."));
                        else if (pm)
                            pm->addStatus(tr("Invalid alias syntax."));
                    }
                    else if (!aliases.contains(new_cmd.at(0)+'\t')){
                        aliases += new_cmd.at(0) + '\t' +  new_cmd.at(1) + '\n';

                        WSSET(WS_CHAT_CMD_ALIASES, aliases.toAscii().toBase64());

                        if (fr == this)
                            addStatus(tr("Alias %1 => %2 has been added").arg(new_cmd.at(0)).arg(new_cmd.at(1)));
                        else if (pm)
                            pm->addStatus(tr("Alias %1 => %2 has been added").arg(new_cmd.at(0)).arg(new_cmd.at(1)));
                    }
                }
            }
        }
    }
#ifdef USE_ASPELL
    else if (cmd == "/aspell" && !emptyParam){
        WBSET(WB_APP_ENABLE_ASPELL, param.trimmed() == "on");

        if (WBGET(WB_APP_ENABLE_ASPELL) && !SpellCheck::getInstance())
            SpellCheck::newInstance();
        else if (SpellCheck::getInstance())
            SpellCheck::deleteInstance();

        if (fr == this)
            addStatus(tr("Aspell switched %1").arg((WBGET(WB_APP_ENABLE_ASPELL)? tr("on") : tr("off"))) );
        else if (pm)
            pm->addStatus(tr("Aspell switched %1").arg((WBGET(WB_APP_ENABLE_ASPELL)? tr("on") : tr("off"))) );
    }
#endif
    else if (cmd == "/back"){
        Util::setAway(false);

        if (fr == this)
            addStatus(tr("Away mode off"));
        else if (pm)
            pm->addStatus(tr("Away mode off"));
    }
    else if (cmd == "/clear"){
        textEdit_CHAT->setHtml("");

        if (fr == this)
            addStatus(tr("Chat has been cleared"));
        else if (pm)
            pm->addStatus(tr("Chat has been cleared"));

    }
    else if (cmd == "/close"){
        this->close();
    }
    else if (cmd == "/fav"){
        addAsFavorite();
    }
    else if (cmd == "/browse" && !emptyParam){
        browseUserFiles(model->CIDforNick(param), false);
    }
    else if (cmd == "/grant" && !emptyParam){
        grantSlot(model->CIDforNick(param));
    }
    else if (cmd == "/magnet" && !emptyParam){
        WISET(WI_DEF_MAGNET_ACTION, param.toInt());
    }
    else if (cmd == "/info" && !emptyParam){
        UserListItem *item = model->itemForNick(param);

        if (item){
            QString ttip = "\n" + getUserInfo(item);

            if (fr == this)
                addStatus(ttip);
            else if (pm)
                pm->addStatus(ttip);
        }
    }
    else if (cmd == "/me" && !emptyParam){
        if (line.endsWith("\n"))//remove extra \n char
            line = line.left(line.lastIndexOf("\n"));

        // This is temporary. It is need to check ClientManager::privateMessage(...) function
        // in dcpp kernel with version > 0.75. And "if (fr == this)" will not be needed.
        if (fr == this)
            line.remove(0, 4);

        if (fr == this)
            sendChat(line, true, false);
        else if (pm)
            pm->sendMessage(line, false, false);
        // This is temporary. It is need to check ClientManager::privateMessage(...) function
        // in dcpp kernel with version > 0.75. And "pm->sendMessage(line, true, false);" will be here.
    }
    else if (cmd == "/pm" && !emptyParam){
        addPM(model->CIDforNick(param), "");
    }
    else if (cmd == "/help" || cmd == "/?" || cmd == "/h"){
        QString out = "\n" +
                      tr(""
#ifdef USE_ASPELL
                         "/aspell on/off - enable/disable spell checking\n"
#endif
                         "/alias <ALIAS_NAME>::<COMMAND> - make alias /ALIAS_NAME to /COMMAND\n"
                         "/alias purge <ALIAS_NAME> - remove alias\n"
                         "/alias list - list all aliases\n"
                         "/away <message> - set away-mode on/off\n"
                         "/back - set away-mode off\n"
                         "/browse <nick> - browse user files\n"
                         "/clear - clear chat window\n"
                         "/magnet - default action with magnet (0-ask, 1-search, 2-download)\n"
                         "/close - close this hub\n"
                         "/fav - add this hub to favorites\n"
                         "/grant <nick> - grant extra slot to user\n"
                         "/help, /?, /h - show this help\n"
                         "/info <nick> - show info about user\n"
                         "/me - say a third person\n"
                         "/pm <nick> - begin private chat with user\n"
                         "/sh <command> - start command and redirect output to the chat");

        if (fr == this)
            addStatus(out);
        else if (pm)
            pm->addStatus(out);
    }
    else if (cmd == "/sh" && !emptyParam){
        if (line.endsWith("\n"))//remove extra \n char
            line = line.left(line.lastIndexOf("\n"));

        line = line.remove(0, 4);

        ShellCommandRunner *sh = new ShellCommandRunner(line, wg);
        connect(sh, SIGNAL(finished(bool,QString)), this, SLOT(slotShellFinished(bool,QString)));

        shell_list.append(sh);

        sh->start();
    }
    else if (cmd == "/ws" && !emptyParam){
        line = line.remove(0, 4);
        line.replace("\n", "");

        WSCMD(line);
    }
    else if (!WSGET(WS_CHAT_CMD_ALIASES).isEmpty()){
        QString aliases = QByteArray::fromBase64(WSGET(WS_CHAT_CMD_ALIASES).toAscii());
        QStringList alias_list = aliases.split('\n', QString::SkipEmptyParts);
        bool ok = false;

        foreach(QString line, alias_list){
            QStringList cmds = line.split('\t', QString::SkipEmptyParts);

            if (cmds.size() == 2 && cmd == ("/" + cmds.at(0))){
                parseForCmd(cmds.at(1), wg);

                ok = true;
            }
        }

        if (!ok)
            return ok;
    }
    else
        return false;

    return true;
}

void HubFrame::addStatus(QString msg){
    if (chatDisabled)
        return;

    QString pure_msg = msg;
    QString status = "";
    QString nick    = " * ";

    pure_msg = LinkParser::parseForLinks(msg, false);
    msg      = LinkParser::parseForLinks(msg, true);

    pure_msg        = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + pure_msg + "</font>";
    msg             = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + msg + "</font>";
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + QDateTime::currentDateTime().toString("hh:mm:ss") + "]</font>";

    status = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";

    addOutput(status + msg);

    status += pure_msg;
    label_LAST_STATUS->setText(status);

    WulforUtil::getInstance()->textToHtml(status, false);
    label_LAST_STATUS->setToolTip(tr("<b>Last status message on hub:</b><br/>%1").arg(status));
}

void HubFrame::addOutput(QString msg){

    /* This is temporary block. Later we must make it more wise. */
    msg.replace("\r", "");
    msg.replace("\n", "\n<br/>");
    msg.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

    textEdit_CHAT->append(msg);
}

void HubFrame::addPM(QString cid, QString output){
    if (!pm.contains(cid)){
        PMWindow *p = new PMWindow(cid, _q(client->getHubUrl().c_str()));
        p->textEdit_CHAT->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(p, SIGNAL(privateMessageClosed(QString)), this, SLOT(slotPMClosed(QString)));
        connect(p, SIGNAL(inputTextChanged()), this, SLOT(slotInputTextChanged()));
        connect(p, SIGNAL(inputTextMenu()), this, SLOT(slotInputContextMenu()));
        connect(p->textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));

        MainWindow::getInstance()->addArenaWidget(p);
        MainWindow::getInstance()->addArenaWidgetOnToolbar(p, WBGET(WB_CHAT_KEEPFOCUS));

        if (!WBGET(WB_CHAT_KEEPFOCUS))
            MainWindow::getInstance()->mapWidgetOnArena(p);

        p->addOutput(output);

        p->setAttribute(Qt::WA_DeleteOnClose);

        pm.insert(cid, p);
    }
    else{
        PMMap::iterator it = pm.find(cid);

        it.value()->addOutput(output);
    }
}

void HubFrame::getParams(HubFrame::VarMap &map, const Identity &id){
    map["NICK"] = _q(id.getNick());
    map["SHARE"] = qlonglong(id.getBytesShared());
    map["COMM"] = _q(id.getDescription());
    map["TAG"] = _q(id.getTag());
    map["CONN"] = _q(id.getConnection());
    map["IP"] = _q(id.getIp());
    map["EMAIL"] = _q(id.getEmail());
    map["ISOP"] = id.isOp();
    map["SPEED"] = QString::fromStdString(id.getConnection());
    map["AWAY"] = id.isAway();

    CID cid = id.getUser()->getCID();
    map["CID"] = _q(cid.toBase32());
}

void HubFrame::on_userUpdated(const HubFrame::VarMap &map, const UserPtr &user, bool join){
    static WulforUtil *WU = WulforUtil::getInstance();
    static WulforSettings *WS = WulforSettings::getInstance();
    static bool showFavJoinsOnly = WS->getBool(WB_CHAT_SHOW_JOINS_FAV);

    if (!model)
        return;

    UserListItem *item = model->itemForPtr(user);

    if (item){
        bool needresort = false;
        bool isOp = map["ISOP"].toBool();

        total_shared -= item->share;

        item->nick = map["NICK"].toString();
        item->comm = map["COMM"].toString();
        item->conn = map["CONN"].toString();
        item->email= map["EMAIL"].toString();
        item->ip   = map["IP"].toString();
        item->share= map["SHARE"].toULongLong();
        item->tag  = map["TAG"].toString();

        if (item->isOp != isOp)
            needresort = true;

        item->isOp = isOp;
        item->px = WU->getUserIcon(user, map["AWAY"].toBool(), item->isOp, map["SPEED"].toString());

        int row = item->row();

        QModelIndex left = model->index(row, COLUMN_NICK);
        QModelIndex right= model->index(row, COLUMN_EMAIL);

        model->repaintData(left, right);

        if (needresort)
            model->needResort();
    }
    else{
        if (join && WS->getBool(WB_CHAT_SHOW_JOINS)){
            do {
                if (showFavJoinsOnly && !FavoriteManager::getInstance()->isFavoriteUser(user))
                    break;

                addStatus(map["NICK"].toString() + tr(" joins the chat"));
            } while (0);
        }

        model->addUser(map, user);

        if (pm.contains(map["NICK"].toString())){
            QString cid = map["CID"].toString();
            QString nick = map["NICK"].toString();

            PMWindow *wnd = pm[nick];

            wnd->cid = cid;
            wnd->plainTextEdit_INPUT->setEnabled(true);
            wnd->hubUrl = _q(client->getHubUrl());

            pm.insert(cid, wnd);

            pm.remove(nick);

            pmUserEvent(cid, tr("User online."));
        }
    }

    total_shared += map["SHARE"].toULongLong();
}

void HubFrame::browseUserFiles(const QString& id, bool match){
    string message;
    string cid = id.toStdString();

    if (!cid.empty()){
        try{
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

            if (user){
                if (user == ClientManager::getInstance()->getMe())
                    MainWindow::getInstance()->browseOwnFiles();
                else if (match)
                    QueueManager::getInstance()->addList(user, client->getHubUrl(), QueueItem::FLAG_MATCH_QUEUE);
                else
                    QueueManager::getInstance()->addList(user, client->getHubUrl(), QueueItem::FLAG_CLIENT_VIEW);
            }
            else {
                message = QString(tr("User not found")).toStdString();
            }
        }
        catch (const Exception &e){
            message = e.getError();

            LogManager::getInstance()->message(message);
        }
    }
}

void HubFrame::grantSlot(const QString& id){
    QString message = tr("User not found");

    if (!id.isEmpty()){
        UserPtr user = ClientManager::getInstance()->findUser(CID(id.toStdString()));

        if (user){
            UploadManager::getInstance()->reserveSlot(user, client->getHubUrl());
            message = tr("Slot granted to ") + WulforUtil::getInstance()->getNicks(user->getCID());
        }
    }

    MainWindow::getInstance()->setStatusMessage(message);
}

void HubFrame::addUserToFav(const QString& id){
    if (id.isEmpty())
        return;

    QString message = tr("User not found.");
    string cid = id.toStdString();

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user){
        if (user != ClientManager::getInstance()->getMe() && !FavoriteManager::getInstance()->isFavoriteUser(user)){
            FavoriteManager::getInstance()->addFavoriteUser(user);

            message = WulforUtil::getInstance()->getNicks(id) + tr(" has been added to favorites.");
        }
    }

    MainWindow::getInstance()->setStatusMessage(message);
}

void HubFrame::delUserFromFav(const QString& id){
    if (id.isEmpty())
        return;

    QString message = tr("User not found.");
    string cid = id.toStdString();

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user){
        if (user != ClientManager::getInstance()->getMe() && FavoriteManager::getInstance()->isFavoriteUser(user)){
            FavoriteManager::getInstance()->removeFavoriteUser(user);

            message = WulforUtil::getInstance()->getNicks(id) + tr(" has been removed from favorites.");
        }
    }

    MainWindow::getInstance()->setStatusMessage(message);
}

void HubFrame::delUserFromQueue(const QString& id){
    if (!id.isEmpty()){
        UserPtr user = ClientManager::getInstance()->findUser(CID(id.toStdString()));

        if (user)
            QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
    }
}

void HubFrame::addAsFavorite(){
    FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());

    if (!existingHub){
        FavoriteHubEntry aEntry;

        aEntry.setServer(client->getHubUrl());
        aEntry.setName(client->getHubName());
        aEntry.setDescription(client->getHubDescription());
        aEntry.setConnect(FALSE);
        aEntry.setNick(client->getMyNick());
        aEntry.setEncoding(client->getEncoding());

        FavoriteManager::getInstance()->addFavorite(aEntry);
        FavoriteManager::getInstance()->save();

        addStatus(tr("Favorite hub added."));
    }
    else{
        addStatus(tr("Favorite hub already exists."));
    }
}

void HubFrame::newMsg(VarMap map){
    QString output = "";

    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";;
    QString color = map["CLR"].toString();
    QString msg_color = WS_CHAT_MSG_COLOR;

    if (message.indexOf(_q(client->getMyNick())) >= 0){
        msg_color = WS_CHAT_SAY_NICK;

        Notification::getInstance()->showMessage(Notification::NICKSAY, getArenaTitle().left(20), nick + ": " + message);
    }

    bool third = map["3RD"].toBool();

    nick = third? ("* " + nick + " ") : ("<" + nick + "> ");

    message = LinkParser::parseForLinks(message, true);

    WulforUtil::getInstance()->textToHtml(nick);

    message = "<font color=\"" + WSGET(msg_color) + "\">" + message + "</font>";
    output  = time + QString(" <a style=\"text-decoration:none\" href=\"user://%1\"><font color=\"%2\"><b>%3</b></font></a>").arg(nick).arg(WSGET(color)).arg(nick.replace("\"", "&quot;"));
    output  += message;

    //WulforUtil::getInstance()->textToHtml(output, false);

    addOutput(output);

    if (!isVisible()){
        hasMessages = true;
        MainWindow::getInstance()->redrawToolPanel();
    }
}

void HubFrame::newPm(VarMap map){
    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";
    QString color = map["CLR"].toString();
    QString full_message = "";

    if (nick != _q(client->getMyNick()))
        Notification::getInstance()->showMessage(Notification::PM, nick, message);

    nick = map["3RD"].toBool()? ("* " + nick + " ") : ("<" + nick + "> ");

    message = LinkParser::parseForLinks(message, true);

    WulforUtil::getInstance()->textToHtml(nick);

    message       = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + message + "</font>";
    full_message  = time + QString(" <a style=\"text-decoration:none\" href=\"user://%1\"><font color=\"%2\"><b>%1</b></font></a>").arg(nick).arg(WSGET(color));
    full_message += message;

    WulforUtil::getInstance()->textToHtml(full_message, false);

    addPM(map["CID"].toString(), full_message);
}

void HubFrame::createPMWindow(const QString &nick){
    createPMWindow(CID(_tq(model->CIDforNick(nick))));
}

void HubFrame::createPMWindow(const dcpp::CID &cid){
    addPM(_q(cid.toBase32()), "");
}

bool HubFrame::hasCID(const dcpp::CID &cid, const QString &nick){
    return (model->CIDforNick(nick) == _q(cid.toBase32()));
}

bool HubFrame::isFindFrameActivated(){
    return lineEdit_FIND->hasFocus();
}

void HubFrame::clearUsers(){
    if (model){
        model->blockSignals(true);
        model->clear();
        model->blockSignals(false);
    }

    total_shared = 0;

    treeView_USERS->repaint();

    slotUsersUpdated();

    model->repaint();
}

void HubFrame::pmUserOffline(QString cid){
    pmUserEvent(cid, tr("User offline."));
}

void HubFrame::pmUserEvent(QString cid, QString e){
    if (!pm.contains(cid))
        return;

    QString output = "";
    QString nick    = " * ";

    QString msg     = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + e + "</font>";
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" +QDateTime::currentDateTime().toString("hh:mm:ss") + "]</font>";

    output = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";
    output += msg;

    WulforUtil::getInstance()->textToHtml(output, false);

    pm[cid]->addOutput(output);
}

void HubFrame::getPassword(){
    MainWindow *MW = MainWindow::getInstance();

    if (!MW->isVisible() && !(client->getPassword().size() > 0)){
        MW->show();
        MW->raise();

    }

    if(client && client->getPassword().size() > 0) {
        client->password(client->getPassword());
        addStatus(tr("Stored password sent..."));
    }
    else if (client && client->isConnected()){
        QString pass = QInputDialog::getText(this, tr("Enter password"), tr("Password"), QLineEdit::Password);

        if (!pass.isEmpty()){
            client->setPassword(pass.toStdString());
            client->password(pass.toStdString());
        }
        else
            client->disconnect(true);
    }
}

void HubFrame::follow(string redirect){
    if(!redirect.empty()) {
        if(ClientManager::getInstance()->isConnected(redirect)) {
            addStatus(tr("Redirect request received to a hub that's already connected"));
            return;
        }

        string url = redirect;

        // the client is dead, long live the client!
        client->removeListener(this);
        HubManager::getInstance()->unregisterHubUrl(_q(client->getHubUrl()));
        ClientManager::getInstance()->putClient(client);
        clearUsers();
        client = ClientManager::getInstance()->getClient(url);

        client->addListener(this);
        client->connect();
    }
}

void HubFrame::findText(QTextDocument::FindFlags flag){
    textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    if (lineEdit_FIND->text().isEmpty())
        return;

    QTextCursor c = textEdit_CHAT->textCursor();

    bool ok = textEdit_CHAT->find(lineEdit_FIND->text(), flag);

    if (flag == QTextDocument::FindBackward && !ok)
        c.movePosition(QTextCursor::End,QTextCursor::MoveAnchor,1);
    else if (flag == 0 && !ok)
        c.movePosition(QTextCursor::Start,QTextCursor::MoveAnchor,1);

    c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), c, flag);

    textEdit_CHAT->setTextCursor(c);

    slotFindAll();
}

void HubFrame::nickCompletion() {
    int cpos =  plainTextEdit_INPUT->textCursor().position();
    QString input = plainTextEdit_INPUT->textCursor().block().text().left(cpos);

    if (cpos == 0 || cpos == input.lastIndexOf(QRegExp("\\s")))
        return;

    int from = input.lastIndexOf(QRegExp("\\s"));

    if (from < 0)
        from = 0;

    QString matchExp = input.mid(from, cpos - from);
    matchExp = matchExp.trimmed();

    if (matchExp.isEmpty())
        return;

    QStringList nicks = model->matchNicksAny(matchExp.toLower());

    if (nicks.size() == 1){
        QString nick = nicks.at(0);

        int i = matchExp.length();

        while (i > 0){
            plainTextEdit_INPUT->textCursor().deletePreviousChar();
            i--;
        }

        if (plainTextEdit_INPUT->textCursor().position() == 0)
            plainTextEdit_INPUT->textCursor().insertText(nick+ ": ");
        else
            plainTextEdit_INPUT->textCursor().insertText(nick+ " ");

    }
    else if (!nicks.isEmpty() && nicks.size() < 15){
        QMenu *m = new QMenu();

        foreach (QString nick, nicks)
            m->addAction(nick);

        QAction *ret = m->exec(plainTextEdit_INPUT->cursor().pos());

        if (ret){
            QString nick = ret->text();
            //QString insertion = nick.right(nick.length()-matchExp.length()) + ": ";

            int i = matchExp.length();

            while (i > 0){
                plainTextEdit_INPUT->textCursor().deletePreviousChar();
                i--;
            }

            if (plainTextEdit_INPUT->textCursor().position() == 0)
                plainTextEdit_INPUT->textCursor().insertText(nick+ ": ");
            else
                plainTextEdit_INPUT->textCursor().insertText(nick+ " ");
        }

        delete m;
    }
}

void HubFrame::slotActivate(){
    plainTextEdit_INPUT->setFocus();
}

void HubFrame::slotUsersUpdated(){
    label_USERSTATE->setText(QString(tr("Users count: %1 | Total share: %2"))
                             .arg(model->rowCount())
                             .arg(WulforUtil::formatBytes(total_shared)));
    label_LAST_STATUS->setMaximumHeight(label_USERSTATE->height());
}

void HubFrame::slotReconnect(){
    clearUsers();

    if (client)
        client->reconnect();
}

void HubFrame::slotMapOnArena(){
    MainWindow *MW = MainWindow::getInstance();

    MW->mapWidgetOnArena(this);
}

void HubFrame::slotClose(){
    MainWindow::getInstance()->remArenaWidget(this);
    MainWindow::getInstance()->remArenaWidgetFromToolbar(this);

    close();
}

void HubFrame::slotPMClosed(QString cid){
    PMMap::iterator it = pm.find(cid);

    if (it != pm.end())
        pm.erase(it);
}

void HubFrame::slotUserListMenu(const QPoint&){
    QItemSelectionModel *selection_model = treeView_USERS->selectionModel();
    QModelIndexList proxy_list = selection_model->selectedRows(0);

    if (proxy_list.size() < 1)
        return;

    QString cid = "";

    if (treeView_USERS->model() != model){
        QModelIndex i = proxy->mapToSource(proxy_list.at(0));
        cid = reinterpret_cast<UserListItem*>(i.internalPointer())->cid;
    }
    else{
        QModelIndex i = proxy_list.at(0);
        cid = reinterpret_cast<UserListItem*>(i.internalPointer())->cid;
    }

    Menu::Action action = Menu::getInstance()->execUserMenu(client, cid);
    UserListItem *item = NULL;

    proxy_list = selection_model->selectedRows(0);

    if (proxy_list.size() < 1)
        return;

    QModelIndexList list;

    if (treeView_USERS->model() != model){
        foreach(QModelIndex i, proxy_list)
            list.push_back(proxy->mapToSource(i));
    }
    else
        list = proxy_list;

    switch (action){
        case Menu::None:
        {
            return;
        }
        case Menu::BrowseFilelist:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    browseUserFiles(item->cid);
            }

            break;
        }
        case Menu::PrivateMessage:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    addPM(item->cid, "");

                if (pm.contains(cid))
                    MainWindow::getInstance()->mapWidgetOnArena(pm[cid]);
            }

            break;
        }
        case Menu::CopyText:
        {
            QString ttip = "";

            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    ttip = getUserInfo(item);
            }

            if (!ttip.isEmpty())
                QApplication::clipboard()->setText(ttip, QClipboard::Clipboard);

            break;
        }
        case Menu::CopyNick:
        {
            QString ret = "";

            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (ret.length() > 0)
                    ret += "\n";

                if (item)
                    ret += item->nick;
            }

            QApplication::clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case Menu::MatchQueue:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    browseUserFiles(item->cid, true);
            }

            break;
        }
        case Menu::FavoriteAdd:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    addUserToFav(item->cid);

                item->fav = true;
            }

            break;
        }
        case Menu::FavoriteRem:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    delUserFromFav(item->cid);

                item->fav = false;
            }

            break;
        }
        case Menu::GrantSlot:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    grantSlot(item->cid);
            }

            break;
        }
        case Menu::RemoveQueue:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    delUserFromQueue(item->cid);
            }

            break;
        }
        default:
        {
            break;
        }
    }
}

void HubFrame::slotChatMenu(const QPoint &){
    QTextEdit *editor = static_cast<QTextEdit*>(sender());

    if (!editor)
        return;

    QString pressedParagraph = "", nick = "";
    int nickStart = 0, nickLen = 0;
    QTextCursor cursor = editor->cursorForPosition(editor->mapFromGlobal(QCursor::pos()));

    pressedParagraph = cursor.block().text();

    nickStart = 1 + pressedParagraph.indexOf("<");
    nickLen = pressedParagraph.indexOf(">") - nickStart;

    // sanity check
    if ((nickStart == 0) || (nickLen < 0)) {
        nickStart = QString("[hh.mm.ss] ").length();

        nickLen = pressedParagraph.indexOf(" ", nickStart) - nickStart;

        nick = pressedParagraph.mid(nickStart, nickLen);

        /* [10:57:15] * somenick does something */
        if (nick == "*") {
            nickStart = pressedParagraph.indexOf(" ", nickStart + 1) + 1;

            if (nickStart > 0) {
                nickLen = pressedParagraph.indexOf(" ", nickStart) - nickStart;
                nick = pressedParagraph.mid(nickStart, nickLen);
            }
        }
    }
    else {
        nick = pressedParagraph.mid(nickStart, nickLen);
    }

    QString cid = model->CIDforNick(nick);

    if (cid.isEmpty()){
        QMenu *m = textEdit_CHAT->createStandardContextMenu(QCursor::pos());
        m->exec(QCursor::pos());

        delete m;

        return;
    }

    QPoint p = QCursor::pos();

    bool pmw = (editor != this->textEdit_CHAT);

    Menu::Action action = Menu::getInstance()->execChatMenu(client, cid, pmw);

    if (!model->itemForNick(nick))//may be user went offline
        return;

    switch (action){
        case Menu::CopyText:
        {
            QString ret = editor->textCursor().selectedText();

            if (ret.isEmpty())
                ret = editor->anchorAt(textEdit_CHAT->mapFromGlobal(p));

            if (ret.startsWith("user://")){
                ret.remove(0, 7);

                ret = ret.trimmed();

                if (ret.startsWith("<") && ret.endsWith(">")){
                    ret.remove(0, 1);//remove <
                    ret = ret.left(ret.lastIndexOf(">"));//remove >
                }
            }

            if (ret.isEmpty())
                ret = editor->textCursor().block().text();

            QApplication::clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case Menu::CopyNick:
        {
            qApp->clipboard()->setText(nick, QClipboard::Clipboard);

            break;
        }
        case Menu::FindInList:
        {
            UserListItem *item = model->itemForNick(nick);

            if (item){
                QModelIndex index = model->index(item->row(), 0, QModelIndex());

                treeView_USERS->clearSelection();
                treeView_USERS->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                treeView_USERS->scrollTo(index, QAbstractItemView::PositionAtCenter);
            }

            break;
        }
        case Menu::BrowseFilelist:
        {
            browseUserFiles(cid, false);

            break;
        }
        case Menu::MatchQueue:
        {
            browseUserFiles(cid, true);

            break;
        }
        case Menu::PrivateMessage:
        {
            addPM(cid, "");

            if (pm.contains(cid))
                MainWindow::getInstance()->mapWidgetOnArena(pm[cid]);

            break;
        }
        case Menu::FavoriteAdd:
        {
            addUserToFav(cid);

            break;
        }
        case Menu::FavoriteRem:
        {
            delUserFromFav(cid);

            break;
        }
        case Menu::GrantSlot:
        {
            grantSlot(cid);

            break;
        }
        case Menu::RemoveQueue:
        {
            delUserFromQueue(cid);

            break;
        }
        case Menu::UserCommands:
        {
            //All work already done in Menu::exec*
            break;
        }
        case Menu::ClearChat:
        {
            if (pmw)
                MainWindow::getInstance()->slotChatClear(); // some hack
            else
                clearChat();

            break;
        }
        case Menu::FindInChat:
        {
            slotHideFindFrame();

            break;
        }
        case Menu::DisableChat:
        {
            disableChat();

            break;
        }
        case Menu::SelectAllChat:
        {
            editor->selectAll();

            break;
        }
        case Menu::ZoomInChat:
        {
            editor->zoomIn();

            break;
        }
        case Menu::ZoomOutChat:
        {
            editor->zoomOut();

            break;
        }
        case Menu::None:
        {
            return;
        }
        default:
        {
            return;
        }
    }
}

void HubFrame::slotHeaderMenu(const QPoint&){
    WulforUtil::headerMenu(treeView_USERS);
}

void HubFrame::slotShowWnd(){
    if (isVisible())
        return;

   MainWindow *MW = MainWindow::getInstance();

   MW->mapWidgetOnArena(this);
}

void HubFrame::slotShellFinished(bool ok, QString output){
    if (ok){
        HubFrame *fr = qobject_cast<HubFrame *>(sender()->parent());
        PMWindow *pm = qobject_cast<PMWindow *>(sender()->parent());

        if (fr == this)
            sendChat(output, false, false);
        else if (pm)
            pm->sendMessage(output, false, false);
    }

    ShellCommandRunner *runner = reinterpret_cast<ShellCommandRunner*>(sender());

    runner->cancel();
    runner->exit(0);
    runner->wait(100);

    if (runner->isRunning())
        runner->terminate();

    if (shell_list.indexOf(runner) >= 0)
        shell_list.removeAt(shell_list.indexOf(runner));

    delete runner;
}

void HubFrame::nextMsg(){
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

void HubFrame::prevMsg(){
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

void HubFrame::slotHideFindFrame(){
    frame->setVisible(!frame->isVisible());

    if (frame->isVisible()){
        QString stext = textEdit_CHAT->textCursor().selectedText();

        if (!stext.isEmpty()){
            lineEdit_FIND->setText(stext);
            lineEdit_FIND->selectAll();
        }

        lineEdit_FIND->setFocus();
    }
    else{
        QTextCursor c = textEdit_CHAT->textCursor();

        c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);

        textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        textEdit_CHAT->setTextCursor(c);
    }
}

void HubFrame::slotFilterTextChanged(){
    QString text = lineEdit_FILTER->text();

    if (!text.isEmpty()){
        if (!proxy){
            proxy = new QSortFilterProxyModel(this);
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
            proxy->setDynamicSortFilter(true);
            proxy->setSourceModel(model);
        }

        proxy->setFilterFixedString(text);
        proxy->setFilterKeyColumn(comboBox_COLUMNS->currentIndex());

        if (treeView_USERS->model() != proxy)
            treeView_USERS->setModel(proxy);
    }
    else if (treeView_USERS->model() != model)
        treeView_USERS->setModel(model);

    if (comboBox_COLUMNS->hasFocus())
        lineEdit_FILTER->setFocus();
}

void HubFrame::slotFindTextEdited(const QString & text){
    if (text.isEmpty()){
        textEdit_CHAT->verticalScrollBar()->setValue(textEdit_CHAT->verticalScrollBar()->maximum());
        textEdit_CHAT->textCursor().movePosition(QTextCursor::End, QTextCursor::MoveAnchor, 1);

        return;
    }

    QTextCursor c = textEdit_CHAT->textCursor();

    c.movePosition(QTextCursor::StartOfLine,QTextCursor::MoveAnchor,1);
    c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), c, 0);

    textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    textEdit_CHAT->setTextCursor(c);

    slotFindAll();
}

void HubFrame::slotFindAll(){
    if (!pushButton_ALL->isChecked()){
        textEdit_CHAT->setExtraSelections(QList<QTextEdit::ExtraSelection>());

        return;
    }

    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!lineEdit_FIND->text().isEmpty()) {
        QTextEdit::ExtraSelection selection;

        QColor color;
        color.setNamedColor(WSGET(WS_CHAT_FIND_COLOR));
        color.setAlpha(WIGET(WI_CHAT_FIND_COLOR_ALPHA));

        selection.format.setBackground(color);

        QTextCursor c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), 0, 0);

        while (!c.isNull()){
            selection.cursor = c;
            extraSelections.append(selection);

            c = textEdit_CHAT->document()->find(lineEdit_FIND->text(), c, 0);
        }
    }

    textEdit_CHAT->setExtraSelections(extraSelections);
}

void HubFrame::slotSmile(){
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

void HubFrame::slotInputTextChanged(){
#ifdef USE_ASPELL
    PMWindow *p = qobject_cast<PMWindow*>(sender());
    QPlainTextEdit *plainTextEdit_INPUT = (p)? qobject_cast<QPlainTextEdit*>(p->inputWidget()) : this->plainTextEdit_INPUT;

    if (!plainTextEdit_INPUT)
        return;
    QString line = plainTextEdit_INPUT->toPlainText();

    if (line.isEmpty() || !SpellCheck::getInstance())
        return;

    SpellCheck *sp = SpellCheck::getInstance();
    QStringList words = line.split(QRegExp("\\W+"), QString::SkipEmptyParts);

    if (words.isEmpty())
        return;

    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextCursor c = plainTextEdit_INPUT->textCursor();

    plainTextEdit_INPUT->moveCursor(QTextCursor::Start);

    QTextEdit::ExtraSelection selection;
    selection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    selection.format.setUnderlineColor(Qt::red);

    bool ok = false;
    foreach (QString s, words){
        if (s.toLongLong(&ok) && ok)
            continue;

        if (plainTextEdit_INPUT->find(s) && !sp->ok(s)){
            selection.cursor = plainTextEdit_INPUT->textCursor();
            extraSelections.append(selection);
        }
    }

    plainTextEdit_INPUT->setTextCursor(c);
    plainTextEdit_INPUT->setExtraSelections(extraSelections);
#endif
}

void HubFrame::slotInputContextMenu(){
    PMWindow *p = qobject_cast<PMWindow*>(sender());
    QPlainTextEdit *plainTextEdit_INPUT = (p)? qobject_cast<QPlainTextEdit*>(p->inputWidget()) : this->plainTextEdit_INPUT;

    if (!plainTextEdit_INPUT)
        return;

    QMenu *m = plainTextEdit_INPUT->createStandardContextMenu();

#ifndef USE_ASPELL
    m->exec(QCursor::pos());
    m->deleteLater();
#else
    if (SpellCheck::getInstance()) {
        SpellCheck *sp = SpellCheck::getInstance();
        QTextCursor c = plainTextEdit_INPUT->cursorForPosition(plainTextEdit_INPUT->mapFromGlobal(QCursor::pos()));
        c.select(QTextCursor::WordUnderCursor);

        QString word = c.selectedText();

        if (sp->ok(word) || word.isEmpty()){
            c.clearSelection();
            m->exec(QCursor::pos());
            m->deleteLater();
        }
        else {
            QStringList list;
            sp->suggestions(word, list);

            m->addSeparator();
            QMenu *ss = new QMenu(tr("Suggestions"), this);
            QAction *add_to_dict = new QAction(tr("Add to dictionary"), m);

            m->addAction(add_to_dict);

            foreach (QString s, list)
                ss->addAction(s);

            m->addMenu(ss);

            QAction *ret = m->exec(QCursor::pos());

            if (ret == add_to_dict)
                sp->addToDict(word);
            else if (ret){
                c.removeSelectedText();

                c.insertText(ret->text());
            }

            m->deleteLater();
            ss->deleteLater();

            slotInputTextChanged();
        }
    }
    else {
        m->exec(QCursor::pos());
        m->deleteLater();
    }
#endif
}

void HubFrame::slotStatusLinkOpen(const QString &url){
    WulforUtil::getInstance()->openUrl(url);
}

void HubFrame::slotHubMenu(QAction *res){
    if (res && !res->toolTip().isEmpty()){//User command
        QString last_user_cmd = res->toolTip();
        QString cmd_name = res->statusTip();
        QString hub = res->data().toString();

        int id = FavoriteManager::getInstance()->findUserCommand(cmd_name.toStdString(), client->getHubUrl());
        UserCommand uc;

        if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
            return;

        StringMap params;

        if (WulforUtil::getInstance()->getUserCommandParams(last_user_cmd, params)){
            UserPtr user = ClientManager::getInstance()->getMe();

            if (user)
                ClientManager::getInstance()->userCommand(user, uc, params, true);
        }
    }
}

void HubFrame::on(ClientListener::Connecting, Client *c) throw(){
    QString status = QString("Connecting to %1").arg(QString::fromStdString(client->getHubUrl()));

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::Connected, Client*) throw(){
    QString status = QString("Connected to %1").arg(QString::fromStdString(client->getHubUrl()));

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));

    HubManager::getInstance()->registerHubUrl(_q(client->getHubUrl()), this);
}

void HubFrame::on(ClientListener::UserUpdated, Client*, const OnlineUser &user) throw(){
    if (user.getIdentity().isHidden() && !WBGET(WB_SHOW_HIDDEN_USERS))
        return;

    UserUpdatedEvent *u_e = new UserUpdatedEvent(user, true);

    getParams(u_e->getMap(), user.getIdentity());

    QApplication::postEvent(this, u_e);
}

void HubFrame::on(ClientListener::UsersUpdated x, Client*, const OnlineUserList &list) throw(){
    UserUpdatedEvent *u_e = NULL;
    bool showHidden = WBGET(WB_SHOW_HIDDEN_USERS);

    for (OnlineUserList::const_iterator it = list.begin(); it != list.end(); ++it){
        if ((*(*it)).getIdentity().isHidden() && !showHidden)
            break;

        u_e = new UserUpdatedEvent((*(*it)), false);

        getParams(u_e->getMap(), (*(*it)).getIdentity());

        QApplication::postEvent(this, u_e);
    }
}

void HubFrame::on(ClientListener::UserRemoved, Client*, const OnlineUser &user) throw(){
    if (user.getIdentity().isHidden() && !WBGET(WB_SHOW_HIDDEN_USERS))
        return;

    QApplication::postEvent(this, new UserRemovedEvent(user.getUser(), user.getIdentity().getBytesShared()));
}

void HubFrame::on(ClientListener::Redirect, Client*, const string &link) throw(){
    if(ClientManager::getInstance()->isConnected(link)) {
        typedef Func1<HubFrame, QString> FUNC;
        FUNC *func = new FUNC(this, &HubFrame::addStatus, tr("Redirect request received to a hub that's already connected"));

        QApplication::postEvent(this, new UserCustomEvent(func));

        return;
    }

    if(BOOLSETTING(AUTO_FOLLOW)) {
        typedef Func1<HubFrame, string> FUNC;
        FUNC *func = new FUNC(this, &HubFrame::follow, link);

        QApplication::postEvent(this, new UserCustomEvent(func));
    }
}

void HubFrame::on(ClientListener::Failed, Client*, const string &msg) throw(){
    QString status = QString("Fail: %1...").arg(_q(msg));

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));

    typedef Func0<HubFrame> FUNC0;
    FUNC0 *f = new FUNC0(this, &HubFrame::clearUsers);

    QApplication::postEvent(this, new UserCustomEvent(f));

    HubManager::getInstance()->unregisterHubUrl(_q(client->getHubUrl()));
}

void HubFrame::on(GetPassword, Client*) throw(){
    typedef Func0<HubFrame> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::getPassword);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::HubUpdated, Client*) throw(){
    typedef Func0<MainWindow> FUNC;
    FUNC *func = new FUNC(MainWindow::getInstance(), &MainWindow::redrawToolPanel);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::Message, Client*, const OnlineUser &user, const string& msg, bool thirdPerson) throw(){
    if (chatDisabled)
        return;

    VarMap map;

    if (AntiSpam::getInstance()){
        if (AntiSpam::getInstance()->isInBlack(_q(user.getIdentity().getNick())))
            return;
    }

    if (_q(msg).startsWith("/me ")){
        QString m = _q(msg);
        m.remove(0, 4);

        typedef Func1<HubFrame, QString> FUNC;
        FUNC *func = new FUNC(this, &HubFrame::addStatus, _q(user.getIdentity().getNick()) + " " + m);

        QApplication::postEvent(this, new UserCustomEvent(func));

        return;
    }

    map["NICK"] = _q(user.getIdentity().getNick());
    map["MSG"]  = _q(msg.c_str());
    map["TIME"] = QDateTime::currentDateTime().toString("hh:mm:ss");

    QString color = WS_CHAT_USER_COLOR;

    if (user.getIdentity().isHub())
        color = WS_CHAT_STAT_COLOR;
    else if (user.getUser() == client->getMyIdentity().getUser())
        color = WS_CHAT_LOCAL_COLOR;
    else if (user.getIdentity().isOp())
        color = WS_CHAT_OP_COLOR;
    else if (user.getIdentity().isBot())
        color = WS_CHAT_BOT_COLOR;

    if (FavoriteManager::getInstance()->isFavoriteUser(user.getUser()))
        color = WS_CHAT_FAVUSER_COLOR;

    map["CLR"] = color;
    map["3RD"] = thirdPerson;

    typedef Func1<HubFrame, VarMap> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::newMsg, map);

    QApplication::postEvent(this, new UserCustomEvent(func));

    if (BOOLSETTING(LOG_MAIN_CHAT)){
        StringMap params;
        params["message"] = msg;
        client->getHubIdentity().getParams(params, "hub", false);
        params["hubURL"] = client->getHubUrl();
        client->getMyIdentity().getParams(params, "my", true);
        LOG(LogManager::CHAT, params);
    }
}

void HubFrame::on(ClientListener::StatusMessage, Client*, const string &msg, int) throw(){
    QString status = QString("%1 ").arg(_q(msg));

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));

    if (BOOLSETTING(LOG_STATUS_MESSAGES)){
        StringMap params;
        client->getHubIdentity().getParams(params, "hub", FALSE);
        params["hubURL"] = client->getHubUrl();
        client->getMyIdentity().getParams(params, "my", TRUE);
        params["message"] = msg;
        LOG(LogManager::STATUS, params);
    }
}

void HubFrame::on(ClientListener::PrivateMessage, Client*, const OnlineUser &from, const OnlineUser &to, const OnlineUser &replyTo,
                  const string &msg, bool thirdPerson) throw()
{
    const OnlineUser& user = (replyTo.getUser() == ClientManager::getInstance()->getMe()) ? to : replyTo;

    if (user.getIdentity().isHub() && BOOLSETTING(IGNORE_HUB_PMS))
        return;
    else if (user.getIdentity().isBot() && BOOLSETTING(IGNORE_BOT_PMS))
        return;

    VarMap map;

    CID id = user.getUser()->getCID();

    QString nick =  _q(from.getIdentity().getNick());

    if (AntiSpam::getInstance()){
        /*QString cid = (from.getUser() == ClientManager::getInstance()->getMe())? _q(to.getIdentity().getNick()):_q(from.getIdentity().getNick());
        QString nick = WulforUtil::getInstance()->getNicks(cid);

        if (!AntiSpam::getInstance()->isInAny(nick)){
            AntiSpam::getInstance()->checkUser(cid, _q(msg), _q(client->getHubUrl()));

            if (AntiSpam::getInstance()->isInBlack(nick) || !AntiSpam::getInstance()->isInAny(nick))
                return;
        }*/

        if (AntiSpam::getInstance()->isInBlack(nick))
            return;
    }

    map["NICK"]  = nick;
    map["MSG"]   = _q(msg);
    map["TIME"]  = QDateTime::currentDateTime().toString("hh:mm:ss");

    QString color = WS_CHAT_PRIV_USER_COLOR;

    if (nick == _q(client->getMyNick()))
        color = WS_CHAT_PRIV_LOCAL_COLOR;
    else if (user.getIdentity().isOp())
        color = WS_CHAT_OP_COLOR;
    else if (user.getIdentity().isBot())
        color = WS_CHAT_BOT_COLOR;
    else if (user.getIdentity().isHub())
        color = WS_CHAT_STAT_COLOR;

    map["CLR"] = color;
    map["3RD"] = thirdPerson;
    map["CID"] = _q(id.toBase32());

    typedef Func1<HubFrame, VarMap> FUNC;
    FUNC *func = NULL;

    if (WBGET(WB_CHAT_REDIRECT_BOT_PMS) && user.getIdentity().isBot())
        func = new FUNC(this, &HubFrame::newMsg, map);
    else
        func = new FUNC(this, &HubFrame::newPm, map);

    QApplication::postEvent(this, new UserCustomEvent(func));

    if (BOOLSETTING(LOG_PRIVATE_CHAT)){
        StringMap params;
        params["message"] = msg;
        params["hubNI"] = _tq(WulforUtil::getInstance()->getHubNames(id));
        params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(id));
        params["userCID"] = id.toBase32();
        params["userNI"] = ClientManager::getInstance()->getNicks(id)[0];
        params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
        LOG(LogManager::PM, params);
    }

    if (from.getUser() != ClientManager::getInstance()->getMe() && Util::getAway() && !pm.contains(_q(id.toBase32())))
        ClientManager::getInstance()->privateMessage(user.getUser(), Util::getAwayMessage(), false, client->getHubUrl());
}

void HubFrame::on(ClientListener::NickTaken, Client*) throw(){
    QString status = QString("Sorry, but nick \"%1\" is already taken by another user.").arg(client->getCurrentNick().c_str());

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::SearchFlood, Client*, const string &str) throw(){
    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, tr("Search flood detected: %1").arg(_q(str)));

    QApplication::postEvent(this, new UserCustomEvent(func));
}
