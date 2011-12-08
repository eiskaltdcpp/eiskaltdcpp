/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "ArenaWidgetFactory.h"
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
#include "SearchFrame.h"
#ifdef USE_ASPELL
#include "SpellCheck.h"
#endif
#include "ArenaWidgetManager.h"

#include "UserListModel.h"
#include "EmoticonFactory.h"

#include "dcpp/LogManager.h"
#include "dcpp/User.h"
#include "dcpp/UserCommand.h"
#include "dcpp/CID.h"
#include "dcpp/HashManager.h"
#include "dcpp/Util.h"
#include "dcpp/ChatMessage.h"

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
#include <QTextLayout>
#include <QTextDocument>
#include <QUrl>
#include <QCloseEvent>
#include <QThread>
#include <QRegExp>
#include <QScrollBar>
#include <QShortcut>
#include <QHeaderView>

#include <QtDebug>

#include <exception>

class HubFramePrivate {
    typedef QMap<QString, PMWindow*> PMMap;
    typedef QMap<QString, QVariant> VarMap;
    typedef QList<ShellCommandRunner*> ShellList;
public:
    QTimer *updater;

    QMenu *arenaMenu;

    Client *client;

    // Work data
    QTextCodec *codec;

    quint64 total_shared;
    QString hub_title;

    bool chatDisabled;
    bool hasMessages;
    bool hasHighlightMessages;
    bool drawLine;

    QStringList status_msg_history;
    QStringList out_messages;
    int out_messages_index;
    bool out_messages_unsent;

    PMMap pm;
    ShellList shell_list;

    // Userlist data and some helpful functions
    UserListModel *model;
    UserListProxyModel *proxy;

    QCompleter * completer;    
};

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

//replace [tag]some_text[/tag] with <txt>some_text</txt>
static bool parseBasicBBCode(const QString &tag, const QString &txt, QString &input, QString &output){
    if (tag.isEmpty())
        return false;

    const QString &bbCode1 = QString("[%1]").arg(tag);
    const QString &bbCode2 = QString("[/%1]").arg(tag);

    if (input.startsWith(bbCode1) && input.indexOf(bbCode2) >= bbCode1.length()){
        input.remove(0, bbCode1.length());
        int c_len = input.indexOf(bbCode2);

        const QString &chunk = HubFrame::LinkParser::parseForLinks(input.left(c_len), false);

        output += QString("<%1>").arg(txt) + chunk + QString("</%1>").arg(txt);
        input.remove(0, c_len+bbCode2.length());

        return true;
    }

    return false;
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
    QAction *search_text = new QAction(WU->getPixmap(WulforUtil::eiFIND), tr("Search text"), NULL);
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
            << search_text
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
    chat_actions_map.insert(search_text, SearchText);
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

    menu->setTitle(WulforUtil::getInstance()->getNicks(cid, _q(client->getHubUrl())));

    if (menu->title().isEmpty())
        menu->setTitle(tr("[User went offline]"));

    menu->addActions(ul_actions);

    QMenu *user_menu = NULL;

    if (!cid.isEmpty()){
        user_menu = WulforUtil::getInstance()->buildUserCmdMenu(QStringList()
                        << _q(client->getHubUrl()), UserCommand::CONTEXT_USER);

        if (user_menu->actions().size() > 0)
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
                ClientManager::getInstance()->userCommand(HintedUser(user, client->getHubUrl()), uc, params, true);

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

    QAction *title = new QAction(WulforUtil::getInstance()->getNicks(cid, _q(client->getHubUrl())), menu);
    QFont f;
    f.setBold(true);
    title->setFont(f);
    title->setEnabled(false);

    if (title->text().isEmpty())
        title->setText(tr("[User went offline]"));

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
        user_menu = WulforUtil::getInstance()->buildUserCmdMenu(QStringList()
                        << _q(client->getHubUrl()), UserCommand::CONTEXT_HUB);

    if (user_menu->actions().size() > 0)
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
                ClientManager::getInstance()->userCommand(HintedUser(user, client->getHubUrl()), uc, params, true);

            return UserCommands;
        }
        else
            return None;
    }
    else
        return None;
}

QString HubFrame::LinkParser::parseForLinks(QString input, bool use_emot){
    if (input.isEmpty())
        return input;

    static QList<QChar> unwise_chars = QList<QChar>() << '{' << '}' << '|' << '\\' << '^' << '[' << ']' << '`';

    QString output = "";

    EmoticonMap emoticons;

    if (use_emot && WBGET(WB_APP_ENABLE_EMOTICON) && EmoticonFactory::getInstance())
        emoticons = EmoticonFactory::getInstance()->getEmoticons();

    const QString &emo_theme = WSGET(WS_APP_EMOTICON_THEME);
    bool force_emot = WBGET(WB_APP_FORCE_EMOTICONS);

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
                    QString magnet = link;

                    QUrl u;

                    if (!magnet.contains("+"))
                        u.setEncodedUrl(magnet.toAscii());
                    else {
                        QString _l = magnet;

                        _l.replace("+", "%20");
                        u.setEncodedUrl(_l.toAscii());
                    }

                    if (u.hasQueryItem("kt")){
                        QString keywords = u.queryItemValue("kt");
                        QString hub = u.hasQueryItem("xs")? u.queryItemValue("xs") : "";

                        if (!(hub.startsWith("dchub://", Qt::CaseInsensitive) ||
                              hub.startsWith("adc://", Qt::CaseInsensitive) ||
                              hub.startsWith("adcs://", Qt::CaseInsensitive)) && !hub.isEmpty())
                            hub.prepend("dchub://");

                        if (keywords.isEmpty())
                            keywords = tr("Invalid keywords");

                        if (!hub.isEmpty())
                            toshow = Qt::escape(keywords) + " (" + Qt::escape(hub) + ")";
                        else
                            toshow = Qt::escape(keywords);
                    }
                    else {
                        QString name, tth;
                        int64_t size;

                        WulforUtil::splitMagnet(link, size, tth, name);
                        toshow = QString("%1 (%2)").arg(name).arg(WulforUtil::formatBytes(size));
                    }
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

        EmoticonMap::iterator it = emoticons.begin();
        EmoticonMap::iterator end_it = emoticons.end();
        bool smile_found = false;

        for (; it != end_it; ++it){//Let's try to parse smiles
            const QString &emo_text = it.key();
            EmoticonObject *obj = it.value();

            if (input.startsWith(emo_text) && obj){
                if (force_emot || input == emo_text){
                     QString img = QString("<img alt=\"%1\" title=\"%1\" align=\"center\" source=\"%2/emoticon%3\" />")
                                  .arg(emo_text)
                                  .arg(emo_theme)
                                  .arg(obj->id);

                    output += img;
                    input.remove(0, emo_text.length());

                    smile_found = true;

                    break;
                }
                else if (output.endsWith(' ') || output.endsWith('\t') || output.isEmpty()){
                    int emo_text_len = emo_text.length();
                    int input_length = input.length();

                    bool nextCharisSpace = false;

                    if (emo_text_len == input_length)
                        nextCharisSpace = true;
                    else if (input_length > emo_text_len){
                        char c = input.at(emo_text_len).toAscii();

                        nextCharisSpace = (c == ' ' || c == '\t');
                    }

                    if (!nextCharisSpace)
                        continue;

                    QString img = QString("<img alt=\"%1\" title=\"%1\" align=\"center\" source=\"%2/emoticon%3\" />")
                                  .arg(emo_text)
                                  .arg(emo_theme)
                                  .arg(obj->id);

                    output += img;
                    input.remove(0, emo_text_len);

                    smile_found = true;

                    break;
                }
            }
        }

        if(smile_found)
            continue;

        if (WBGET("hubframe/use-bb-code", false)){
            if      (parseBasicBBCode("b", "b", input, output))
                continue;
            else if (parseBasicBBCode("u", "u", input, output))
                continue;
            else if (parseBasicBBCode("i", "i", input, output))
                continue;
            else if (parseBasicBBCode("s", "s", input, output))
                continue;

            if (input.startsWith("[color=") && input.indexOf("[/color]") > 8){
                QRegExp exp("\\[color=(\\w+|#.{6,6})\\]((.*))\\[/color\\].*");
                QString chunk = input.left(input.indexOf("[/color]")+8);

                if (exp.exactMatch(chunk)){
                    if (exp.numCaptures() == 3){
                        output += "<font color=\"" + exp.cap(1) + "\">" + parseForLinks(exp.cap(2), false) + "</font>";

                        input.remove(0, chunk.length());
                    }
                }
            }
            else if (input.startsWith(("[url")) && input.indexOf("[/url]") > 0){
                QRegExp exp("\\[url=*((.+[^\\]\\[]))*\\]((.+))\\[/url\\]");
                QString chunk = input.left(input.indexOf("[/url]")+6);

                if (exp.exactMatch(chunk) && exp.numCaptures() == 4){
                    QString link = exp.cap(2);
                    QString title = exp.cap(3);

                    link = link.isEmpty()? title : link;

                    if (link.startsWith("="))
                        link.remove(0, 1);

                    if (!title.isEmpty()){
                        output += "<a href=\"" + link + "\" title=\"" + Qt::escape(title) + "\">" + Qt::escape(title) + "</a>";

                        input.remove(0, chunk.length());
                    }
                }
            }
            else if (input.startsWith(("[code]")) && input.indexOf("[/code]") > 0){
                input.remove(0, 6);
                int c_len = input.indexOf("[/code]");

                QString chunk = input.left(c_len);

                output += "<table border=1 width=100%><tr><td align=\"left\">Code:</td></tr><tr><td align=\"left\"><pre style=\"white-space: pre;\"><tt>" + chunk + "</tt></pre></td></tr></table>";
                input.remove(0, c_len+7);

                continue;
            }
        }

        if (input.startsWith("<")){
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
        else if (input.startsWith("[magnet=\"") && input.indexOf("[/magnet]") > 9){//9 - length of [magnet="
            QString chunk = input.left(input.indexOf("[/magnet]"));

            do{
                if (chunk.isEmpty())
                    break;

                chunk.remove(0, 9);

                if (chunk.isEmpty() || chunk.indexOf("\"") <= 0)
                    break;

                QString magnet = chunk.left(chunk.indexOf("\""));

                magnet = magnet.trimmed();

                if (magnet.isEmpty())
                    break;

                QString name, tth;
                int64_t size;

                WulforUtil::splitMagnet(magnet, size, tth, name);

                chunk.remove(0, magnet.length());

                if (chunk.indexOf("]") < 1)
                    break;

                chunk.remove(0, chunk.indexOf("]") + 1);

                if (chunk.isEmpty())
                    break;

                QString toshow = tr("%1 (%2)").arg(chunk).arg(WulforUtil::formatBytes(size));
                QString html_link = "<a href=\"" + magnet + "\" title=\"" + toshow + "\" style=\"cursor: hand\">" + toshow + "</a>";

                output += html_link;
                input.remove(0, input.indexOf("[/magnet]")+1+9);
            }
            while (0);
        }

        if (input.isEmpty())
            break;

        output += input.at(0);

        input.remove(0, 1);
    }

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

        const TTHValue *tth = HashManager::getInstance()->getFileTTHif(_tq(fi.absoluteFilePath()));
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
        d_ptr(new HubFramePrivate())
{
    Q_D(HubFrame);
    
    d->total_shared = 0;
    d->arenaMenu = NULL;
    d->codec = NULL;
    d->chatDisabled = false;
    d->hasMessages = false;
    d->hasHighlightMessages = false;
    d->client = NULL;
    
    setupUi(this);

    if (!Menu::getInstance())
        Menu::newInstance();

    Menu::counter++;

    d->client = ClientManager::getInstance()->getClient(hub.toStdString());
    d->client->addListener(this);

    QString enc = WulforUtil::getInstance()->qtEnc2DcEnc(encoding);

    if (enc.isEmpty())
        enc = WulforUtil::getInstance()->qtEnc2DcEnc(WSGET(WS_DEFAULT_LOCALE));

    if (enc.indexOf(" ") > 0){
        enc = enc.left(enc.indexOf(" "));
        enc.replace(" ", "");
    }

    d->client->setEncoding(enc.toStdString());

    d->codec = WulforUtil::getInstance()->codecForEncoding(encoding);

    init();

    FavoriteHubEntry* entry = FavoriteManager::getInstance()->getFavoriteHubEntry(_tq(hub));

    if (entry && entry->getDisableChat())
        disableChat();

    d->client->connect();

    setAttribute(Qt::WA_DeleteOnClose);

    d->out_messages_index = 0;
    d->out_messages_unsent = false;

    FavoriteManager::getInstance()->addListener(this);
}


HubFrame::~HubFrame(){
    Q_D(HubFrame);
    
    Menu::counter--;

    if (!Menu::counter)
        Menu::deleteInstance();

    treeView_USERS->setModel(NULL);

    delete d->model;
    delete d->proxy;

    delete d->updater;
    
    delete d;
}

bool HubFrame::eventFilter(QObject *obj, QEvent *e){
    Q_D(HubFrame);
    
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

        if (qobject_cast<LineEdit*>(obj) == lineEdit_FIND && k_e->key() == Qt::Key_Escape){
            lineEdit_FIND->clear();

            requestFilter();

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
                    cid = d->model->CIDforNick(nick, _q(d->client->getHubUrl()));
                }
                if ((positionCursor < r) && (positionCursor > l))
                    cursoratnick = true;
            }
            else if (isUserList){
                QModelIndex index = treeView_USERS->indexAt(treeView_USERS->viewport()->mapFromGlobal(QCursor::pos()));

                if (treeView_USERS->model() == d->proxy)
                    index = d->proxy->mapToSource(index);

                if (index.isValid()){
                    UserListItem *i = reinterpret_cast<UserListItem*>(index.internalPointer());

                    nick = i->getNick();
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
                    cid = d->model->CIDforNick(nick, _q(d->client->getHubUrl()));
                    //qDebug() << cid;
                    }
                if (((positionCursor < r) && (positionCursor > l))/* || positionCursor > l1*/)
                    cursoratnick = true;
            }
            else if (isUserList){
                QModelIndex index = treeView_USERS->indexAt(treeView_USERS->viewport()->mapFromGlobal(QCursor::pos()));

                if (treeView_USERS->model() == d->proxy)
                    index = d->proxy->mapToSource(index);

                if (index.isValid()){
                    UserListItem *i = reinterpret_cast<UserListItem*>(index.internalPointer());

                    nick = i->getNick();
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
    Q_D(HubFrame);
    
    blockSignals(true);
    
    QObject::disconnect(this, NULL, this, NULL);

    FavoriteManager::getInstance()->removeListener(this);

    HubManager::getInstance()->unregisterHubUrl(_q(d->client->getHubUrl()));

    d->client->removeListener(this);
    d->client->disconnect(true);
    ClientManager::getInstance()->putClient(d->client);

    d->updater->stop();

    save();

    PMMap::const_iterator it = d->pm.constBegin();

    for (; it != d->pm.constEnd(); ++it){
        PMWindow *w = const_cast<PMWindow*>(it.value());

        disconnect(w, SIGNAL(privateMessageClosed(QString)), this, SLOT(slotPMClosed(QString)));

        w->close();
    }

    d->pm.clear();

    foreach (ShellCommandRunner *r, d->shell_list){
        r->cancel();
        r->exit(0);

        r->wait(100);

        if (r->isRunning())
            r->terminate();

        delete r;
    }

    if (isVisible())
        HubManager::getInstance()->setActiveHub(NULL);

    setAttribute(Qt::WA_DeleteOnClose);

    e->accept();
    
    blockSignals(false);
    {
        emit closeRequest();
    }
    blockSignals(true);
}

void HubFrame::showEvent(QShowEvent *e){
    Q_D(HubFrame);
    
    e->accept();

    d->drawLine = false;

    HubManager::getInstance()->setActiveHub(this);

    d->hasMessages = false;
    d->hasHighlightMessages = false;
    
    MainWindow::getInstance()->redrawToolPanel();
}

void HubFrame::hideEvent(QHideEvent *e){
    Q_D(HubFrame);
    
    e->accept();

    d->drawLine = true;

    if (!isVisible())
        HubManager::getInstance()->setActiveHub(NULL);
}

void HubFrame::init(){
    Q_D(HubFrame);
    
    d->updater = new QTimer();
    d->updater->setInterval(5000);
    d->updater->setSingleShot(false);

    d->model = new UserListModel(this);
    d->proxy = NULL;

    treeView_USERS->setModel(d->model);
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

    for (int i = 0; i < d->model->columnCount(); i++)
        comboBox_COLUMNS->addItem(d->model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());

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
    connect(d->updater, SIGNAL(timeout()), this, SLOT(slotUsersUpdated()));
    connect(textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));
    connect(toolButton_BACK, SIGNAL(clicked()), this, SLOT(slotFindBackward()));
    connect(toolButton_FORWARD, SIGNAL(clicked()), this, SLOT(slotFindForward()));
    connect(toolButton_HIDE, SIGNAL(clicked()), this, SLOT(slotHideFindFrame()));
    connect(lineEdit_FIND, SIGNAL(textEdited(QString)), this, SLOT(slotFindTextEdited(QString)));
    connect(lineEdit_FILTER, SIGNAL(textChanged(QString)), this, SLOT(slotFilterTextChanged()));
    connect(comboBox_COLUMNS, SIGNAL(activated(int)), this, SLOT(slotFilterTextChanged()));
    connect(toolButton_SMILE, SIGNAL(clicked()), this, SLOT(slotSmile()));
    connect(toolButton_SMILE, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotSmileContextMenu()));
    connect(toolButton_ALL, SIGNAL(clicked()), this, SLOT(slotFindAll()));
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

    d->updater->start();

    d->completer = new QCompleter(this);
#if QT_VERSION >= 0x040600
    d->completer->setMaxVisibleItems(10); // This property was introduced in Qt 4.6.
#endif
    plainTextEdit_INPUT->setCompleter(d->completer, d->model);

    slotSettingsChanged(WS_APP_EMOTICON_THEME, WSGET(WS_APP_EMOTICON_THEME));//toggle emoticon button
}

void HubFrame::initMenu(){
    Q_D(HubFrame);
    WulforUtil *WU = WulforUtil::getInstance();

    delete d->arenaMenu;

    d->arenaMenu = new QMenu(tr("Hub menu"), this);

    QAction *reconnect = new QAction(WU->getPixmap(WulforUtil::eiRECONNECT), tr("Reconnect"), d->arenaMenu);
    QAction *show_wnd  = new QAction(WU->getPixmap(WulforUtil::eiCHAT), tr("Show widget"), d->arenaMenu);
    QAction *addToFav  = new QAction(WU->getPixmap(WulforUtil::eiFAVSERVER), tr("Add to Favorites"), d->arenaMenu);
    QMenu   *copyInfo  = new QMenu(tr("Copy"), d->arenaMenu);
    QAction *copyIP    = copyInfo->addAction(tr("Hub IP"));
    QAction *copyURL   = copyInfo->addAction(tr("Hub URL"));
    QAction *copyTitle = copyInfo->addAction(tr("Hub Title"));

    QAction *sep       = new QAction(d->arenaMenu);
    sep->setSeparator(true);
    QAction *close_wnd = new QAction(WU->getPixmap(WulforUtil::eiEXIT), tr("Close"), d->arenaMenu);

    d->arenaMenu->addActions(QList<QAction*>() << reconnect
                                            << show_wnd
                                            << addToFav
                         );

    d->arenaMenu->addMenu(copyInfo);

    if (d->client && d->client->isConnected()){
        QMenu *u_c = WulforUtil::getInstance()->buildUserCmdMenu(QList<QString>() << _q(d->client->getHubUrl()), UserCommand::CONTEXT_HUB, d->arenaMenu);

        if (u_c){
            if (u_c->actions().size() > 0){
                u_c->setTitle(tr("Hub Menu"));

                d->arenaMenu->addMenu(u_c);

                connect(u_c, SIGNAL(triggered(QAction*)), this, SLOT(slotHubMenu(QAction*)));
            }
        }
    }

    d->arenaMenu->addActions(QList<QAction*>() << sep << close_wnd);

    connect(reconnect,  SIGNAL(triggered()), this, SLOT(slotReconnect()));
    connect(show_wnd,   SIGNAL(triggered()), this, SLOT(slotShowWnd()));
    connect(addToFav,   SIGNAL(triggered()), this, SLOT(addAsFavorite()));
    connect(copyIP,     SIGNAL(triggered()), this, SLOT(slotCopyHubIP()));
    connect(copyTitle,  SIGNAL(triggered()), this, SLOT(slotCopyHubTitle()));
    connect(copyURL,    SIGNAL(triggered()), this, SLOT(slotCopyHubURL()));
    connect(close_wnd,  SIGNAL(triggered()), this, SLOT(slotClose()));
}


void HubFrame::save(){
    Q_D(HubFrame);
    
    WSSET(WS_CHAT_USERLIST_STATE, treeView_USERS->header()->saveState().toBase64());
    WISET(WI_CHAT_WIDTH, textEdit_CHAT->width());
    WISET(WI_CHAT_USERLIST_WIDTH, treeView_USERS->width());
    WISET(WI_CHAT_SORT_COLUMN, d->model->getSortColumn());
    WISET(WI_CHAT_SORT_ORDER, WulforUtil::getInstance()->sortOrderToInt(d->model->getSortOrder()));
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
    Q_D(HubFrame);
    QString ret = tr("Not connected");

    if (d->client && d->client->isConnected()){
        ret  = QString("%1 - %2 [%3]").arg(QString(d->client->getHubName().c_str()))
                                      .arg(QString(d->client->getHubDescription().c_str()))
                                      .arg(QString(d->client->getIp().c_str()));
        QString prefix = QString("[+%1] ").arg(d->client->isSecure()? ("S") : (d->client->isTrusted()? ("T"): ("")));

        ret.prepend(prefix);
    }
    else if (d->client){
        ret = QString("[-] %1").arg(d->client->getHubUrl().c_str());
    }

    return ret;
}

QString HubFrame::getCIDforNick(QString nick){
    Q_D(HubFrame);
    
    return d->model->CIDforNick(nick, _q(d->client->getHubUrl()));
}

QString HubFrame::getArenaShortTitle(){
    Q_D(HubFrame);
    QString ret = tr("Not connected");

    if (d->client && d->client->isConnected()){
        ret = QString("[+] %1").arg(QString(d->client->getHubName().c_str()));
    }
    else if (d->client){
        ret = QString("[-] %1").arg(d->client->getHubUrl().c_str());
    }

    return ret;
}

QMenu *HubFrame::getMenu(){
    initMenu();

    Q_D(HubFrame);
    
    return d->arenaMenu;
}

const QPixmap &HubFrame::getPixmap(){
    Q_D(HubFrame);
    
    if (d->hasHighlightMessages)
        return WICON(WulforUtil::eiMESSAGE);
    else if (d->hasMessages)
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
    Q_D(HubFrame);
    
    if (!d->chatDisabled){
        addStatus(tr("Chat disabled."));

        d->chatDisabled = true;
    }
    else{
        d->chatDisabled = false;

        addStatus(tr("Chat enabled."));
    }

    plainTextEdit_INPUT->setEnabled(!d->chatDisabled);
    frame_INPUT->setVisible(!d->chatDisabled);
}

void HubFrame::getStatistic(quint64 &users, quint64 &share) const{
    Q_D(const HubFrame);
    
    if (d->model)
        users = d->model->rowCount();

    share = d->total_shared;
}

bool HubFrame::isConnected() const {
    Q_D(const HubFrame);
    
    return (d->client? d->client->isConnected() : false);
}

QString HubFrame::getUserInfo(UserListItem *item){
    Q_D(HubFrame);
    QString ttip = "";

    ttip += d->model->headerData(COLUMN_NICK, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->getNick() + "\n";
    ttip += d->model->headerData(COLUMN_COMMENT, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->getComment() + "\n";
    ttip += d->model->headerData(COLUMN_EMAIL, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->getEmail() + "\n";
    ttip += d->model->headerData(COLUMN_IP, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->getIP() + "\n";
    ttip += d->model->headerData(COLUMN_SHARE, Qt::Horizontal, Qt::DisplayRole).toString() + ": " +
            WulforUtil::formatBytes(item->getShare()) + "\n";
    ttip += d->model->headerData(COLUMN_TAG, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->getTag() + "\n";
    ttip += d->model->headerData(COLUMN_CONN, Qt::Horizontal, Qt::DisplayRole).toString() + ": " + item->getConnection() + "\n";

    if (item->isOP())
        ttip += tr("Hub role: Operator");
    else
        ttip += tr("Hub role: User");

    if (item->isFav())
        ttip += tr("\nFavorite user");

    return ttip;
}

void HubFrame::sendMsg(const QString &msg){
    sendChat(msg, false, false);
}

void HubFrame::sendChat(QString msg, bool thirdPerson, bool stripNewLines){
    Q_D(HubFrame);
    
    if (!d->client || !d->client->isConnected() || msg.isEmpty() || msg.isNull())
        return;

    if (stripNewLines)
        msg.replace("\n", "");

    if (msg.trimmed().isEmpty())
        return;

    if (msg.endsWith("\n"))
        msg = msg.left(msg.lastIndexOf("\n"));

    bool script_ret = false;
#ifdef LUA_SCRIPT
    script_ret = ((ClientScriptInstance *) (this->client))->onHubFrameEnter(this->client, msg.toStdString());
#endif
    if (!script_ret && !parseForCmd(msg, this))
        d->client->hubMessage(msg.toStdString(), thirdPerson);

    //qDebug() << "cmd: " << cmd <<" sript_ret: " << script_ret;
    if (!thirdPerson){
        if (d->out_messages_unsent){
            d->out_messages.removeLast();
            d->out_messages_unsent = false;
        }

        d->out_messages << msg;

        if (d->out_messages.size() > WIGET(WI_OUT_IN_HIST))
            d->out_messages.removeFirst();

        d->out_messages_index = d->out_messages.size()-1;
    }
}

bool HubFrame::parseForCmd(QString line, QWidget *wg){
    HubFrame *fr = qobject_cast<HubFrame *>(wg);
    PMWindow *pm = qobject_cast<PMWindow *>(wg);
    Q_D(HubFrame);

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
    else if (cmd == "/kword" && !emptyParam){
        if (list.size() < 2 || list.size() > 3)
            return false;

        enum { List=0, Add, Remove };

        int kw_action = List;

        if (list.at(1) == QString("add"))
            kw_action = Add;
        else if (list.at(1) == QString("purge"))
            kw_action = Remove;
        else if (list.at(1) == QString("list"))
            kw_action = List;
        else {
            if (fr == this)
                addStatus(tr("Invalid command syntax."));
            else if (pm)
                pm->addStatus(tr("Invalid command syntax."));

            return false;
        }

        if (kw_action != List && list.size() != 3){
            if (fr == this)
                addStatus(tr("Invalid command syntax."));
            else if (pm)
                pm->addStatus(tr("Invalid command syntax."));

            return false;
        }

        QStringList kwords = WVGET("hubframe/chat-keywords", QStringList()).toStringList();

        switch (kw_action){
        case List:
            {
                QString str = tr("List of keywords:\n");

                foreach (const QString s, kwords)
                    str += "\t" + s + "\n";

                if (fr == this)
                    addStatus(str);
                else if (pm)
                    pm->addStatus(str);;

                break;
            }
        case Remove:
            {
                QString kword = list.last();

                if (kwords.contains(kword))
                    kwords.removeOne(kword);

                break;
            }
        case Add:
            {
                QString kword = list.last();

                if (!kwords.contains(kword))
                    kwords.push_back(kword);

                break;
            }
        default:
            break;
        }

        WVSET("hubframe/chat-keywords", kwords);
    }
    else if (cmd == "/ratio"){
        double ratio;
        double up   = static_cast<double>(SETTING(TOTAL_UPLOAD));
        double down = static_cast<double>(SETTING(TOTAL_DOWNLOAD));


        if (down > 0)
            ratio = up / down;
        else
            ratio = 0;

        QString line = tr("ratio: %1 (uploads: %2, downloads: %3)")
                          .arg(QString().setNum(ratio, 'f', 3))
                          .arg(WulforUtil::formatBytes(up))
                          .arg(WulforUtil::formatBytes(down));

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
        browseUserFiles(d->model->CIDforNick(param, _q(d->client->getHubUrl())), false);
    }
    else if (cmd == "/grant" && !emptyParam){
        grantSlot(d->model->CIDforNick(param, _q(d->client->getHubUrl())));
    }
    else if (cmd == "/magnet" && !emptyParam){
        WISET(WI_DEF_MAGNET_ACTION, param.toInt());
    }
    else if (cmd == "/info" && !emptyParam){
        UserListItem *item = d->model->itemForNick(param, _q(d->client->getHubUrl()));

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
        addPM(d->model->CIDforNick(param, _q(d->client->getHubUrl())), "", false);
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
        out += tr("/kword add <keyword> - add user-defined keyword which will be highlighted in the chat\n");
        out += tr("/kword purge <keyword> - remove user-defined keyword\n");
        out += tr("/kword list - full list of keywords which will be highlighted in the chat\n");
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

        d->shell_list.append(sh);

        sh->start();
    }
    else if (cmd == "/ws" && !emptyParam){
        line = line.remove(0, 4);
        line.replace("\n", "");

        WSCMD(line);
    }
    else if (cmd == "/dcpps" && !emptyParam) {
        line = line.remove(0,7);
        QString out = _q(SettingsManager::getInstance()->parseCoreCmd (_tq(line)));
        if (fr == this)
            addStatus(out);
        else if (pm)
            pm->addStatus(out);
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

QString HubFrame::getHubUrl() {
    Q_D(HubFrame);
    
    if (d->client)
        return _q(d->client->getHubUrl());
    
    return "";
}

QString HubFrame::getHubName() {
    Q_D(HubFrame);
    
    if (d->client)
        return _q(d->client->getHubName());
    
    return "";
}

QString HubFrame::getMyNick() {
    Q_D(HubFrame);
    
    if (d->client)
        return _q(d->client->getMyNick());
    
    return "";
}

void HubFrame::addStatus(QString msg){
    Q_D(HubFrame);
    
    if (d->chatDisabled)
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

    pure_msg  = LinkParser::parseForLinks(msg.left(WIGET(WI_CHAT_STATUS_MSG_MAX_LEN)), false);
    short_msg = LinkParser::parseForLinks(short_msg, false);
    msg       = LinkParser::parseForLinks(msg, true);

    pure_msg        = "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\">" + pure_msg + "</font>";
    short_msg       = "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\">" + short_msg + "</font>";
    msg             = "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\">" + msg + "</font>";
    QString time    = "";

    if (!WSGET(WS_CHAT_TIMESTAMP).isEmpty())
        time = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP)) + "]</font>";

    status   = time + "<font color=\"" + WSGET(WS_CHAT_STAT_COLOR) + "\"><b>" + nick + "</b> </font>";

    QRegExp rot_msg = QRegExp("is(\\s+)kicking(\\s+)(\\S+)*(\\s+)because:");

    bool isRotating = (msg.indexOf("is kicking because:") >= 0) || (rot_msg.indexIn(msg) >= 0);

    if (!(isRotating && WBGET(WB_CHAT_ROTATING_MSGS)))
        addOutput(status + msg);

    label_LAST_STATUS->setText(status + short_msg);

    status += pure_msg;
    WulforUtil::getInstance()->textToHtml(status, false);

    d->status_msg_history.push_back(status);

    if (WIGET(WI_CHAT_STATUS_HISTORY_SZ) > 0){
        while (d->status_msg_history.size() > WIGET(WI_CHAT_STATUS_HISTORY_SZ))
            d->status_msg_history.removeFirst();
    }
    else
        d->status_msg_history.clear();

    label_LAST_STATUS->setToolTip(d->status_msg_history.join("<br/>"));
}

void HubFrame::addOutput(QString msg){
    msg.replace("\r", "");
    msg = "<pre>" + msg + "</pre>";
    textEdit_CHAT->append(msg);
}

void HubFrame::addPM(QString cid, QString output, bool keepfocus){
    Q_D(HubFrame);
    bool redirectToMainChat = WBGET("hubframe/redirect-pm-to-main-chat", true);

    if (!d->pm.contains(cid)){
        PMWindow *p = ArenaWidgetFactory().create<PMWindow, QString, QString>(cid, _q(d->client->getHubUrl()));
        p->textEdit_CHAT->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(p, SIGNAL(privateMessageClosed(QString)), this, SLOT(slotPMClosed(QString)));
        connect(p, SIGNAL(inputTextChanged()), this, SLOT(slotInputTextChanged()));
        connect(p, SIGNAL(inputTextMenu()), this, SLOT(slotInputContextMenu()));
        connect(p->textEdit_CHAT, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotChatMenu(QPoint)));

        p->setCompleter(d->completer, d->model);
        p->addOutput(output);
        p->setAttribute(Qt::WA_DeleteOnClose);

        if (!(keepfocus && WBGET(WB_CHAT_KEEPFOCUS))){
            ArenaWidgetManager::getInstance()->activate(p);

            p->requestFocus();
        }

        d->pm.insert(cid, p);

        if (!p->isVisible() && redirectToMainChat)
            addOutput("<b>PM: </b>" + output);
    }
    else{
        PMMap::iterator it = d->pm.find(cid);

        if (output.indexOf(_q(d->client->getMyNick())) >= 0)
            it.value()->setHasHighlightMessages(true);

        it.value()->addOutput(output);

        if (!(keepfocus && WBGET(WB_CHAT_KEEPFOCUS))){
            ArenaWidgetManager::getInstance()->activate(it.value());

            it.value()->requestFocus();
        }

        if (! it.value()->isVisible() && redirectToMainChat)
            addOutput("<b>PM: </b>" + output);
    }
}

bool HubFrame::isOP(const QString& nick) {
    Q_D(HubFrame);
    
    UserListItem *item = d->model->itemForNick(nick, _q(d->client->getHubUrl()));
    
    return (item? item->isOP() : false);
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
    Q_D(HubFrame);
    
    static WulforUtil *WU = WulforUtil::getInstance();
    static WulforSettings *WS = WulforSettings::getInstance();
    static bool showFavJoinsOnly = WS->getBool(WB_CHAT_SHOW_JOINS_FAV);

    if (!d->model)
        return;

    UserListItem *item = d->model->itemForPtr(user);

    QString cid = map["CID"].toString();
    QString nick = map["NICK"].toString();

    if (item){
        bool isOp = map["ISOP"].toBool();

        d->total_shared -= item->getShare();
 
        item->updateIdentity();

        d->model->needResort();
    }
    else{
        if (join && WS->getBool(WB_CHAT_SHOW_JOINS)){
            do {
                if (showFavJoinsOnly && !FavoriteManager::getInstance()->isFavoriteUser(user))
                    break;

                addStatus(nick + tr(" joins the chat"));
            } while (0);
        }

        d->model->addUser(nick, cid, user);

        if (FavoriteManager::getInstance()->isFavoriteUser(user))
            Notification::getInstance()->showMessage(Notification::FAVORITE, tr("Favorites"), tr("%1 is now online").arg(nick));

        if (d->pm.contains(nick)){
            PMWindow *wnd = d->pm[nick];

            wnd->cid = cid;
            wnd->plainTextEdit_INPUT->setEnabled(true);
            wnd->hubUrl = _q(d->client->getHubUrl());

            d->pm.insert(cid, wnd);

            d->pm.remove(nick);

            pmUserEvent(cid, tr("User online."));
        }
    }

    d->total_shared += map["SHARE"].toULongLong();
}

void HubFrame::userRemoved(const dcpp::UserPtr &user, qlonglong share){
    Q_D(HubFrame);
    d->total_shared -= share;

    QString cid = _q(user->getCID().toBase32());
    QString nick = "";
    UserListItem *item = d->model->itemForPtr(user);

    if (item)
        nick = item->getNick();

    if (d->pm.contains(cid)){
        pmUserOffline(cid);

        PMWindow *pmw = d->pm[cid];

        d->pm.insert(nick, pmw);

        pmw->cid = nick;
        pmw->plainTextEdit_INPUT->setEnabled(false);//we need interface function

        d->pm.remove(cid);
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
        Notification::getInstance()->showMessage(Notification::FAVORITE, tr("Favorites"), tr("%1 is now offline").arg(nick));

    d->model->removeUser(user);
}

void HubFrame::browseUserFiles(const QString& id, bool match){
    string message;
    string cid = id.toStdString();

    if (!cid.empty()){
        try{
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

            if (user){
                Q_D(HubFrame);
                
                if (user == ClientManager::getInstance()->getMe())
                    MainWindow::getInstance()->browseOwnFiles();
                else if (match)
                    QueueManager::getInstance()->addList(HintedUser(user, d->client->getHubUrl()), QueueItem::FLAG_MATCH_QUEUE, "");
                else
                    QueueManager::getInstance()->addList(HintedUser(user, d->client->getHubUrl()), QueueItem::FLAG_CLIENT_VIEW, "");
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
    Q_D(HubFrame);
    
    QString message = tr("User not found");

    if (!id.isEmpty()){
        UserPtr user = ClientManager::getInstance()->findUser(CID(id.toStdString()));

        if (user){
            UploadManager::getInstance()->reserveSlot(HintedUser(user, d->client->getHubUrl()));
            message = tr("Slot granted to ") + WulforUtil::getInstance()->getNicks(user->getCID(), _q(d->client->getHubUrl()));
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
        Q_D(HubFrame);
        
        UserListItem *item = NULL;
        
        if (d->model)
            item = d->model->itemForPtr(user);

        bool bFav = FavoriteManager::getInstance()->isFavoriteUser(user);

        if (item) {
            QModelIndex ixb = d->model->index(item->row(), COLUMN_NICK);
            QModelIndex ixe = d->model->index(item->row(), COLUMN_EMAIL);

            d->model->repaintData(ixb, ixe);
        }

        QString message = WulforUtil::getInstance()->getNicks(id, _q(d->client->getHubUrl())) +
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
    Q_D(HubFrame);
    FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(d->client->getHubUrl());

    if (!existingHub){
        FavoriteHubEntry aEntry;

        aEntry.setServer(d->client->getHubUrl());
        aEntry.setName(d->client->getHubName());
        aEntry.setDescription(d->client->getHubDescription());
        aEntry.setConnect(FALSE);
        aEntry.setNick(d->client->getMyNick());
        aEntry.setEncoding(d->client->getEncoding());

        FavoriteManager::getInstance()->addFavorite(aEntry);
        FavoriteManager::getInstance()->save();

        addStatus(tr("Favorite hub added."));
    }
    else{
        addStatus(tr("Favorite hub already exists."));
    }
}

void HubFrame::disablePrivateMessages(bool disable) {
    if (disable)
        disconnect(this, SIGNAL(corePrivateMsg(VarMap)), this, SLOT(newPm(VarMap)));
    else
        connect(this, SIGNAL(corePrivateMsg(VarMap)), this, SLOT(newPm(VarMap)), Qt::QueuedConnection);
}


void HubFrame::newMsg(const VarMap &map){
    Q_D(HubFrame);
    QString output = "";

    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";;
    QString color = map["CLR"].toString();
    QString msg_color = WS_CHAT_MSG_COLOR;
    QString trigger = "";

    const QStringList &kwords = WVGET("hubframe/chat-keywords", QStringList()).toStringList();

    foreach (const QString &word, kwords){
        if (message.contains(word, Qt::CaseInsensitive)){
            msg_color = WS_CHAT_SAY_NICK;
            trigger = word;
            
            break;
        }
    }

    if (message.indexOf(_q(d->client->getMyNick())) >= 0){
        msg_color = WS_CHAT_SAY_NICK;
        trigger = _q(d->client->getMyNick());
            
        Notification::getInstance()->showMessage(Notification::NICKSAY, getArenaTitle().left(20), nick + ": " + message);
    }
    
    if (msg_color == WS_CHAT_SAY_NICK){
        VarMap tmap = map;
        tmap["TRIGGER"] = trigger;
        
        emit highlighted(tmap);
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

    if (!isVisible()){
        if (msg_color == WS_CHAT_SAY_NICK)
            d->hasHighlightMessages = true;

        d->hasMessages = true;

        MainWindow::getInstance()->redrawToolPanel();
    }

    if (d->drawLine && WBGET("hubframe/unreaden-draw-line", true)){
        QString hr = "<hr />";

        QTextDocument *chatDoc = textEdit_CHAT->document();

        int scrollbarValue = textEdit_CHAT->verticalScrollBar()->value();

        for (QTextBlock it = chatDoc->begin(); it != chatDoc->end(); it = it.next()){
            if (it.userState() == 1){
                if (it.text().isEmpty()){ // additional check that it is not message
                    QTextCursor c(it);
                    c.select(QTextCursor::BlockUnderCursor);
                    c.deleteChar(); // delete string with horizontal line

                    if (scrollbarValue > textEdit_CHAT->verticalScrollBar()->maximum())
                        scrollbarValue = textEdit_CHAT->verticalScrollBar()->maximum();

                    textEdit_CHAT->verticalScrollBar()->setValue(scrollbarValue);

                    break;
                }
            }
        }

        d->drawLine = false;

        chatDoc->lastBlock().setUserState(0); // add label for the last of the old messages

        output.prepend(hr);

        addOutput(output);

        for (QTextBlock it = chatDoc->begin(); it != chatDoc->end(); it = it.next()){
            if (it.userState() == 0){
                it.setUserState(-1); // delete label for the last of the old messages

                if (it.blockNumber() < chatDoc->blockCount()-3){
                    it = it.next().next();
                    it.setUserState(1); // add label for string with horizontal line

                    it = it.previous();
                    if (it.text().isEmpty()){ // additional check that it is not message
                        QTextCursor c(it);
                        c.select(QTextCursor::BlockUnderCursor);
                        c.deleteChar(); // delete empty string above horizontal line
                    }
                }

                break;
            }
        }

        return;
    }

    addOutput(output);
}

void HubFrame::newPm(const VarMap &map){
    Q_D(HubFrame);
    QString nick = map["NICK"].toString();
    QString message = map["MSG"].toString();
    QString time    = "<font color=\"" + WSGET(WS_CHAT_TIME_COLOR)+ "\">[" + map["TIME"].toString() + "]</font>";
    QString color = map["CLR"].toString();
    QString full_message = "";

    if (nick != _q(d->client->getMyNick())){
        bool show_msg = false;

        if (!d->pm.contains(map["CID"].toString()))
            show_msg = true;
        else
            show_msg = (!d->pm[map["CID"].toString()]->isVisible() || WBGET("notification/play-sound-with-active-pm", true));

        if (show_msg)
            Notification::getInstance()->showMessage(Notification::PM, nick, message);
    }

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
    Q_D(HubFrame);
    createPMWindow(CID(_tq(d->model->CIDforNick(nick, _q(d->client->getHubUrl())))));
}

void HubFrame::createPMWindow(const dcpp::CID &cid){
    addPM(_q(cid.toBase32()), "");
}

bool HubFrame::hasCID(const dcpp::CID &cid, const QString &nick){
    Q_D(HubFrame);
    return (d->model->CIDforNick(nick, _q(d->client->getHubUrl())) == _q(cid.toBase32()));
}

void HubFrame::clearUsers(){
    Q_D(HubFrame);
    
    if (d->model){
        d->model->blockSignals(true);
        d->model->clear();
       d-> model->blockSignals(false);
        treeView_USERS->setModel(d->model);
    }

    d->total_shared = 0;

    treeView_USERS->repaint();

    slotUsersUpdated();

    d->model->repaint();
}

void HubFrame::pmUserOffline(QString cid){
    pmUserEvent(cid, tr("User offline."));
}

void HubFrame::pmUserEvent(QString cid, QString e){
    Q_D(HubFrame);
    
    if (!d->pm.contains(cid))
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

    d->pm[cid]->addOutput(output);
}

void HubFrame::getPassword(){
    Q_D(HubFrame);
    
    MainWindow *MW = MainWindow::getInstance();

    if (!MW->isVisible() && !(d->client->getPassword().size() > 0)){
        MW->show();
        MW->raise();

    }

    if(d->client && d->client->getPassword().size() > 0) {
        d->client->password(d->client->getPassword());
        addStatus(tr("Stored password sent..."));
    }
    else if (d->client && d->client->isConnected()){
        QString pass = QInputDialog::getText(this, _q(d->client->getHubUrl()), tr("Password"), QLineEdit::Password);

        if (!pass.isEmpty()){
            d->client->setPassword(pass.toStdString());
            d->client->password(pass.toStdString());
        }
        else
            d->client->disconnect(true);
    }
}

void HubFrame::follow(QString redirect){
    if(!redirect.isEmpty()) {
        if(ClientManager::getInstance()->isConnected(_tq(redirect))) {
            addStatus(tr("Redirect request received to a hub that's already connected"));
            return;
        }

        string url = _tq(redirect);
        
        Q_D(HubFrame);
        // the client is dead, long live the client!
        d->client->removeListener(this);
        HubManager::getInstance()->unregisterHubUrl(_q(d->client->getHubUrl()));
        ClientManager::getInstance()->putClient(d->client);
        clearUsers();
        d->client = ClientManager::getInstance()->getClient(url);

        d->client->addListener(this);
        d->client->connect();
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
    Q_D(HubFrame);
    
    if (treeView_USERS->model() == d->proxy){
        label_USERSTATE->setText(QString(tr("Users count: %3/%1 | Total share: %2"))
                                 .arg(d->model->rowCount())
                                 .arg(WulforUtil::formatBytes(d->total_shared))
                                 .arg(d->proxy->rowCount()));
    }
    else {
        label_USERSTATE->setText(QString(tr("Users count: %1 | Total share: %2"))
                                 .arg(d->model->rowCount())
                                 .arg(WulforUtil::formatBytes(d->total_shared)));
    }

    label_LAST_STATUS->setMaximumHeight(label_USERSTATE->height());
}

void HubFrame::slotReconnect(){
    clearUsers();

    Q_D(HubFrame);
    
    if (d->client)
        d->client->reconnect();
}

void HubFrame::slotMapOnArena(){
    MainWindow *MW = MainWindow::getInstance();

    ArenaWidgetManager::getInstance()->activate(this);
}

void HubFrame::slotClose(){
    ArenaWidgetManager::getInstance()->rem(this);
}

void HubFrame::slotPMClosed(QString cid){
    Q_D(HubFrame);
    
    PMMap::iterator it = d->pm.find(cid);

    if (it != d->pm.end())
        d->pm.erase(it);
}

template < QString (UserListItem::*func)() const >
static void copyTagToClipboard(QModelIndexList &list){
    QString ret = "";
    UserListItem *item = NULL;
    
    foreach ( QModelIndex i, list ) {
        item = reinterpret_cast<UserListItem*> ( i.internalPointer() );

        if ( ret.length() > 0 )
            ret += "\n";

        if ( item )
            ret += (item->*func)();
    }

    qApp->clipboard()->setText ( ret, QClipboard::Clipboard );
}

template < qulonglong (UserListItem::*func)() const >
static void copyTagToClipboard(QModelIndexList &list){
    QString ret = "";
    UserListItem *item = NULL;
    
    foreach ( QModelIndex i, list ) {
        item = reinterpret_cast<UserListItem*> ( i.internalPointer() );

        if ( ret.length() > 0 )
            ret += "\n";

        if ( item )
            ret += WulforUtil::formatBytes((item->*func)());
    }

    qApp->clipboard()->setText ( ret, QClipboard::Clipboard );
}

void HubFrame::slotUserListMenu(const QPoint&){
    QItemSelectionModel *selection_model = treeView_USERS->selectionModel();
    QModelIndexList proxy_list = selection_model->selectedRows(0);

    if (proxy_list.size() < 1)
        return;

    QString cid = "";
    
    Q_D(HubFrame);

    if (treeView_USERS->model() != d->model){
        QModelIndex i = d->proxy->mapToSource(proxy_list.at(0));
        cid = reinterpret_cast<UserListItem*>(i.internalPointer())->cid;
    }
    else{
        QModelIndex i = proxy_list.at(0);
        cid = reinterpret_cast<UserListItem*>(i.internalPointer())->cid;
    }

    Menu::Action action = Menu::getInstance()->execUserMenu(d->client, cid);
    UserListItem *item = NULL;

    proxy_list = selection_model->selectedRows(0);

    if (proxy_list.size() < 1)
        return;

    QModelIndexList list;

    if (treeView_USERS->model() != d->model){
        foreach(QModelIndex i, proxy_list)
            list.push_back(d->proxy->mapToSource(i));
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
            copyTagToClipboard<&UserListItem::getNick> (list);

            break;
        }
        case Menu::CopyIP:
        {
            copyTagToClipboard<&UserListItem::getIP> (list);

            break;
        }
        case Menu::CopyShare:
        {
            copyTagToClipboard<&UserListItem::getShare> (list);

            break;
        }
        case Menu::CopyTag:
        {
            copyTagToClipboard<&UserListItem::getTag> (list);

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

                    (*AntiSpam::getInstance()) << eIN_WHITE << item->getNick();
                }
            }

            break;
        }
        case Menu::AntiSpamBlack:
        {
            if (AntiSpam::getInstance()){
                foreach(QModelIndex i, list){
                    item = reinterpret_cast<UserListItem*>(i.internalPointer());

                    (*AntiSpam::getInstance()) << eIN_BLACK << item->getNick();
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
    QTextEdit *editor = qobject_cast<QTextEdit*>(sender());

    if (!editor)
        return;

    QString pressedParagraph = "", nick = "";

    QTextCursor cursor = editor->cursorForPosition(editor->mapFromGlobal(QCursor::pos()));

    cursor.movePosition(QTextCursor::StartOfBlock);

    pressedParagraph = cursor.block().text();

    int row_counter = 0;
    QRegExp nick_exp("<((.+))>");
    QRegExp thirdPerson_exp("\\*\\W+((\\w+))");// * Some_nick say something

    while (!(pressedParagraph.contains(nick_exp) || pressedParagraph.contains(thirdPerson_exp)) && row_counter < 600){//try to find nick in above rows (max 600 rows)
        cursor.movePosition(QTextCursor::PreviousBlock);
        pressedParagraph = cursor.block().text();

        row_counter++;
    }

#if QT_VERSION >= 0x040600
    if (nick_exp.captureCount() >= 2)
        nick = nick_exp.cap(1);
    else if (thirdPerson_exp.exactMatch(pressedParagraph) && thirdPerson_exp.captureCount() >= 2)
        nick = thirdPerson_exp.cap(1);
#else
    // QRegExp::captureCount() function was introduced in Qt 4.6,
    if (nick_exp.capturedTexts().size() >= 2)
        nick = nick_exp.cap(1);
    else if (thirdPerson_exp.exactMatch(pressedParagraph) && thirdPerson_exp.capturedTexts().size() >= 2)
        nick = thirdPerson_exp.cap(1);
#endif

    Q_D(HubFrame);
        
    QString cid = d->model->CIDforNick(nick, _q(d->client->getHubUrl()));

    if (cid.isEmpty()){
        QMenu *m = editor->createStandardContextMenu(QCursor::pos());
        m->exec(QCursor::pos());

        delete m;

        return;
    }

    QPoint p = QCursor::pos();

    bool pmw = (editor != this->textEdit_CHAT);

    Menu::Action action = Menu::getInstance()->execChatMenu(d->client, cid, pmw);

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
        case Menu::SearchText:
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
                break;

            SearchFrame *sf = ArenaWidgetFactory().create<SearchFrame, QWidget*>(this);
            sf->fastSearch(ret, false);

            break;
        }
        case Menu::CopyNick:
        {
            qApp->clipboard()->setText(nick, QClipboard::Clipboard);

            break;
        }
        case Menu::FindInList:
        {
            UserListItem *item = d->model->itemForNick(nick, _q(d->client->getHubUrl()));

            if (item){
                QModelIndex index = d->model->index(item->row(), 0, QModelIndex());

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

            if (d->pm.contains(cid))
                ArenaWidgetManager::getInstance()->activate(d->pm[cid]);

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

   ArenaWidgetManager::getInstance()->activate(this);
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
    
    Q_D(HubFrame);

    if (d->shell_list.indexOf(runner) >= 0)
        d->shell_list.removeAt(d->shell_list.indexOf(runner));

    delete runner;
}

void HubFrame::nextMsg(){
    if (!plainTextEdit_INPUT->hasFocus())
        return;
    
    Q_D(HubFrame);

    if (d->out_messages_index < 0 ||
        d->out_messages_index+1 > d->out_messages.size()-1 ||
        d->out_messages.size() == 0)
        return;

    if (d->out_messages.at(d->out_messages_index) != plainTextEdit_INPUT->toPlainText())
        d->out_messages[d->out_messages_index] = plainTextEdit_INPUT->toPlainText();

    if (d->out_messages_index+1 <= d->out_messages.size()-1)
        d->out_messages_index++;

    plainTextEdit_INPUT->setPlainText(d->out_messages.at(d->out_messages_index));

    if (d->out_messages_unsent && d->out_messages_index == d->out_messages.size()-1){
        d->out_messages.removeLast();
        d->out_messages_unsent = false;
        d->out_messages_index = d->out_messages.size()-1;
    }
}

void HubFrame::prevMsg(){
    if (!plainTextEdit_INPUT->hasFocus())
        return;

    Q_D(HubFrame);
    
    if (d->out_messages_index < 1 ||
        d->out_messages_index-1 > d->out_messages.size()-1 ||
        d->out_messages.size() == 0)
        return;

    if (!d->out_messages_unsent && d->out_messages_index == d->out_messages.size()-1){
        d->out_messages << plainTextEdit_INPUT->toPlainText();
        d->out_messages_unsent = true;
        d->out_messages_index++;
    }

    if (d->out_messages.at(d->out_messages_index) != plainTextEdit_INPUT->toPlainText())
        d->out_messages[d->out_messages_index] = plainTextEdit_INPUT->toPlainText();

    if (d->out_messages_index >= 1)
        d->out_messages_index--;

    plainTextEdit_INPUT->setPlainText(d->out_messages.at(d->out_messages_index));
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
    
    Q_D(HubFrame);

    if (!text.isEmpty()){
        if (!d->proxy){
            d->proxy = new UserListProxyModel();
            d->proxy->setDynamicSortFilter(true);
            d->proxy->setSourceModel(d->model);
        }

        bool isRegExp = false;

        if (text.startsWith("##")){
            isRegExp = true;
            text.remove(0, 2);
        }

        if (!isRegExp){
            d->proxy->setFilterFixedString(text);
            d->proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
        }
        else{
            d->proxy->setFilterRegExp(text);
            d->proxy->setFilterCaseSensitivity(Qt::CaseSensitive);
        }

        d->proxy->setFilterKeyColumn(comboBox_COLUMNS->currentIndex());

        if (treeView_USERS->model() != d->proxy)
            treeView_USERS->setModel(d->proxy);
    }
    else if (treeView_USERS->model() != d->model)
        treeView_USERS->setModel(d->model);

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
    if (!toolButton_ALL->isChecked()){
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
        Q_D(HubFrame);

        if (WulforUtil::getInstance()->getUserCommandParams(last_user_cmd, params)){
            d->client->getMyIdentity().getParams(params, "my", true);
            d->client->getHubIdentity().getParams(params, "hub", false);

            d->client->escapeParams(params);
            d->client->sendUserCmd(uc, params);
        }
    }
}

void HubFrame::slotSettingsChanged(const QString &key, const QString &value){
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
    else if (key == WS_TRANSLATION_FILE){
        retranslateUi(this);
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
    Q_D(HubFrame);
    
    if (d->client && d->client->isConnected()){
        qApp->clipboard()->setText(_q(d->client->getIp()), QClipboard::Clipboard);
    }
}

void HubFrame::slotCopyHubTitle(){
    Q_D(HubFrame);
    
    if (d->client && d->client->isConnected()){
        qApp->clipboard()->setText(QString("%1 - %2").arg(_q(d->client->getHubName())).arg(_q(d->client->getHubDescription())), QClipboard::Clipboard);
    }
}

void HubFrame::slotCopyHubURL(){
    Q_D(HubFrame);
    
    if (d->client && d->client->isConnected()){
        qApp->clipboard()->setText(_q(d->client->getHubUrl()), QClipboard::Clipboard);
    }
}

void HubFrame::on(FavoriteManagerListener::UserAdded, const FavoriteUser& aUser) noexcept {
    emit coreFavoriteUserAdded(_q(aUser.getUser()->getCID().toBase32()));
}

void HubFrame::on(FavoriteManagerListener::UserRemoved, const FavoriteUser& aUser) noexcept {
    emit coreFavoriteUserRemoved(_q(aUser.getUser()->getCID().toBase32()));
}

void HubFrame::on(ClientListener::Connecting, Client *c) noexcept{
    Q_D(HubFrame);
    
    QString status = tr("Connecting to %1").arg(QString::fromStdString(d->client->getHubUrl()));

    emit coreConnecting(status);
}

void HubFrame::on(ClientListener::Connected, Client*) noexcept{
    Q_D(HubFrame);
    
    QString status = tr("Connected to %1").arg(QString::fromStdString(d->client->getHubUrl()));

    emit coreConnected(status);

    HubManager::getInstance()->registerHubUrl(_q(d->client->getHubUrl()), this);
}

void HubFrame::on(ClientListener::UserUpdated, Client*, const OnlineUser &user) noexcept{
    if (user.getIdentity().isHidden() && !WBGET(WB_SHOW_HIDDEN_USERS))
        return;

    VarMap params;

    getParams(params, user.getIdentity());

    emit coreUserUpdated(params, user, true);
}

void HubFrame::on(ClientListener::UsersUpdated x, Client*, const OnlineUserList &list) noexcept{
    bool showHidden = WBGET(WB_SHOW_HIDDEN_USERS);

    for (OnlineUserList::const_iterator it = list.begin(); it != list.end(); ++it){
        if ((*(*it)).getIdentity().isHidden() && !showHidden)
            break;

        VarMap params;

        getParams(params, (*(*it)).getIdentity());

        emit coreUserUpdated(params, (*(*it)), true);
    }
}

void HubFrame::on(ClientListener::UserRemoved, Client*, const OnlineUser &user) noexcept{
    if (user.getIdentity().isHidden() && !WBGET(WB_SHOW_HIDDEN_USERS))
        return;

    emit coreUserRemoved(user.getUser(), user.getIdentity().getBytesShared());
}

void HubFrame::on(ClientListener::Redirect, Client*, const string &link) noexcept{
    if(ClientManager::getInstance()->isConnected(link)) {
        emit coreStatusMsg(tr("Redirect request received to a hub that's already connected"));

        return;
    }

    if(BOOLSETTING(AUTO_FOLLOW))
        emit coreFollow(_q(link));
}

void HubFrame::on(ClientListener::Failed, Client*, const string &msg) noexcept{
    QString status = tr("Fail: %1...").arg(_q(msg));

    emit coreStatusMsg(status);
    emit coreFailed();
    emit coreHubUpdated();
}

void HubFrame::on(GetPassword, Client*) noexcept{
    emit corePassword();
}

void HubFrame::on(ClientListener::HubUpdated, Client*) noexcept{
    emit coreHubUpdated();
}

void HubFrame::on(ClientListener::Message, Client*, const ChatMessage &message) noexcept{
    if (message.text.empty())
        return;

    VarMap map;
    QString msg = _q(message.text);
    bool third = false;

    if (msg.startsWith("/me ")){
        msg.remove(0, 4);

        third = true;
    }
    else
        third = message.thirdPerson;

    Q_D(HubFrame);
    
    map["HUBURL"] = _q(d->client->getHubUrl());
    
    if(message.to && message.replyTo)
    {
        //private message
        const OnlineUser *user = (message.replyTo->getUser() == ClientManager::getInstance()->getMe())?
                                 message.to : message.replyTo;

        bool isBot = user->getIdentity().isBot() || user->getUser()->isSet(User::BOT);
        bool isHub = user->getIdentity().isHub();
        bool isOp  = user->getIdentity().isOp();

        if (isHub && BOOLSETTING(IGNORE_HUB_PMS))
            return;
        else if (isBot && BOOLSETTING(IGNORE_BOT_PMS))
            return;

        CID id           = user->getUser()->getCID();
        QString nick     =  _q(message.from->getIdentity().getNick());
        bool isInSandBox = false;
        bool isEcho      = (message.from->getUser() == ClientManager::getInstance()->getMe());
        bool hasPMWindow = d->pm.contains(_q(id.toBase32()));//PMWindow is created

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
                    AntiSpam::getInstance()->checkUser(_q(id.toBase32()), msg, _q(d->client->getHubUrl()));

                    return;
                }
            } while (0);
        }
        else if (isEcho && isInSandBox && !hasPMWindow)
            return;

        map["NICK"]  = nick;
        map["MSG"]   = msg;
        map["TIME"]  = QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP));
        map["ECHO"]  = isEcho;

        QString color = WS_CHAT_PRIV_USER_COLOR;

        if (nick == _q(d->client->getMyNick()))
            color = WS_CHAT_PRIV_LOCAL_COLOR;
        else if (isOp)
            color = WS_CHAT_OP_COLOR;
        else if (isBot)
            color = WS_CHAT_BOT_COLOR;
        else if (isHub)
            color = WS_CHAT_STAT_COLOR;

        map["CLR"] = color;
        map["3RD"] = third;
        map["CID"] = _q(id.toBase32());
        map["I4"]  = _q(ClientManager::getInstance()->getOnlineUserIdentity(message.from->getUser()).getIp());

        if (WBGET(WB_CHAT_REDIRECT_BOT_PMS) && isBot)
            emit coreMessage(map);
        else
            emit corePrivateMsg(map);

        if (!(isBot || isHub) && (message.from->getUser() != ClientManager::getInstance()->getMe()) && Util::getAway() && !hasPMWindow)
            ClientManager::getInstance()->privateMessage(HintedUser(user->getUser(), d->client->getHubUrl()), Util::getAwayMessage(), false);

        if (BOOLSETTING(LOG_PRIVATE_CHAT)){
            string info = Util::formatAdditionalInfo(map["I4"].toString().toStdString(),BOOLSETTING(USE_IP),BOOLSETTING(GET_USER_COUNTRY));
            QString qinfo = !info.empty() ? _q(info) : "";

            StringMap params;
            params["message"] = _tq(qinfo + "<" + nick + "> " + msg);
            params["hubNI"] = _tq(WulforUtil::getInstance()->getHubNames(id));
            params["hubURL"] = d->client->getHubUrl();
            params["userCID"] = id.toBase32();
            params["userNI"] = user->getIdentity().getNick();
            params["myCID"] = ClientManager::getInstance()->getMe()->getCID().toBase32();
            params["userI4"] = ClientManager::getInstance()->getOnlineUserIdentity(message.from->getUser()).getIp();
            LOG(LogManager::PM, params);
        }
    }
    else
    {
        // chat message
        const OnlineUser *user = message.from;

        if (d->chatDisabled)
            return;

        if (AntiSpam::getInstance() && AntiSpam::getInstance()->isInBlack(_q(user->getIdentity().getNick())))
            return;

        map["NICK"] = _q(user->getIdentity().getNick());
        map["MSG"]  = msg;
        map["TIME"] = QDateTime::currentDateTime().toString(WSGET(WS_CHAT_TIMESTAMP));

        QString color = WS_CHAT_USER_COLOR;

        if (user->getIdentity().isHub())
            color = WS_CHAT_STAT_COLOR;
        else if (user->getUser() == d->client->getMyIdentity().getUser())
            color = WS_CHAT_LOCAL_COLOR;
        else if (user->getIdentity().isOp())
            color = WS_CHAT_OP_COLOR;
        else if (user->getIdentity().isBot())
            color = WS_CHAT_BOT_COLOR;

        if (FavoriteManager::getInstance()->isFavoriteUser(user->getUser()))
            color = WS_CHAT_FAVUSER_COLOR;

        map["CLR"] = color;
        map["3RD"] = third;
        map["I4"]  = _q(ClientManager::getInstance()->getOnlineUserIdentity(user->getUser()).getIp());

        emit coreMessage(map);

        if (BOOLSETTING(LOG_MAIN_CHAT)){
            string info = Util::formatAdditionalInfo(map["I4"].toString().toStdString(),BOOLSETTING(USE_IP),BOOLSETTING(GET_USER_COUNTRY));
            QString qinfo = !info.empty() ? _q(info) : "";
            QString nick  =  _q(user->getIdentity().getNick());
            
            StringMap params;
            params["message"] = _tq(qinfo + "<" + nick + "> " + msg);
            d->client->getHubIdentity().getParams(params, "hub", false);
            params["hubURL"] = d->client->getHubUrl();
            params["userNI"] = _tq(nick);
            params["userI4"] = ClientManager::getInstance()->getOnlineUserIdentity(user->getUser()).getIp();
            d->client->getMyIdentity().getParams(params, "my", true);
            LOG(LogManager::CHAT, params);
        }
    }
}

void HubFrame::on(ClientListener::StatusMessage, Client*, const string &msg, int) noexcept{
    QString status = QString("%1 ").arg(_q(msg));

    emit coreStatusMsg(status);
    
    Q_D(HubFrame);

    if (BOOLSETTING(LOG_STATUS_MESSAGES)){
        StringMap params;
        d->client->getHubIdentity().getParams(params, "hub", FALSE);
        params["hubURL"] = d->client->getHubUrl();
        d->client->getMyIdentity().getParams(params, "my", TRUE);
        params["message"] = msg;
        LOG(LogManager::STATUS, params);
    }
}

void HubFrame::on(ClientListener::NickTaken, Client*) noexcept{
    Q_D(HubFrame);
    QString status = tr("Sorry, but nick \"%1\" is already taken by another user.").arg(d->client->getCurrentNick().c_str());

    emit coreStatusMsg(status);
}

void HubFrame::on(ClientListener::SearchFlood, Client*, const string &str) noexcept{
    emit coreStatusMsg(tr("Search flood detected: %1").arg(_q(str)));
}
