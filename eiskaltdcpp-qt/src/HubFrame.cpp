/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "HubFrame.h"
#include "MainWindow.h"
#include "PMWindow.h"
#include "WulforUtil.h"
#include "Antispam.h"
#include "HubManager.h"
#include "Notification.h"
#include "ShellCommandRunner.h"
#include "EmoticonDialog.h"
#include "WulforSettings.h"
#include "FlowLayout.h"
#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif

#include "UserListModel.h"

#include "dcpp/LogManager.h"
#include "dcpp/User.h"
#include "dcpp/UserCommand.h"
#include "dcpp/CID.h"
#include "dcpp/HashManager.h"
#include "dcpp/Util.h"

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
#include <QHeaderView>

#include <QtDebug>

#include <exception>

static inline void clearLayout(QLayout *l){
    if (!l)
        return;

    QLayoutItem *item = NULL;
    while ((item = l->takeAt(0)) != NULL){
        l->removeWidget(item->widget());
        item->widget()->deleteLater();

        delete item;
    }

    l->invalidate();
}

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

    // Userlist actions
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

    // Chat actions
    QAction *sep1        = new QAction(NULL);
    QAction *clear_chat  = new QAction(WU->getPixmap(WulforUtil::eiCLEAR), tr("Clear chat"), NULL);
    QAction *find_in_chat= new QAction(WU->getPixmap(WulforUtil::eiFIND), tr("Find in chat"), NULL);
    QAction *dis_chat    = new QAction(WU->getPixmap(WulforUtil::eiFILECLOSE), tr("Disable/Enable chat"), NULL);
    QAction *sep2        = new QAction(NULL);
    QAction *select_all  = new QAction(tr("Select all"), NULL);
    QAction *sep3        = new QAction(NULL);
    QAction *zoom_in     = new QAction(WU->getPixmap(WulforUtil::eiZOOM_IN), tr("Zoom In"), NULL);
    QAction *zoom_out    = new QAction(WU->getPixmap(WulforUtil::eiZOOM_OUT), tr("Zoom Out"), NULL);

    // submenu copy_data for user list
    QAction *copy_data_nick  = new QAction(tr("Nick"), NULL);
    QAction *copy_data_ip    = new QAction(tr("IP"), NULL);
    QAction *copy_data_share = new QAction(tr("Share"), NULL);
    QAction *copy_data_tag   = new QAction(tr("Tag"), NULL);
    QAction *sep4            = new QAction(NULL);
    QAction *copy_data_all   = new QAction(tr("All"), NULL);

    QMenu *menuCopyData = new QMenu(NULL);
    menuCopyData->addActions(QList<QAction*>() << copy_data_nick << copy_data_ip << copy_data_share << copy_data_tag << sep4 << copy_data_all);

    QAction *copy_data   = new QAction(WU->getPixmap(WulforUtil::eiEDITCOPY), tr("Copy data"), NULL);
    copy_data->setMenu(menuCopyData);
    // end submenu

    sep1->setSeparator(true), sep2->setSeparator(true), sep3->setSeparator(true), sep4->setSeparator(true);

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
            << copy_data
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

    chat_actions_map.insert(copy_text, CopyText);
    chat_actions_map.insert(copy_nick, CopyNick);
    chat_actions_map.insert(clear_chat, ClearChat);
    chat_actions_map.insert(find_in_chat, FindInChat);
    chat_actions_map.insert(dis_chat, DisableChat);
    chat_actions_map.insert(select_all, SelectAllChat);
    chat_actions_map.insert(zoom_in, ZoomInChat);
    chat_actions_map.insert(zoom_out, ZoomOutChat);

    chat_actions_map.insert(copy_data_nick,  CopyNick);
    chat_actions_map.insert(copy_data_ip,    CopyIP);
    chat_actions_map.insert(copy_data_share, CopyShare);
    chat_actions_map.insert(copy_data_tag,   CopyTag);
    chat_actions_map.insert(copy_data_all,   CopyText);
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
    menu->setProperty("iconVisibleInMenu", true);

    menu->setTitle(WulforUtil::getInstance()->getNicks(cid));

    menu->addActions(ul_actions);

    QMenu *user_menu = NULL;

    if (!cid.isEmpty()){
        user_menu = WulforUtil::getInstance()->buildUserCmdMenu(QStringList() << _q(client->getHubUrl()),
                        UserCommand::CONTEXT_CHAT);
        menu->addMenu(user_menu);
    }

    QMenu *antispam_menu = NULL;

    if (AntiSpam::getInstance()){
        antispam_menu = new QMenu(NULL);
        antispam_menu->setTitle(tr("AntiSpam"));
        antispam_menu->menuAction()->setIcon(WICON(WulforUtil::eiSPAM));
        antispam_menu->setProperty("iconVisibleInMenu", true);

        antispam_menu->addAction(tr("Add to Black"))->setData(static_cast<int>(AntiSpamBlack));
        antispam_menu->addAction(tr("Add to White"))->setData(static_cast<int>(AntiSpamWhite));

        menu->addMenu(antispam_menu);
    }

    QAction *res = menu->exec(QCursor::pos());

    if (user_menu)
        user_menu->deleteLater();
    if (antispam_menu)
        antispam_menu->deleteLater();

    if (actions.contains(res))
        return static_cast<Action>(actions.indexOf(res));
    else if (chat_actions_map.contains(res))
        return chat_actions_map[res];
    else if (antispam_menu && antispam_menu->actions().contains(res))
        return static_cast<HubFrame::Menu::Action>(res->data().toInt());
    else if (res && !res->toolTip().isEmpty()){//User command{
        last_user_cmd = res->toolTip();
        QString cmd_name = res->statusTip();
        QString hub = res->data().toString();

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
    else
        return None;
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
        user_menu = WulforUtil::getInstance()->buildUserCmdMenu(QStringList() << _q(client->getHubUrl()),
                        UserCommand::CONTEXT_CHAT);
        menu->addMenu(user_menu);
    }

    QMenu *antispam_menu = NULL;

    if (AntiSpam::getInstance()){
        antispam_menu = new QMenu(NULL);
        antispam_menu->setTitle(tr("AntiSpam"));

        antispam_menu->addAction(tr("Add to Black"))->setData(static_cast<int>(AntiSpamBlack));
        antispam_menu->addAction(tr("Add to White"))->setData(static_cast<int>(AntiSpamWhite));

        menu->addMenu(antispam_menu);
    }

    QAction *res = menu->exec(QCursor::pos());

    if (user_menu)
        user_menu->deleteLater();

    title->deleteLater();

    if (actions.contains(res))
        return static_cast<Action>(actions.indexOf(res));
    else if (chat_actions_map.contains(res))
        return chat_actions_map[res];
    else if (antispam_menu && antispam_menu->actions().contains(res))
        return static_cast<HubFrame::Menu::Action>(res->data().toInt());
    else if (res && !res->toolTip().isEmpty()){//User command
        last_user_cmd = res->toolTip();
        QString cmd_name = res->statusTip();
        QString hub = res->data().toString();

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
    else
        return None;
}

QString HubFrame::LinkParser::parseForLinks(QString input, bool use_emot){
    if (input.isEmpty() || input.isNull())
        return input;

    static QList<QChar> unwise_chars = QList<QChar>() << '{' << '}' << '|' << '\\' << '^' << '[' << ']' << '`';

    QString output = "";

    while (!input.isEmpty()){
        for (int j = 0; j < link_types.size(); j++){
            const QString &linktype = link_types.at(j);

            if (input.startsWith(linktype)){
                int l_pos = linktype.length();

                while (l_pos < input.size()){
                    QChar ch = input.at(l_pos);

                    if (ch.isSpace() || ch == '\n'  ||
                        ch == '>'    || ch == '<' || unwise_chars.contains(ch)){
                        break;
                    }
                    else
                        l_pos++;
                }

                QString link = input.left(l_pos);
                QString toshow = link;

                if (linktype == "http://"  || linktype == "https://" || linktype == "ftp://"){
                    while (!QUrl(link).isValid() && !link.isEmpty()){
                        input.prepend(link.at(link.length()-1));
                        link.remove(link.length()-1, 1);
                    }

                    toshow = QUrl::fromEncoded(link.toUtf8()).toString();
                }

                if (linktype == "magnet:"){
                    QString name, tth;
                    int64_t size;

                    WulforUtil::splitMagnet(link, size, tth, name);
                    toshow = QString("%1 (%2)").arg(name).arg(WulforUtil::formatBytes(size));
                }

                if (linktype == "www.")
                    toshow.prepend("http://");

                QString html_link = "";

                if (linktype != "magnet:")
                    html_link = QString("<a href=\"%1\" title=\"%1\" style=\"cursor: hand\">%1</a>").arg(toshow);
                else{
                    html_link = "<a href=\"" + link + "\" title=\"" + toshow + "\" style=\"cursor: hand\">" + toshow + "</a>";
                }

                output += html_link;
                input.remove(0, l_pos);
            }

            if (input.isEmpty())
                break;
        }

        if (input.isEmpty())
            break;

        if (input.startsWith("[b]") && input.indexOf("[/b]") > 0){
            input.remove(0, 3);
            int c_len = input.indexOf("[/b]");

            QString chunk = Qt::escape(input.left(c_len));

            output += "<b>" + chunk + "</b>";
            input.remove(0, c_len+4);

            continue;
        }
        else if (input.startsWith("[u]") && input.indexOf("[/u]") > 0){
            input.remove(0, 3);
            int c_len = input.indexOf("[/u]");

            QString chunk = Qt::escape(input.left(c_len));

            output += "<u>" + chunk + "</u>";
            input.remove(0, c_len+4);

            continue;
        }
        else if (input.startsWith("[i]") && input.indexOf("[/i]") > 0){
            input.remove(0, 3);
            int c_len = input.indexOf("[/i]");

            QString chunk = Qt::escape(input.left(c_len));

            output += "<i>" + chunk + "</i>";
            input.remove(0, c_len+4);

            continue;
        }
        else if (input.startsWith("_") && input.length() >= 3){
            int c_len = input.indexOf("_", 1);

            if (c_len > 1){
                QString chunk = Qt::escape(input.left(c_len));
                chunk.remove(0, 1);

                QChar lastOutputChar = output.isEmpty()? ' ' : (output.at(output.length()-1));

                if (!chunk.contains(QRegExp("\\s")) && (lastOutputChar.isSpace() || lastOutputChar.isPunct())){
                    output += "<u>" + chunk + "</u>";

                    input.remove(0, c_len + 1);
                }
            }
        }
        else if (input.startsWith("*") && input.length() >= 3){
            int c_len = input.indexOf("*", 1);

            if (c_len > 1){
                QString chunk = Qt::escape(input.left(c_len));
                chunk.remove(0, 1);

                QChar lastOutputChar = output.isEmpty()? ' ' : (output.at(output.length()-1));

                if (!chunk.contains(QRegExp("\\s")) && (lastOutputChar.isSpace() || lastOutputChar.isPunct())){
                    output += "<b>" + chunk + "</b>";

                    input.remove(0, c_len + 1);
                }
            }
        }
        else if (input.startsWith("<")){
            output += "&lt;";
            input.remove(0, 1);

            continue;
        }
        else if (input.startsWith(">")){
            output += "&gt;";
            input.remove(0, 1);

            continue;
        }
        else if (input.startsWith("&")){
            input.remove(0, 1);
            output += "&amp;";

            continue;
        }      

        output += input.at(0);

        input.remove(0, 1);
    }

    if (use_emot && WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        output = EmoticonFactory::getInstance()->convertEmoticons(output);

    return output;
}

void HubFrame::LinkParser::parseForMagnetAlias(QString &output){
    int pos = 0;
    QRegExp rx("(<magnet(?:\\s+show=([^>]+))?>(.+)</magnet>)");
    rx.setMinimal(true);
    while ((pos = output.indexOf(rx, pos)) >= 0) {
        QFileInfo fi(rx.cap(3));
        if (fi.isDir() || !fi.exists()) {
            pos++;
            continue;
        }
        QString name = fi.fileName();
        if (!rx.cap(2).isEmpty())
            name = rx.cap(2);

        const TTHValue *tth = HashManager::getInstance()->getFileTTHif(fi.absoluteFilePath().toStdString());
        if (tth != NULL) {
            QString urlStr = WulforUtil::getInstance()->makeMagnet(name, fi.size(), _q(tth->toBase32()));
            output.replace(pos, rx.cap(1).length(), urlStr);
        } else {
            output.replace(pos, rx.cap(1).length(), tr("not shared"));
        }
    }
}

HubFrame::HubFrame(QWidget *parent=NULL, QString hub="", QString encoding=""):
        QWidget(parent),
        total_shared(0),
        arenaMenu(NULL),
        codec(NULL),
        chatDisabled(false),
        hasMessages(false),
        hasHighlightMessages(false),
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

    FavoriteManager::getInstance()->addListener(this);
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

        if ((static_cast<QTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) &&
            (k_e->modifiers() != Qt::ShiftModifier))
        {
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

        if ((static_cast<QTextEdit*>(obj) == plainTextEdit_INPUT) &&
            (!WBGET(WB_USE_CTRL_ENTER) || k_e->modifiers() == Qt::ControlModifier) &&
            ((k_e->key() == Qt::Key_Enter || k_e->key() == Qt::Key_Return) && k_e->modifiers() != Qt::ShiftModifier) ||
             (k_e->key() == Qt::Key_Enter && k_e->modifiers() == Qt::KeypadModifier))
        {
            sendChat(plainTextEdit_INPUT->toPlainText(), false, false);

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
            bool cursoratnick = false;

            if (isChat){
                QTextCursor cursor = textEdit_CHAT->textCursor();
                QString pressedParagraph = cursor.block().text();
                int positionCursor = cursor.columnNumber();

                int l = pressedParagraph.indexOf(" <");
                int r = pressedParagraph.indexOf("> ");

                if (l < r){
                    nick = pressedParagraph.mid(l+2, r-l-2);
                    cid = model->CIDforNick(nick, _q(client->getHubUrl()));
                }
                if ((positionCursor < r) && (positionCursor > l))
                    cursoratnick = true;
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
                        plainTextEdit_INPUT->textCursor().insertText(nick + WSGET(WS_CHAT_SEPARATOR) + " ");
                    else
                        plainTextEdit_INPUT->textCursor().insertText(nick + " ");

                    plainTextEdit_INPUT->setFocus();
                }
                else if (WIGET(WI_CHAT_MDLCLICK_ACT) == 2 && (cursoratnick || isUserList))
                    addPM(cid, "", false);
                else if (cursoratnick || isUserList)
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
            bool cursoratnick = false;
            QTextCursor cursor = textEdit_CHAT->textCursor();

            if (isChat){
                QString nickstatus="",nickmessage="";
                QString pressedParagraph = cursor.block().text();
                //qDebug() << pressedParagraph;
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
                if (!nickmessage.isEmpty() || !nickstatus.isEmpty()) {
                    //qDebug() << nickstatus;
                    //qDebug() << nickmessage;
                    nick = nickmessage + nickstatus;
                    //qDebug() << nick;
                    cid = model->CIDforNick(nick, _q(client->getHubUrl()));
                    //qDebug() << cid;
                    }
                if (((positionCursor < r) && (positionCursor > l))/* || positionCursor > l1*/)
                    cursoratnick = true;
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
                if (WIGET(WI_CHAT_DBLCLICK_ACT) == 1 && (cursoratnick || isUserList)){
                    browseUserFiles(cid, false);
                }
                else if (WIGET(WI_CHAT_DBLCLICK_ACT) == 2 && (cursoratnick || isUserList)){
                    addPM(cid, "", false);
                }
                else if (textEdit_CHAT->anchorAt(textEdit_CHAT->mapFromGlobal(QCursor::pos())).startsWith("user://") || isUserList){//may be dbl click on user nick
                    if (plainTextEdit_INPUT->textCursor().position() == 0)
                        plainTextEdit_INPUT->textCursor().insertText(nick + WSGET(WS_CHAT_SEPARATOR) + " ");
                    else
                        plainTextEdit_INPUT->textCursor().insertText(nick + " ");

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

void HubFrame::closeEvent(QCloseEvent *e){
    MainWindow *MW = MainWindow::getInstance();

    MW->remArenaWidgetFromToolbar(this);
    MW->remWidgetFromArena(this);
    MW->remArenaWidget(this);

    FavoriteManager::getInstance()->removeListener(this);

    HubManager::getInstance()->unregisterHubUrl(_q(client->getHubUrl()));

    client->removeListener(this);
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

    emit closeRequest();
}

void HubFrame::showEvent(QShowEvent *e){
    e->accept();

    HubManager::getInstance()->setActiveHub(this);

    hasMessages = false;
    hasHighlightMessages = false;
    MainWindow::getInstance()->redrawToolPanel();
}

void HubFrame::hideEvent(QHideEvent *e){
    e->accept();

    if (!isVisible())
        HubManager::getInstance()->setActiveHub(NULL);
}

void HubFrame::init(){
    updater = new QTimer();
    updater->setInterval(5000);
    updater->setSingleShot(false);

    model = new UserListModel(this);
    proxy = NULL;

    treeView_USERS->setModel(model);
    treeView_USERS->setSortingEnabled(true);
    treeView_USERS->setItemsExpandable(false);
    treeView_USERS->setUniformRowHeights(true);
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
    toolButton_SMILE->setContextMenuPolicy(Qt::CustomContextMenu);
    toolButton_SMILE->setIcon(WICON(WulforUtil::eiEMOTICON));

    toolButton_HIDE->setIcon(WICON(WulforUtil::eiEDITDELETE));

    frame_SMILES->setLayout(new FlowLayout(frame_SMILES));
    frame_SMILES->setVisible(false);

    QSize sz;
    Q_UNUSED(sz);

    if (EmoticonFactory::getInstance())
        EmoticonFactory::getInstance()->fillLayout(frame_SMILES->layout(), sz);

    foreach(EmoticonLabel *l, frame_SMILES->findChildren<EmoticonLabel*>())
        connect(l, SIGNAL(clicked()), this, SLOT(slotSmileClicked()));

    connect(this, SIGNAL(coreConnecting(QString)), this, SLOT(addStatus(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreConnected(QString)), this, SLOT(addStatus(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUserUpdated(VarMap,dcpp::UserPtr,bool)), this, SLOT(userUpdated(VarMap,dcpp::UserPtr,bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreUserRemoved(dcpp::UserPtr,qlonglong)), this, SLOT(userRemoved(dcpp::UserPtr,qlonglong)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreStatusMsg(QString)), this, SLOT(addStatus(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreFollow(QString)), this, SLOT(follow(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreFailed()), this, SLOT(clearUsers()), Qt::QueuedConnection);
    connect(this, SIGNAL(corePassword()), this, SLOT(getPassword()), Qt::QueuedConnection);
    connect(this, SIGNAL(coreMessage(VarMap)), this, SLOT(newMsg(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(corePrivateMsg(VarMap)), this, SLOT(newPm(VarMap)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreHubUpdated()), MainWindow::getInstance(), SLOT(redrawToolPanel()), Qt::QueuedConnection);
    connect(this, SIGNAL(coreFavoriteUserAdded(QString)), this, SLOT(changeFavStatus(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(coreFavoriteUserRemoved(QString)), this, SLOT(changeFavStatus(QString)), Qt::QueuedConnection);

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
    connect(toolButton_SMILE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSmileContextMenu()));
    connect(pushButton_ALL, SIGNAL(clicked()), this, SLOT(slotFindAll()));
    connect(WulforSettings::getInstance(), SIGNAL(strValueChanged(QString, QString)), this, SLOT(slotSettingsChanged(QString,QString)));
    connect(WulforSettings::getInstance(), SIGNAL(intValueChanged(QString,int)), this, SLOT(slotBoolSettingsChanged(QString,int)));

#ifdef USE_ASPELL
    connect(plainTextEdit_INPUT, SIGNAL(textChanged()), this, SLOT(slotInputTextChanged()));
    connect(plainTextEdit_INPUT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotInputContextMenu()));

    plainTextEdit_INPUT->setContextMenuPolicy(Qt::CustomContextMenu);
#endif

    plainTextEdit_INPUT->setWordWrapMode(QTextOption::NoWrap);
    plainTextEdit_INPUT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    plainTextEdit_INPUT->installEventFilter(this);
    plainTextEdit_INPUT->setAcceptRichText(false);

    textEdit_CHAT->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    textEdit_CHAT->setTabStopWidth(40);
    updateStyles();

    load();

    updater->start();

    completer = new QCompleter(this);
#if QT_VERSION >= 0x040600
    completer->setMaxVisibleItems(10); // This property was introduced in Qt 4.6.
#endif
    plainTextEdit_INPUT->setCompleter(completer, model);

    slotSettingsChanged(WS_APP_EMOTICON_THEME, WSGET(WS_APP_EMOTICON_THEME));//toggle emoticon button
}

void HubFrame::initMenu(){
    WulforUtil *WU = WulforUtil::getInstance();

    delete arenaMenu;

    arenaMenu = new QMenu(tr("Hub menu"), this);

    QAction *reconnect = new QAction(WU->getPixmap(WulforUtil::eiRECONNECT), tr("Reconnect"), arenaMenu);
    QAction *show_wnd  = new QAction(WU->getPixmap(WulforUtil::eiCHAT), tr("Show widget"), arenaMenu);
    QAction *addToFav  = new QAction(WU->getPixmap(WulforUtil::eiFAVSERVER), tr("Add to Favorites"), arenaMenu);
    QMenu   *copyInfo  = new QMenu(tr("Copy"), arenaMenu);
    QAction *copyIP    = copyInfo->addAction(tr("Hub IP"));
    QAction *copyURL   = copyInfo->addAction(tr("Hub URL"));
    QAction *copyTitle = copyInfo->addAction(tr("Hub Title"));

    QAction *sep       = new QAction(arenaMenu);
    sep->setSeparator(true);
    QAction *close_wnd = new QAction(WU->getPixmap(WulforUtil::eiEXIT), tr("Close"), arenaMenu);

    arenaMenu->addActions(QList<QAction*>() << reconnect
                                            << show_wnd
                                            << addToFav
                         );

    arenaMenu->addMenu(copyInfo);

    if (client && client->isConnected()){
        QMenu *u_c = WulforUtil::getInstance()->buildUserCmdMenu(QList<QString>() << _q(client->getHubUrl()), UserCommand::CONTEXT_HUB, arenaMenu);

        if (u_c){
            u_c->setTitle(tr("Hub Menu"));

            arenaMenu->addMenu(u_c);

            connect(u_c, SIGNAL(triggered(QAction*)), this, SLOT(slotHubMenu(QAction*)));
        }
    }

    arenaMenu->addActions(QList<QAction*>() << sep << close_wnd);

    connect(reconnect,  SIGNAL(triggered()), this, SLOT(slotReconnect()));
    connect(show_wnd,   SIGNAL(triggered()), this, SLOT(slotShowWnd()));
    connect(addToFav,   SIGNAL(triggered()), this, SLOT(addAsFavorite()));
    connect(copyIP,     SIGNAL(triggered()), this, SLOT(slotCopyHubIP()));
    connect(copyTitle,  SIGNAL(triggered()), this, SLOT(slotCopyHubTitle()));
    connect(copyURL,    SIGNAL(triggered()), this, SLOT(slotCopyHubURL()));
    connect(close_wnd,  SIGNAL(triggered()), this, SLOT(slotClose()));
}


void HubFrame::save(){
    WSSET(WS_CHAT_USERLIST_STATE, treeView_USERS->header()->saveState().toBase64());
    WISET(WI_CHAT_WIDTH, textEdit_CHAT->width());
    WISET(WI_CHAT_USERLIST_WIDTH, treeView_USERS->width());
    WISET(WI_CHAT_SORT_COLUMN, model->getSortColumn());
    WISET(WI_CHAT_SORT_ORDER, WulforUtil::getInstance()->sortOrderToInt(model->getSortOrder()));
    WSSET("hubframe/chat-background-color", textEdit_CHAT->palette().color(QPalette::Active, QPalette::Base).name());
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
    label_USERSTATE->setVisible(WBGET(WB_USERS_STATISTICS));

    label_LAST_STATUS->setVisible(WBGET(WB_LAST_STATUS));

    if (!WSGET("hubframe/chat-background-color", "").isEmpty()){
        QPalette p = textEdit_CHAT->palette();
        QColor clr = p.color(QPalette::Active, QPalette::Base);

        clr.setNamedColor(WSGET("hubframe/chat-background-color"));

        if (clr.isValid()){
            p.setColor(QPalette::Base, clr);

            textEdit_CHAT->setPalette(p);
        }
    }
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
    if (hasHighlightMessages)
        return WICON(WulforUtil::eiMESSAGE);
    else if (hasMessages)
        return WICON(WulforUtil::eiHUBMSG);
    else
        return WICON(WulforUtil::eiSERVER);
}

void HubFrame::clearChat(){
    textEdit_CHAT->setHtml("");
    addStatus(tr("Chat cleared."));

    updateStyles();

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

void HubFrame::getStatistic(quint64 &users, quint64 &share) const{
    if (model)
        users = model->rowCount();

    share = total_shared;
}

bool HubFrame::isConnected() const {
    return (client? client->isConnected() : false);
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

void HubFrame::sendMsg(const QString &msg){
    sendChat(msg, false, false);
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
        client->hubMessage(msg.toStdString(), thirdPerson);

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
    else if (cmd == "/ratio"){
        double ratio;
        double down = QString(WSGET(WS_APP_TOTAL_DOWN)).toDouble();
        double up = QString(WSGET(WS_APP_TOTAL_UP)).toDouble();

        if (down > 0)
            ratio = up / down;
        else
            ratio = 0;

        QString line = tr("ratio: %1 (uploads: %2, downloads: %3)")
            .arg(QString().setNum(ratio, 'f', 2)).arg(WulforUtil::formatBytes(up)).arg(WulforUtil::formatBytes(down));

        if (param.trimmed() == "show"){
            if (fr == this)
                sendChat(line, true, false);
            else if (pm)
                pm->sendMessage(line, true, false);
        } else if (emptyParam){
            if (fr == this)
                addStatus(line);
            else if (pm)
                pm->addStatus(line);
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
        browseUserFiles(model->CIDforNick(param, _q(client->getHubUrl())), false);
    }
    else if (cmd == "/grant" && !emptyParam){
        grantSlot(model->CIDforNick(param, _q(client->getHubUrl())));
    }
    else if (cmd == "/magnet" && !emptyParam){
        WISET(WI_DEF_MAGNET_ACTION, param.toInt());
    }
    else if (cmd == "/info" && !emptyParam){
        UserListItem *item = model->itemForNick(param, _q(client->getHubUrl()));

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
            pm->sendMessage(line, true, false);
    }
    else if (cmd == "/pm" && !emptyParam){
        addPM(model->CIDforNick(param, _q(client->getHubUrl())), "", false);
    }
    else if (cmd == "/help" || cmd == "/?" || cmd == "/h"){
        QString out = "\n";
#ifdef USE_ASPELL
        out += tr("/aspell on/off - enable/disable spell checking\n");
#endif
        out += tr("/alias <ALIAS_NAME>::<COMMAND> - make alias /ALIAS_NAME to /COMMAND\n");
        out += tr("/alias purge <ALIAS_NAME> - remove alias\n");
        out += tr("/alias list - list all aliases\n");
        out += tr("/away <message> - set away-mode on/off\n");
        out += tr("/back - set away-mode off\n");
        out += tr("/browse <nick> - browse user files\n");
        out += tr("/clear - clear chat window\n");
        out += tr("/magnet - default action with magnet (0-ask, 1-search, 2-download)\n");
        out += tr("/close - close this hub\n");
        out += tr("/fav - add this hub to favorites\n");
        out += tr("/grant <nick> - grant extra slot to user\n");
        out += tr("/help, /?, /h - show this help\n");
        out += tr("/info <nick> - show info about user\n");
        out += tr("/ratio [show] - show ratio [send in chat]\n");
        out += tr("/me - say a third person\n");
        out += tr("/pm <nick> - begin private chat with user\n");
        out += tr("/sh <command> - start command and redirect output to the chat\n");
#ifdef LUA_SCRIPT
        out += tr("/luafile <file> - load Lua file\n");
        out += tr("/lua <chunk> - execute Lua chunk\n");
#endif

        if (out.endsWith("\n"))
            out.remove(out.size()-1, 1);

        if (fr == this)
            addStatus(out);
        else if (pm)
            pm->addStatus(out);
    }
#ifdef LUA_SCRIPT
        else if (cmd == "/lua" && !emptyParam) {
            ScriptManager::getInstance()->EvaluateChunk(Text::fromT(_tq(param)));
        }
        else if( cmd == "/luafile" && !emptyParam) {
            ScriptManager::getInstance()->EvaluateFile(Text::fromT(_tq(param)));
        }
#endif
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

    QString pure_msg;
    QString short_msg = "";
    QString status = "";
    QString nick    = " * ";

    QStringList lines = msg.split(QRegExp("[\\n\\r\\f]+"), QString::SkipEmptyParts);
    for (int i = 0; i < lines.size(); ++i) {
        if (lines.at(i).contains(QRegExp("\\w+"))) {
            short_msg = lines.at(i);
            break;
        }
    }
    if (short_msg.isEmpty() && !lines.isEmpty())
        short_msg = lines.first();

    pure_msg  = LinkParser::parseForLinks(msg, false);
    short_msg = LinkParser::parseForLinks(short_msg, false);
    msg       = LinkParser::parseForLinks(msg, true);

    pure_msg        = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + pure_msg + "</font>";
    short_msg       = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + short_msg + "</font>";
    msg             = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + msg + "</font>";
    QString time    = "";

    if (!WSGET(WS_CHAT_TIMESTAMP).isEmpty())
        time = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP)) + "]</font>";

    status   = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";

    QRegExp rot_msg = QRegExp("is(\\s+)kicking(\\s+)(\\S+)*(\\s+)because:");

    bool isRotating = (msg.indexOf("is kicking because:") >= 0) || (rot_msg.indexIn(msg) >= 0);

    if (!(isRotating && WBGET(WB_CHAT_ROTATING_MSGS)))
        addOutput(status + msg);

    label_LAST_STATUS->setText(status + short_msg);

    status += pure_msg.left(WIGET(WI_CHAT_STATUS_MSG_MAX_LEN));
    WulforUtil::getInstance()->textToHtml(status, false);

    status_msg_history.push_back(status);

    if (WIGET(WI_CHAT_STATUS_HISTORY_SZ) > 0){
        while (status_msg_history.size() > WIGET(WI_CHAT_STATUS_HISTORY_SZ))
            status_msg_history.removeFirst();
    }
    else
        status_msg_history.clear();

    label_LAST_STATUS->setToolTip(status_msg_history.join("<br/>"));
}

void HubFrame::addOutput(QString msg){
    msg.replace("\r", "");
    msg = "<pre>" + msg + "</pre>";
    textEdit_CHAT->append(msg);
}

void HubFrame::addPM(QString cid, QString output, bool keepfocus){
    if (!pm.contains(cid)){
        PMWindow *p = new PMWindow(cid, _q(client->getHubUrl().c_str()));
        p->textEdit_CHAT->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(p, SIGNAL(privateMessageClosed(QString)), this, SLOT(slotPMClosed(QString)));
        connect(p, SIGNAL(inputTextChanged()), this, SLOT(slotInputTextChanged()));
        connect(p, SIGNAL(inputTextMenu()), this, SLOT(slotInputContextMenu()));
        connect(p->textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));

        MainWindow::getInstance()->addArenaWidget(p);
        MainWindow::getInstance()->addArenaWidgetOnToolbar(p, WBGET(WB_CHAT_KEEPFOCUS));

        if (!keepfocus || !WBGET(WB_CHAT_KEEPFOCUS))
            MainWindow::getInstance()->mapWidgetOnArena(p);

        p->setCompleter(completer, model);

        p->addOutput(output);

        p->setAttribute(Qt::WA_DeleteOnClose);

        pm.insert(cid, p);
    }
    else{
        PMMap::iterator it = pm.find(cid);

        if (output.indexOf(_q(client->getMyNick())) >= 0)
            it.value()->setHasHighlightMessages(true);

        if (!keepfocus || !WBGET(WB_CHAT_KEEPFOCUS))
            MainWindow::getInstance()->mapWidgetOnArena(it.value());

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

void HubFrame::userUpdated(const HubFrame::VarMap &map, const UserPtr &user, bool join){
    static WulforUtil *WU = WulforUtil::getInstance();
    static WulforSettings *WS = WulforSettings::getInstance();
    static bool showFavJoinsOnly = WS->getBool(WB_CHAT_SHOW_JOINS_FAV);

    if (!model)
        return;

    UserListItem *item = model->itemForPtr(user);

    QString cid = map["CID"].toString();
    QString nick = map["NICK"].toString();

    if (item){
        bool isOp = map["ISOP"].toBool();

        total_shared -= item->share;

        item->nick = nick;
        item->comm = map["COMM"].toString();
        item->conn = map["CONN"].toString();
        item->email= map["EMAIL"].toString();
        item->ip   = map["IP"].toString();
        item->share= map["SHARE"].toULongLong();
        item->tag  = map["TAG"].toString();
        item->isOp = isOp;
        item->px = WU->getUserIcon(user, map["AWAY"].toBool(), item->isOp, map["SPEED"].toString());

        model->repaintItem(item);
        model->needResort();
    }
    else{
        if (join && WS->getBool(WB_CHAT_SHOW_JOINS)){
            do {
                if (showFavJoinsOnly && !FavoriteManager::getInstance()->isFavoriteUser(user))
                    break;

                addStatus(nick + tr(" joins the chat"));
            } while (0);
        }

        model->addUser(nick, map["SHARE"].toULongLong(),
                       map["COMM"].toString(), map["TAG"].toString(),
                       map["CONN"].toString(), map["IP"].toString(),
                       map["EMAIL"].toString(), map["ISOP"].toBool(),
                       map["AWAY"].toBool(), map["SPEED"].toString(),
                       cid, user);

        if (FavoriteManager::getInstance()->isFavoriteUser(user))
            Notification::getInstance()->showMessage(Notification::FAVORITE, tr("Favorites"), tr("%1 become online").arg(nick));

        if (pm.contains(nick)){
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

void HubFrame::userRemoved(const dcpp::UserPtr &user, qlonglong share){
    total_shared -= share;

    QString cid = _q(user->getCID().toBase32());
    QString nick = "";
    UserListItem *item = model->itemForPtr(user);

    if (item)
        nick = item->nick;

    if (pm.contains(cid)){
        pmUserOffline(cid);

        PMWindow *pmw = pm[cid];

        pm.insert(nick, pmw);

        pmw->cid = nick;
        pmw->plainTextEdit_INPUT->setEnabled(false);//we need interface function

        pm.remove(cid);
    }

    if (WulforSettings::getInstance()->getBool(WB_CHAT_SHOW_JOINS)){
        do {
            if (WulforSettings::getInstance()->getBool(WB_CHAT_SHOW_JOINS_FAV) &&
                !FavoriteManager::getInstance()->isFavoriteUser(user))
                break;

            addStatus(nick + tr(" left the chat"));
        } while (0);
    }

    if (FavoriteManager::getInstance()->isFavoriteUser(user))
        Notification::getInstance()->showMessage(Notification::FAVORITE, tr("Favorites"), tr("%1 become offline").arg(nick));

    model->removeUser(user);
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

    string cid = id.toStdString();

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user){
        if (user != ClientManager::getInstance()->getMe() && !FavoriteManager::getInstance()->isFavoriteUser(user))
            FavoriteManager::getInstance()->addFavoriteUser(user);
    }
}

void HubFrame::delUserFromFav(const QString& id){
    if (id.isEmpty())
        return;

    string cid = id.toStdString();

    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user){
        if (user != ClientManager::getInstance()->getMe() && FavoriteManager::getInstance()->isFavoriteUser(user))
            FavoriteManager::getInstance()->removeFavoriteUser(user);
    }
}

void HubFrame::changeFavStatus(const QString &id) {
    if (id.isEmpty())
        return;

    UserPtr user = ClientManager::getInstance()->findUser(CID(id.toStdString()));

    if (user) {
        UserListItem *item = NULL;
        if (model)
            item = model->itemForPtr(user);

        bool bFav = FavoriteManager::getInstance()->isFavoriteUser(user);

        if (item) {
            item->fav = bFav;
            QModelIndex ixb = model->index(item->row(), COLUMN_NICK);
            QModelIndex ixe = model->index(item->row(), COLUMN_EMAIL);

            model->repaintData(ixb, ixe);
        }

        QString message = WulforUtil::getInstance()->getNicks(id) +
                (bFav ? tr(" has been added to favorites.") : tr(" has been removed from favorites."));

        MainWindow::getInstance()->setStatusMessage(message);
    }
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

void HubFrame::newMsg(const VarMap &map){
    QString output = "";

    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";;
    QString color = map["CLR"].toString();
    QString msg_color = WS_CHAT_MSG_COLOR;

    emit newMessage(this, _q(client->getHubUrl()), map["CID"].toString(), nick, message);

    if (message.indexOf(_q(client->getMyNick())) >= 0){
        msg_color = WS_CHAT_SAY_NICK;

        Notification::getInstance()->showMessage(Notification::NICKSAY, getArenaTitle().left(20), nick + ": " + message);
    }

    bool third = map["3RD"].toBool();

    nick = third? ("* " + nick + " ") : ("<" + nick + "> ");

    message = LinkParser::parseForLinks(message, true);

    WulforUtil::getInstance()->textToHtml(nick, true);

    message = "<font color=\"" + WSGET(msg_color) + "\">" + message + "</font>";

    output  += time;
    string info= Util::formatAdditionalInfo(map["I4"].toString().toStdString(),BOOLSETTING(USE_IP),BOOLSETTING(GET_USER_COUNTRY));

    if (!info.empty())
        output  += " <font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">" + _q(info) + "</font>";

    output  += QString(" <a style=\"text-decoration:none\" href=\"user://%1\"><font color=\"%2\"><b>%3</b></font></a>")
               .arg(nick).arg(WSGET(color)).arg(nick.replace("\"", "&quot;"));
    output  += message;

    addOutput(output);

    if (!isVisible()){
        if (msg_color == WS_CHAT_SAY_NICK)
            hasHighlightMessages = true;

        hasMessages = true;

        MainWindow::getInstance()->redrawToolPanel();
    }
}

void HubFrame::newPm(const VarMap &map){
    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";
    QString color = map["CLR"].toString();
    QString full_message = "";

    if (nick != _q(client->getMyNick()))
        Notification::getInstance()->showMessage(Notification::PM, nick, message);

    bool third = map["3RD"].toBool();

    if (message.startsWith("/me ")){
        message.remove(0, 4);
        third = true;
    }

    nick = third? ("* " + nick + " ") : ("<" + nick + "> ");

    message = LinkParser::parseForLinks(message, true);

    WulforUtil::getInstance()->textToHtml(nick, true);

    message       = "<font color=\"" + WSGET(WS_CHAT_MSG_COLOR) + "\">" + message + "</font>";
    full_message  += time;
    string info= Util::formatAdditionalInfo(map["I4"].toString().toStdString(),BOOLSETTING(USE_IP),BOOLSETTING(GET_USER_COUNTRY));

    if (!info.empty())
        full_message += " <font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">" + _q(info) + "</font>";

    full_message  += QString(" <a style=\"text-decoration:none\" href=\"user://%1\"><font color=\"%2\"><b>%3</b></font></a>")
                     .arg(nick).arg(WSGET(color)).arg(nick.replace("\"", "&quot;"));
    full_message  += message;

    WulforUtil::getInstance()->textToHtml(full_message, false);

    addPM(map["CID"].toString(), full_message);
}

void HubFrame::createPMWindow(const QString &nick){
    createPMWindow(CID(_tq(model->CIDforNick(nick, _q(client->getHubUrl())))));
}

void HubFrame::createPMWindow(const dcpp::CID &cid){
    addPM(_q(cid.toBase32()), "");
}

bool HubFrame::hasCID(const dcpp::CID &cid, const QString &nick){
    return (model->CIDforNick(nick, _q(client->getHubUrl())) == _q(cid.toBase32()));
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
    QString time    = "";

    if (!WSGET(WS_CHAT_TIMESTAMP).isEmpty())
        time = "<font color=\""+WSGET(WS_CHAT_TIME_COLOR)+">["+QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP))+"]</font>";

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
        QString pass = QInputDialog::getText(this, _q(client->getHubUrl()), tr("Password"), QLineEdit::Password);

        if (!pass.isEmpty()){
            client->setPassword(pass.toStdString());
            client->password(pass.toStdString());
        }
        else
            client->disconnect(true);
    }
}

void HubFrame::follow(QString redirect){
    if(!redirect.isEmpty()) {
        if(ClientManager::getInstance()->isConnected(_tq(redirect))) {
            addStatus(tr("Redirect request received to a hub that's already connected"));
            return;
        }

        string url = _tq(redirect);

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

void HubFrame::updateStyles(){
    QString custom_font_desc = WSGET(WS_CHAT_FONT);
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

    custom_font_desc = WSGET(WS_CHAT_ULIST_FONT);

    if (!custom_font_desc.isEmpty() && custom_font.fromString(custom_font_desc))
        treeView_USERS->setFont(custom_font);
}

void HubFrame::slotActivate(){
    plainTextEdit_INPUT->setFocus();
}

void HubFrame::slotUsersUpdated(){
    if (treeView_USERS->model() == proxy){
        label_USERSTATE->setText(QString(tr("Users count: %3/%1 | Total share: %2"))
                                 .arg(model->rowCount())
                                 .arg(WulforUtil::formatBytes(total_shared))
                                 .arg(proxy->rowCount()));
    }
    else {
        label_USERSTATE->setText(QString(tr("Users count: %1 | Total share: %2"))
                                 .arg(model->rowCount())
                                 .arg(WulforUtil::formatBytes(total_shared)));
    }

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
                    addPM(item->cid, "", false);

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
                    ttip += getUserInfo(item) + "\n";

                ttip += "\n";
            }

            if (!ttip.isEmpty())
                qApp->clipboard()->setText(ttip, QClipboard::Clipboard);

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

            qApp->clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case Menu::CopyIP:
        {
            QString ret = "";

            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (ret.length() > 0)
                    ret += "\n";

                if (item)
                    ret += item->ip;
            }

            qApp->clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case Menu::CopyShare:
        {
            QString ret = "";

            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (ret.length() > 0)
                    ret += "\n";

                if (item)
                    ret += WulforUtil::formatBytes(item->share);
            }

            qApp->clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case Menu::CopyTag:
        {
            QString ret = "";

            foreach(QModelIndex i, list){
                item = reinterpret_cast<UserListItem*>(i.internalPointer());

                if (ret.length() > 0)
                    ret += "\n";

                if (item)
                    ret += item->tag;
            }

            qApp->clipboard()->setText(ret, QClipboard::Clipboard);

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
        case Menu::AntiSpamWhite:
        {

            if (AntiSpam::getInstance()){
                foreach(QModelIndex i, list){
                    item = reinterpret_cast<UserListItem*>(i.internalPointer());

                    (*AntiSpam::getInstance()) << eIN_WHITE << item->nick;
                }
            }

            break;
        }
        case Menu::AntiSpamBlack:
        {
            if (AntiSpam::getInstance()){
                foreach(QModelIndex i, list){
                    item = reinterpret_cast<UserListItem*>(i.internalPointer());

                    (*AntiSpam::getInstance()) << eIN_BLACK << item->nick;
                }
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

    cursor.movePosition(QTextCursor::StartOfBlock);

    pressedParagraph = cursor.block().text();

    int row_counter = 0;

    while (!pressedParagraph.contains(QRegExp("(<(\\w+)>)")) && row_counter < 600){//try to find nick in above rows (max 600 rows)
        cursor.movePosition(QTextCursor::PreviousBlock);
        pressedParagraph = cursor.block().text();
        row_counter++;
    }

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

    QString cid = model->CIDforNick(nick, _q(client->getHubUrl()));

    if (cid.isEmpty()){
        QMenu *m = editor->createStandardContextMenu(QCursor::pos());
        m->exec(QCursor::pos());

        delete m;

        return;
    }

    QPoint p = QCursor::pos();

    bool pmw = (editor != this->textEdit_CHAT);

    Menu::Action action = Menu::getInstance()->execChatMenu(client, cid, pmw);

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

            qApp->clipboard()->setText(ret, QClipboard::Clipboard);

            break;
        }
        case Menu::CopyNick:
        {
            qApp->clipboard()->setText(nick, QClipboard::Clipboard);

            break;
        }
        case Menu::FindInList:
        {
            UserListItem *item = model->itemForNick(nick, _q(client->getHubUrl()));

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
            addPM(cid, "", false);

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
        case Menu::AntiSpamWhite:
        {
            if (AntiSpam::getInstance())
                (*AntiSpam::getInstance()) << eIN_WHITE << nick;

            break;
        }
        case Menu::AntiSpamBlack:
        {
            if (AntiSpam::getInstance())
                (*AntiSpam::getInstance()) << eIN_BLACK << nick;

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

        LinkParser::parseForMagnetAlias(output);

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
            proxy = new UserListProxyModel();
            proxy->setDynamicSortFilter(true);
            proxy->setSourceModel(model);
        }

        bool isRegExp = false;

        if (text.startsWith("##")){
            isRegExp = true;
            text.remove(0, 2);
        }

        if (!isRegExp){
            proxy->setFilterFixedString(text);
            proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        }
        else{
            proxy->setFilterRegExp(text);
            proxy->setFilterCaseSensitivity(Qt::CaseSensitive);
        }

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

    if (WBGET(WB_CHAT_USE_SMILE_PANEL)){
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

void HubFrame::slotSmileClicked(){
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

    if (WBGET(WB_CHAT_HIDE_SMILE_PANEL))
        frame_SMILES->setVisible(false);
}

void HubFrame::slotSmileContextMenu(){
#ifndef WIN32
    QString emot = CLIENT_DATA_DIR "/emoticons/";
#else
    QString emot = qApp->applicationDirPath()+QDir::separator()+CLIENT_DATA_DIR "/emoticons/";
#endif//WIN32

    QMenu *m = new QMenu(this);

    foreach (QString f, QDir(emot).entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot)){
        if (!f.isEmpty()){
            QAction * act = m->addAction(f);
            act->setCheckable(true);

            if (f == WSGET(WS_APP_EMOTICON_THEME)){
                act->setChecked(false);
                act->setChecked(true);
            }
        }
    }

    QAction *a = m->exec(QCursor::pos());

    if (a && a->isChecked())
        WSSET(WS_APP_EMOTICON_THEME, a->text());

    m->deleteLater();
}

void HubFrame::slotInputTextChanged(){
#ifdef USE_ASPELL
    PMWindow *p = qobject_cast<PMWindow*>(sender());
    QTextEdit *plainTextEdit_INPUT = (p)? qobject_cast<QTextEdit*>(p->inputWidget()) : this->plainTextEdit_INPUT;

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

    QTextEdit::ExtraSelection selection;
    selection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
    selection.format.setUnderlineColor(Qt::red);

    bool ok = false;
    while (!words.empty()){
        QString s = words.takeLast();

        if ((s.toLongLong(&ok) && ok) || !QUrl(s).scheme().isEmpty())
            continue;

        if (plainTextEdit_INPUT->find(s, QTextDocument::FindBackward) && !sp->ok(s)){
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
    QTextEdit *plainTextEdit_INPUT = (p)? qobject_cast<QTextEdit*>(p->inputWidget()) : this->plainTextEdit_INPUT;

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
            QAction *add_to_dict = new QAction(tr("Add to dictionary"), m);

            m->addAction(add_to_dict);

            QMenu *ss = NULL;
            if (!list.isEmpty()) {
                ss = new QMenu(tr("Suggestions"), this);


                foreach (QString s, list)
                    ss->addAction(s);

                m->addMenu(ss);
            }

            QAction *ret = m->exec(QCursor::pos());

            if (ret == add_to_dict)
                sp->addToDict(word);
            else if (ss && ret && ret->parent() == ss){
                c.removeSelectedText();

                c.insertText(ret->text());
            }

            m->deleteLater();

            if (ss)
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

        int id = FavoriteManager::getInstance()->findUserCommand(cmd_name.toStdString(), hub.toStdString());
        UserCommand uc;

        if (id == -1 || !FavoriteManager::getInstance()->getUserCommand(id, uc))
            return;

        StringMap params;

        if (WulforUtil::getInstance()->getUserCommandParams(last_user_cmd, params)){
            client->getMyIdentity().getParams(params, "my", true);
            client->getHubIdentity().getParams(params, "hub", false);

            client->escapeParams(params);
            client->sendUserCmd(Util::formatParams(uc.getCommand(), params, false));
        }
    }
}

void HubFrame::slotSettingsChanged(const QString &key, const QString &value){
    Q_UNUSED(value);

    if (key == WS_CHAT_FONT || key == WS_CHAT_ULIST_FONT)
        updateStyles();
    else if (key == WS_APP_EMOTICON_THEME){
        if (EmoticonFactory::getInstance()){
            EmoticonFactory::getInstance()->load();

            frame_SMILES->setVisible(false);

            clearLayout(frame_SMILES->layout());

            QSize sz;
            Q_UNUSED(sz);

            EmoticonFactory::getInstance()->fillLayout(frame_SMILES->layout(), sz);

            foreach(EmoticonLabel *l, frame_SMILES->findChildren<EmoticonLabel*>())
                connect(l, SIGNAL(clicked()), this, SLOT(slotSmileClicked()));
        }
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
}

void HubFrame::slotBoolSettingsChanged(const QString &key, int value){
    if (key == WB_APP_ENABLE_EMOTICON){
        bool enable = static_cast<bool>(value);

        if (enable){
            EmoticonFactory::newInstance();
            EmoticonFactory::getInstance()->load();

            frame_SMILES->setVisible(false);

            clearLayout(frame_SMILES->layout());

            QSize sz;
            Q_UNUSED(sz);

            EmoticonFactory::getInstance()->fillLayout(frame_SMILES->layout(), sz);

            foreach(EmoticonLabel *l, frame_SMILES->findChildren<EmoticonLabel*>())
                connect(l, SIGNAL(clicked()), this, SLOT(slotSmileClicked()));

        }
        else{
            if (EmoticonFactory::getInstance())
                EmoticonFactory::deleteInstance();

            frame_SMILES->setVisible(false);

            clearLayout(frame_SMILES->layout());
        }

        toolButton_SMILE->setVisible(enable);
    }
}

void HubFrame::slotCopyHubIP(){
    if (client && client->isConnected()){
        qApp->clipboard()->setText(_q(client->getIp()), QClipboard::Clipboard);
    }
}

void HubFrame::slotCopyHubTitle(){
    if (client && client->isConnected()){
        qApp->clipboard()->setText(QString("%1 - %2").arg(_q(client->getHubName())).arg(_q(client->getHubDescription())), QClipboard::Clipboard);
    }
}

void HubFrame::slotCopyHubURL(){
    if (client && client->isConnected()){
        qApp->clipboard()->setText(_q(client->getHubUrl()), QClipboard::Clipboard);
    }
}

void HubFrame::on(FavoriteManagerListener::UserAdded, const FavoriteUser& aUser) throw() {
    emit coreFavoriteUserAdded(_q(aUser.getUser()->getCID().toBase32()));
}

void HubFrame::on(FavoriteManagerListener::UserRemoved, const FavoriteUser& aUser) throw() {
    emit coreFavoriteUserRemoved(_q(aUser.getUser()->getCID().toBase32()));
}

void HubFrame::on(ClientListener::Connecting, Client *c) throw(){
    QString status = tr("Connecting to %1").arg(QString::fromStdString(client->getHubUrl()));

    emit coreConnecting(status);
}

void HubFrame::on(ClientListener::Connected, Client*) throw(){
    QString status = tr("Connected to %1").arg(QString::fromStdString(client->getHubUrl()));

    emit coreConnected(status);

    HubManager::getInstance()->registerHubUrl(_q(client->getHubUrl()), this);
}

void HubFrame::on(ClientListener::UserUpdated, Client*, const OnlineUser &user) throw(){
    if (user.getIdentity().isHidden() && !WBGET(WB_SHOW_HIDDEN_USERS))
        return;

    VarMap params;

    getParams(params, user.getIdentity());

    emit coreUserUpdated(params, user, true);
}

void HubFrame::on(ClientListener::UsersUpdated x, Client*, const OnlineUserList &list) throw(){
    bool showHidden = WBGET(WB_SHOW_HIDDEN_USERS);

    for (OnlineUserList::const_iterator it = list.begin(); it != list.end(); ++it){
        if ((*(*it)).getIdentity().isHidden() && !showHidden)
            break;

        VarMap params;

        getParams(params, (*(*it)).getIdentity());

        emit coreUserUpdated(params, (*(*it)), true);
    }
}

void HubFrame::on(ClientListener::UserRemoved, Client*, const OnlineUser &user) throw(){
    if (user.getIdentity().isHidden() && !WBGET(WB_SHOW_HIDDEN_USERS))
        return;

    emit coreUserRemoved(user.getUser(), user.getIdentity().getBytesShared());
}

void HubFrame::on(ClientListener::Redirect, Client*, const string &link) throw(){
    if(ClientManager::getInstance()->isConnected(link)) {
        emit coreStatusMsg(tr("Redirect request received to a hub that's already connected"));

        return;
    }

    if(BOOLSETTING(AUTO_FOLLOW))
        emit coreFollow(_q(link));
}

void HubFrame::on(ClientListener::Failed, Client*, const string &msg) throw(){
    QString status = tr("Fail: %1...").arg(_q(msg));

    emit coreStatusMsg(status);
    emit coreFailed();
}

void HubFrame::on(GetPassword, Client*) throw(){
    emit corePassword();
}

void HubFrame::on(ClientListener::HubUpdated, Client*) throw(){
    emit coreHubUpdated();
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

        emit coreStatusMsg(_q(user.getIdentity().getNick()) + " " + m);

        return;
    }

    map["NICK"] = _q(user.getIdentity().getNick());
    map["MSG"]  = _q(msg.c_str());
    map["TIME"] = QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP));

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
    map["I4"]  = _q(ClientManager::getInstance()->getOnlineUserIdentity(user).getIp());

    emit coreMessage(map);

    if (BOOLSETTING(LOG_MAIN_CHAT)){
        StringMap params;
        params["message"] = Util::formatMessage(user.getIdentity().getNick(), msg, thirdPerson);
        client->getHubIdentity().getParams(params, "hub", false);
        params["hubURL"] = client->getHubUrl();
        params["userI4"] = ClientManager::getInstance()->getOnlineUserIdentity(user).getIp();
        client->getMyIdentity().getParams(params, "my", true);
        LOG(LogManager::CHAT, params);
    }
}

void HubFrame::on(ClientListener::StatusMessage, Client*, const string &msg, int) throw(){
    QString status = QString("%1 ").arg(_q(msg));

    emit coreStatusMsg(status);

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

    bool isBot = user.getIdentity().isBot();
    bool isHub = user.getIdentity().isHub();
    bool isOp = user.getIdentity().isOp();

    if (isHub && BOOLSETTING(IGNORE_HUB_PMS))
        return;
    else if (isBot && BOOLSETTING(IGNORE_BOT_PMS))
        return;

    VarMap map;
    CID id           = user.getUser()->getCID();
    QString nick     =  _q(from.getIdentity().getNick());
    bool isInSandBox = false;
    bool isEcho      = (from.getUser() == ClientManager::getInstance()->getMe());
    bool hasPMWindow = pm.contains(_q(id.toBase32()));//PMWindow is created

    if (AntiSpam::getInstance())
        isInSandBox = AntiSpam::getInstance()->isInSandBox(_q(id.toBase32()));

    if (AntiSpam::getInstance() && !isEcho){
        do {
            if (hasPMWindow)
                break;

            if (isOp && !WBGET(WB_ANTISPAM_FILTER_OPS) && !isBot)
                break;

            if (AntiSpam::getInstance()->isInBlack(nick))
                return;
            else if (!(AntiSpam::getInstance()->isInWhite(nick) || AntiSpam::getInstance()->isInGray(nick))){
                AntiSpam::getInstance()->checkUser(_q(id.toBase32()), _q(msg), _q(client->getHubUrl()));

                return;
            }
        } while (0);
    }
    else if (isEcho && isInSandBox && !hasPMWindow)
        return;

    map["NICK"]  = nick;
    map["MSG"]   = _q(msg);
    map["TIME"]  = QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP));

    QString color = WS_CHAT_PRIV_USER_COLOR;

    if (nick == _q(client->getMyNick()))
        color = WS_CHAT_PRIV_LOCAL_COLOR;
    else if (isOp)
        color = WS_CHAT_OP_COLOR;
    else if (isBot)
        color = WS_CHAT_BOT_COLOR;
    else if (isHub)
        color = WS_CHAT_STAT_COLOR;

    map["CLR"] = color;
    map["3RD"] = thirdPerson;
    map["CID"] = _q(id.toBase32());
    map["I4"]  = _q(ClientManager::getInstance()->getOnlineUserIdentity(from).getIp());

    if (WBGET(WB_CHAT_REDIRECT_BOT_PMS) && isBot)
        emit coreMessage(map);
    else
        emit corePrivateMsg(map);

    if (BOOLSETTING(LOG_PRIVATE_CHAT)){
        StringMap params;
        params["message"] = Util::formatMessage(from.getIdentity().getNick(), msg, thirdPerson);
        params["hubNI"] = _tq(WulforUtil::getInstance()->getHubNames(id));
        params["hubURL"] = Util::toString(ClientManager::getInstance()->getHubs(id));
        params["userCID"] = id.toBase32();
        params["userNI"] = ClientManager::getInstance()->getNicks(id)[0];
        params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
        params["userI4"] = ClientManager::getInstance()->getOnlineUserIdentity(from).getIp();
        LOG(LogManager::PM, params);
    }

    if (!(isBot || isHub) && from.getUser() != ClientManager::getInstance()->getMe() && Util::getAway())
        ClientManager::getInstance()->privateMessage(user.getUser(), Util::getAwayMessage(), false, client->getHubUrl());
}

void HubFrame::on(ClientListener::NickTaken, Client*) throw(){
    QString status = tr("Sorry, but nick \"%1\" is already taken by another user.").arg(client->getCurrentNick().c_str());

    emit coreStatusMsg(status);
}

void HubFrame::on(ClientListener::SearchFlood, Client*, const string &str) throw(){
    emit coreStatusMsg(tr("Search flood detected: %1").arg(_q(str)));
}
