#include "HubFrame.h"
#include "MainWindow.h"
#include "PMWindow.h"
#include "Func.h"
#include "WulforManager.h"
#include "WulforUtil.h"
#include "Antispam.h"
#include "HubManager.h"

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

#include <QDebug>

#include <exception>

QStringList HubFrame::LinkParser::link_types = QString("http://,ftp://,dchub://,adc://,adcs://,magnet:,www.").split(",");
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
    QAction *copy_nick   = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy nick"), NULL);
    QAction *browse      = new QAction(WU->getPixmap(WulforUtil::eiFOLDER_BLUE), tr("Browse files"), NULL);
    QAction *match_queue = new QAction(QPixmap(), tr("Match Queue"), NULL);
    QAction *private_msg = new QAction(WU->getPixmap(WulforUtil::eiMESSAGE), tr("Private Message"), NULL);
    QAction *fav_add     = new QAction(QPixmap(), tr("Add to Favorites"), NULL);
    QAction *fav_del     = new QAction(QPixmap(), tr("Remove from Favorites"), NULL);
    QAction *grant_slot  = new QAction(WU->getPixmap(WulforUtil::eiEDITADD), tr("Grant slot"), NULL);
    QAction *rem_queue   = new QAction(QPixmap(), tr("Remove from Queue"), NULL);

    //Chat actions
    QAction *sep1        = new QAction(NULL);
    QAction *clear_chat  = new QAction(WU->getPixmap(WulforUtil::eiCLEAR), tr("Clear chat"), NULL);
    QAction *dis_chat    = new QAction(WU->getPixmap(WulforUtil::eiFILECLOSE), tr("Disable/Enable chat"), NULL);
    QAction *sep2        = new QAction(NULL);
    QAction *select_all  = new QAction(tr("Select all"), NULL);
    QAction *sep3        = new QAction(NULL);
    QAction *zoom_in     = new QAction(WU->getPixmap(WulforUtil::eiZOOM_IN), tr("Zoom In"), NULL);
    QAction *zoom_out    = new QAction(WU->getPixmap(WulforUtil::eiZOOM_OUT), tr("Zoom Out"), NULL);

    sep1->setSeparator(true), sep2->setSeparator(true), sep3->setSeparator(true);

    actions << copy_nick
            << browse
            << match_queue
            << private_msg
            << fav_add
            << fav_del
            << grant_slot
            << rem_queue;

    chat_actions << sep1
                 << clear_chat
                 << dis_chat
                 << sep2
                 << select_all
                 << sep3
                 << zoom_in
                 << zoom_out;

    chat_actions_map.insert(clear_chat, ClearChat);
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

    menu->addActions(actions);

    QMenu *user_menu = NULL;

    if (!cid.isEmpty()){
        user_menu = buildUserCmdMenu(_q(client->getAddress()), cid);
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

HubFrame::Menu::Action HubFrame::Menu::execChatMenu(Client *client, const QString &cid = QString()){
    if (!client)
        return None;

    menu->clear();

    menu->addActions(actions);
    menu->addActions(chat_actions);
    
    QMenu *user_menu = NULL;
    
    if (!cid.isEmpty()){
        user_menu = buildUserCmdMenu(_q(client->getAddress()), cid);
        menu->addMenu(user_menu);
    }

    QAction *res = menu->exec(QCursor::pos());

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

QMenu *HubFrame::Menu::buildUserCmdMenu(const QString &hub, const QString &cid){
    if (cid.isEmpty() || hub.isEmpty())
        return NULL;

    dcpp::StringList hubs;
    QMap<QString, QMenu*> registered_menus;

    hubs.push_back(hub.toStdString());

    QMenu *usr_menu = new QMenu(tr("User commands"));
    UserCommand::List commands = FavoriteManager::getInstance()->getUserCommands(UserCommand::CONTEXT_CHAT, hubs);
    bool separator = false;

    for (UserCommand::List::iterator i = commands.begin(); i != commands.end(); ++i){
        UserCommand& uc = *i;

        // Add line separator only if it's not a duplicate
        if (uc.getType() == UserCommand::TYPE_SEPARATOR && !separator){
            QAction *sep = new QAction(usr_menu);
            sep->setSeparator(true);

            usr_menu->addAction(sep);

            separator = true;
        }
        else if (uc.getType() == UserCommand::TYPE_RAW || uc.getType() == UserCommand::TYPE_RAW_ONCE){
            separator = false;

            QString raw_name = _q(uc.getName());
            QAction *action = NULL;

            if (raw_name.contains("\\")){
                QStringList submenus = raw_name.split("\\", QString::SkipEmptyParts);
                QString name = submenus.takeLast();
                QString key = "";
                QMenu *parent = usr_menu;
                QMenu *submenu;

                foreach (QString s, submenus){
                    key += s + "\\";

                    if (registered_menus.contains(key))
                        parent = registered_menus[key];
                    else {
                        submenu = new QMenu(s, parent);
                        parent->addMenu(submenu);

                        registered_menus.insert(key, submenu);

                        parent = submenu;
                    }
                }

                action = new QAction(name, parent);
                parent->addAction(action);
            }
            else{
                action = new QAction(_q(uc.getName()), usr_menu);
                usr_menu->addAction(action);
            }

            action->setToolTip(_q(uc.getCommand()));
            action->setStatusTip(_q(uc.getName()));
            action->setData(_q(uc.getHub()));

        }
    }

    return usr_menu;
}

QString HubFrame::LinkParser::parseForLinks(QString input){
    if (input.isEmpty() || input.isNull())
        return input;

    QString output = "";

    while (!input.isEmpty()){
        for (int j = 0; j < link_types.size(); j++){
            QString linktype = link_types.at(j);

            if (input.startsWith(linktype)){
                int l_pos = linktype.length();

                while (l_pos < input.size()){
                    QChar ch = input.at(l_pos);

                    if (ch == ',' || ch == '!' || ch.isSpace()){
                        break;
                    }
                    else
                        l_pos++;
                }

                QString link = input.left(l_pos);
                QString toshow = QUrl::fromEncoded(link.toAscii()).toString();

                if (linktype == "magnet:"){
                    QUrl url;
                    toshow = link;
                    toshow.replace("+", "%20");
                    url.setEncodedUrl(toshow.toAscii());

                    if (url.hasQueryItem("dn")) {
                        toshow = url.queryItemValue("dn");

                        if (url.hasQueryItem("xl"))
                            toshow += " (" + QString::fromStdString(Util::formatBytes(url.queryItemValue("xl").toULongLong())) + ")";

                        toshow.prepend("magnet://");
                    }
                }

                if (linktype == "www.")
                    toshow.prepend("http://");

                QString html_link = "";

                if (linktype != "magnet:")
                    html_link = QString("<a href=\"%1\" title=\"%1\">%1</a>").arg(toshow);
                else
                    html_link = QString("<a href=\"%1\" title=\"%2\">%2</a>").arg(link).arg(toshow);

                output += html_link;
                input.remove(0, l_pos);
            }

            if (input.isEmpty())
                break;
        }

        if (input.isEmpty())
            break;

        if (input.at(0) == ' '){//convert all spaces to html
            output += "&nbsp;";
        }
        else {
            output += input.at(0);
        }

        input.remove(0, 1);
    }

    return output;
}

HubFrame::HubFrame(QWidget *parent=NULL, QString hub="", QString encoding=""):
        QWidget(parent),
        total_shared(0),
        arenaMenu(NULL),
        codec(NULL),
        chatDisabled(false)
{
    setupUi(this);

    if (!Menu::getInstance())
        Menu::newInstance();

    Menu::counter++;

    client = ClientManager::getInstance()->getClient(hub.toStdString());
    client->addListener(this);

    QString enc = WulforUtil::getInstance()->qtEnc2DcEnc(encoding);

    if (enc.isEmpty())
        enc = WulforSettings::getInstance()->getStr(WS_DEFAULT_LOCALE);

    if (enc.indexOf(" ") > 0){
        enc = enc.left(enc.indexOf(" "));
        enc.replace(" ", "");
    }

    client->setEncoding(enc.toStdString());

    codec = WulforUtil::getInstance()->codecForEncoding(encoding);

    init();

    client->connect();

    setAttribute(Qt::WA_DeleteOnClose);
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

        if ((static_cast<QPlainTextEdit*>(obj) == plainTextEdit_INPUT) && (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return)){
            sendChat(plainTextEdit_INPUT->toPlainText(), false, true);

            plainTextEdit_INPUT->setPlainText("");

            return false;
        }
        else if (static_cast<QLineEdit*>(obj) == lineEdit_FILTER){
            bool ret = QWidget::eventFilter(obj, e);
            QString filter_text = lineEdit_FILTER->text();

            if (!filter_text.isEmpty()){
                if (!proxy){
                    proxy = new QSortFilterProxyModel(this);
                    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
                    proxy->setDynamicSortFilter(true);
                    proxy->setSortRole(Qt::DisplayRole);
                    proxy->setSourceModel(model);
                }

                proxy->setFilterFixedString(filter_text);
                proxy->setFilterKeyColumn(comboBox_COLUMNS->currentIndex());

                if (treeView_USERS->model() != proxy)
                    treeView_USERS->setModel(proxy);
            }
            else if (treeView_USERS->model() != model)
                treeView_USERS->setModel(model);

            return ret;
        }
    }
    else if (e->type() == QEvent::MouseButtonPress){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if ((static_cast<QWidget*>(obj) == textEdit_CHAT->viewport()) && (m_e->button() == Qt::LeftButton)){
            QString pressedParagraph = textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos()));

            WulforUtil::getInstance()->openUrl(pressedParagraph);
        }
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

        model->removeUser(u_e->getUser());

        if (pm.contains(_q(u_e->getUser()->getCID().toBase32())))
            pmUserOffline(_q(u_e->getUser()->getCID().toBase32()));
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

    blockSignals(false);

    setAttribute(Qt::WA_DeleteOnClose);

    e->accept();
}

void HubFrame::showEvent(QShowEvent *e){
    e->accept();

    if (isVisible())
        HubManager::getInstance()->setActiveHub(this);
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
    treeView_USERS->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_USERS->header()->setContextMenuPolicy(Qt::CustomContextMenu);

    lineEdit_FILTER->installEventFilter(this);

    textEdit_CHAT->document()->setMaximumBlockCount(WIGET(WI_CHAT_MAXPARAGRAPHS));
    textEdit_CHAT->setContextMenuPolicy(Qt::CustomContextMenu);
    textEdit_CHAT->setReadOnly(true);
    textEdit_CHAT->setAutoFormatting(QTextEdit::AutoNone);
    textEdit_CHAT->viewport()->installEventFilter(this);//QTextEdit don't receive all mouse events

    for (int i = 0; i < model->columnCount(); i++)
        comboBox_COLUMNS->addItem(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());

    connect(treeView_USERS, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotUserListMenu(QPoint)));
    connect(treeView_USERS->header(), SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotHeaderMenu(QPoint)));
    connect(updater, SIGNAL(timeout()), this, SLOT(slotUsersUpdated()));
    connect(textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));

    plainTextEdit_INPUT->installEventFilter(this);

    initMenu();

    load();

    updater->start();
}

void HubFrame::initMenu(){
    WulforUtil *WU = WulforUtil::getInstance();

    arenaMenu = new QMenu(tr("Hub menu"), this);

    QAction *reconnect = new QAction(WU->getPixmap(WulforUtil::eiRECONNECT), tr("Reconnect"), arenaMenu);
    QAction *show_wnd  = new QAction(WU->getPixmap(WulforUtil::eiCHAT), tr("Show window"), arenaMenu);
    QAction *sep       = new QAction(arenaMenu);
    sep->setSeparator(true);
    QAction *close_wnd = new QAction(WU->getPixmap(WulforUtil::eiEXIT), tr("Close"), arenaMenu);

    arenaMenu->addActions(QList<QAction*>() << reconnect
                                            << show_wnd
                                            << sep
                                            << close_wnd
                         );

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
}

QWidget *HubFrame::getWidget(){
    return this;
}

QString HubFrame::getArenaTitle(){
    QString ret = tr("Not connected");

    if (client && client->isConnected()){
        ret  = QString("%1 - %2 [%3]").arg(QString(client->getHubUrl().c_str()))
                                      .arg(QString(client->getHubDescription().c_str()).left(70))
                                      .arg(QString(client->getIp().c_str()));
        QString prefix = QString("[+%1] ").arg(client->isSecure()? ("S") : (client->isTrusted()? ("T"): ("")));

        ret.prepend(prefix);
    }
    else if (client){
        ret = QString("[-] %1").arg(client->getHubUrl().c_str());
    }

    return ret;
}

QMenu *HubFrame::getMenu(){
    return arenaMenu;
}

void HubFrame::sendChat(QString msg, bool thirdPerson, bool stripNewLines){
    if (!client || !client->isConnected() || msg.isEmpty() || msg.isNull())
        return;

    if (stripNewLines)
        msg.replace("\n", "");

    if (msg.trimmed().isEmpty())
        return;

    if (!parseForCmd(msg))
        client->hubMessage(Text::toUtf8(msg.toStdString()), thirdPerson);
}

bool HubFrame::parseForCmd(QString line){
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

            addStatus(tr("Away mode off"));
        }
        else {
            Util::setAway(true);
            Util::setManualAway(true);

            line.remove(0, 6);

            Util::setAwayMessage(line.toStdString());

            addStatus(tr("Away mode on: ") + QString::fromStdString(Util::getAwayMessage()));
        }
    }
    else if (cmd == "/back"){
        Util::setAway(false);

        addStatus(tr("Away mode off"));
    }
    else if (cmd == "/clear"){
        textEdit_CHAT->setHtml("");

        addStatus(tr("Chat has been cleared"));
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
    else if (cmd == "/me" && !emptyParam){
        if (line.endsWith("\n"))//remove extra \n char
            line = line.left(line.lastIndexOf("\n"));

        line.remove(0, 4);

        sendChat(line, true, false);
    }
    else if (cmd == "/pm" && !emptyParam){
        addPM(model->CIDforNick(param), "");
    }
    else if (cmd == "/help" || cmd == "/?" || cmd == "/h"){
        QString out = "\n" +
                      tr("/away - set away-mode on/off\n"
                         "/back - set away-mode off\n"
                         "/browse <nick> - browse user files\n"
                         "/clear - clear chat window\n"
                         "/close - close this hub\n"
                         "/fav - add this hub to favorites\n"
                         "/grant <nick> - grant extra slot to user\n"
                         "/help, /?, /h - show this help\n"
                         "/me - say a third person\n"
                         "/pm <nick> - begin private chat with user\n");

        addStatus(out);
    }
    else
        return false;

    return true;
}

void HubFrame::addStatus(QString msg){
    QString status = "";
    QString nick    = " * ";

    WulforUtil::getInstance()->textToHtml(msg);
    WulforUtil::getInstance()->textToHtml(nick);

    msg             = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + msg + "</font>";
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + _q(Util::getTimeString().c_str()) + "]</font>";

    status = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";
    status += msg;

    WulforUtil::getInstance()->textToHtml(status, false);

    addOutput(status);
}

void HubFrame::addOutput(QString msg){
    msg.replace("\n", "<br />");
    msg.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");

    textEdit_CHAT->append(msg);
}

void HubFrame::addPM(QString cid, QString output){
    if (!pm.contains(cid)){
        PMWindow *p = new PMWindow(cid, _q(client->getHubUrl().c_str()));
        p->textEdit_CHAT->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(p, SIGNAL(privateMessageClosed(QString)), this, SLOT(slotPMClosed(QString)));
        connect(p->textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));

        MainWindow::getInstance()->addArenaWidget(p);
        MainWindow::getInstance()->addArenaWidgetOnToolbar(p);
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

        QModelIndex left = model->index(item->row(), COLUMN_NICK);
        QModelIndex right= model->index(item->row(), COLUMN_EMAIL);

        model->repaintData(left, right);

        if (needresort)
            model->sort(model->getSortColumn(), model->getSortOrder());
    }
    else{
        if (join && WS->getBool(WB_SHOW_JOINS))
            addStatus(map["NICK"].toString() + tr(" joins the chat"));

        model->addUser(map, user);
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

    if (message.indexOf(_q(client->getMyNick())) >= 0)
        msg_color = WS_CHAT_SAY_NICK;

    bool third = map["3RD"].toBool();

    nick = third? ("* " + nick + " ") : ("<" + nick + "> ");

    WulforUtil::getInstance()->textToHtml(message);
    WulforUtil::getInstance()->textToHtml(nick);

    message = LinkParser::parseForLinks(message);

    message = "<font color=\"" + WSGET(msg_color) + "\">" + message + "</font>";
    output  = time + "<font color=\"" + WSGET(color) + "\"><b>" + nick + "</b> </font>";
    output  += message;

    //WulforUtil::getInstance()->textToHtml(output, false);

    addOutput(output);
}

void HubFrame::newPm(VarMap map){
    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";
    QString color = map["CLR"].toString();
    QString full_message = "";

    nick = map["3RD"].toBool()? ("* " + nick + " ") : ("<" + nick + "> ");

    WulforUtil::getInstance()->textToHtml(message);
    WulforUtil::getInstance()->textToHtml(nick);

    message = LinkParser::parseForLinks(message);

    message       = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + message + "</font>";
    full_message  = time + "<font color=\"" + WSGET(color) + "\"><b>" + nick + "</b> </font>";
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
    QString nick    = " * DC-CORE";

    QString msg     = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + e + "</font>";
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + _q(Util::getTimeString().c_str()) + "]</font>";

    output = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";
    output += msg;

    WulforUtil::getInstance()->textToHtml(output, false);

    pm[cid]->addOutput(output);
}

void HubFrame::getPassword(){
    MainWindow *MW = MainWindow::getInstance();

    if (!MW->isVisible()){
        typedef BFunc0<MainWindow> BFUNC;
        typedef Func0 <HubFrame> FUNC;

        BFUNC *bfunc = new BFUNC(MW, &MainWindow::isVisible);
        FUNC  *func  = new FUNC(this, &HubFrame::getPassword);

        WulforManager::getInstance()->dispatchConditionFunc(bfunc, func);//wait when MainWindow becomes visible
    }
    else {
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

void HubFrame::slotUsersUpdated(){
    label_USERSTATE->setText(QString(tr("Users count: %1 | Total share: %2")).arg(model->rowCount()).arg(_q(Util::formatBytes(total_shared))));
}

void HubFrame::slotReconnect(){
    clearUsers();

    if (client)
        client->connect();
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

    QModelIndexList list;

    if (treeView_USERS->model() != model){
        foreach(QModelIndex i, proxy_list)
            list.push_back(proxy->mapToSource(i));
    }
    else
        list = proxy_list;

    QString cid = (reinterpret_cast<UserListItem*>(list.at(0).internalPointer()))->cid;
    Menu::Action action = Menu::getInstance()->execUserMenu(client, cid);
    UserListItem *item = NULL;

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
            }

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
            }

            break;
        }
        case Menu::FavoriteRem:
        {
            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (item)
                    delUserFromFav(item->cid);
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
#warning "We need to check timestamp mode"
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

    if (cid.isEmpty())
        return;

    Menu::Action action = Menu::getInstance()->execChatMenu(client, cid);

    switch (action){
        case Menu::CopyNick:
        {
            qApp->clipboard()->setText(nick, QClipboard::Clipboard);

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
            editor->setHtml("");

            addStatus(tr("Chat cleared."));

            break;
        }
        case Menu::DisableChat:
        {
            chatDisabled = !chatDisabled;

            plainTextEdit_INPUT->setEnabled(!chatDisabled);

            if (chatDisabled)
                addStatus(tr("Chat disabled."));
            else
                addStatus(tr("Chat enabled."));

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
    QMenu * mcols = new QMenu(this);
    QAction * column;
    int index;

    for (int i = 0; i < model->columnCount(); ++i) {
        index = treeView_USERS->header()->logicalIndex(i);
        column = mcols->addAction(model->headerData(index, Qt::Horizontal).toString());
        column->setCheckable(true);

        column->setChecked(!treeView_USERS->header()->isSectionHidden(index));
        column->setData(index);
    }

    QAction * chosen = mcols->exec(QCursor::pos());

    if (chosen) {
        index = chosen->data().toInt();

        if (treeView_USERS->header()->isSectionHidden(index)) {
            treeView_USERS->header()->showSection(index);
        } else {
            treeView_USERS->header()->hideSection(index);
        }
    }

    delete mcols;
}

void HubFrame::slotShowWnd(){
    if (isVisible())
        return;

   MainWindow *MW = MainWindow::getInstance();

   MW->mapWidgetOnArena(this);
}

void HubFrame::on(ClientListener::Connecting, Client *c) throw(){
    QString status = QString("Connecting to %1...").arg(QString::fromStdString(client->getHubUrl()));

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::Connected, Client*) throw(){
    QString status = QString("Connected to %1...").arg(QString::fromStdString(client->getHubUrl()));

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

    map["NICK"] = _q(user.getIdentity().getNick());
    map["MSG"]  = _q(msg.c_str());
    map["TIME"] = _q(Util::getTimeString());

    QString color = WS_CHAT_USER_COLOR;

    if (user.getIdentity().isHub())
        color = WS_CHAT_STAT_COLOR;
    else if (user.getUser() == client->getMyIdentity().getUser())
        color = WS_CHAT_LOCAL_COLOR;
    else if (user.getIdentity().isOp())
        color = WS_CHAT_OP_COLOR;
    else if (user.getIdentity().isBot())
        color = WS_CHAT_BOT_COLOR;

    map["CLR"] = color;
    map["3RD"] = thirdPerson;

    typedef Func1<HubFrame, VarMap> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::newMsg, map);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::StatusMessage, Client*, const string &msg, int) throw(){
    QString status = QString("%1...").arg(_q(msg.c_str()));

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::PrivateMessage, Client*, const OnlineUser &from, const OnlineUser &to, const OnlineUser &replyTo,
                  const string &msg, bool thirdPerson) throw()
{
    VarMap map;

    const OnlineUser& user = (replyTo.getUser() == ClientManager::getInstance()->getMe()) ? to : replyTo;
    CID id = user.getUser()->getCID();

    QString nick =  _q(from.getIdentity().getNick());

    if (AntiSpam::getInstance() && nick != _q(client->getMyNick())){
        if (AntiSpam::getInstance()->isInBlack(nick))
            return;
        /*else if (!AntiSpam::getInstance()->isInAny(nick)){
            AntiSpam::getInstance()->checkUser(_q(id.toBase32()), _q(msg), _q(client->getHubUrl()));

            if (AntiSpam::getInstance()->isInBlack(nick) || !AntiSpam::getInstance()->isInAny(nick))
                return;
        }*/
    }


    map["NICK"]  = nick;
    map["MSG"]   = _q(msg);
    map["TIME"]  = _q(Util::getTimeString());

    QString color = WS_CHAT_PRIV_USER_COLOR;

    if (nick == _q(client->getMyNick()))
        color = WS_CHAT_PRIV_LOCAL_COLOR;

    map["CLR"] = color;
    map["3RD"] = thirdPerson;
    map["CID"] = _q(id.toBase32());

    typedef Func1<HubFrame, VarMap> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::newPm, map);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::NickTaken, Client*) throw(){
    QString status = QString("Sorry, but nick \"%1\" is already taken by another user.").arg(client->getCurrentNick().c_str());

    typedef Func1<HubFrame, QString> FUNC;
    FUNC *func = new FUNC(this, &HubFrame::addStatus, status);

    QApplication::postEvent(this, new UserCustomEvent(func));
}

void HubFrame::on(ClientListener::SearchFlood, Client*, const string&) throw(){
}
