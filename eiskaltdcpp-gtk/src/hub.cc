/*
 * Copyright © 2004-2010 Jens Oknelid, paskharen@gmail.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#include "hub.hh"
#include <dcpp/AdcHub.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/HashManager.h>
#include <dcpp/SearchManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/UserCommand.h>
#include <dcpp/version.h>
#include <dcpp/ChatMessage.h>//NOTE: core 0.762
#include <dcpp/Util.h>
#include <dcpp/StringTokenizer.h>
#include "privatemessage.hh"
#include "search.hh"
#include "settingsmanager.hh"
#include "emoticonsdialog.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include <dcpp/StringTokenizer.h>

#include "VersionGlobal.h"

#ifdef LUA_SCRIPT
#include <dcpp/ScriptManager.h>
#endif

using namespace std;
using namespace dcpp;

const string Hub::tagPrefix = "#";

Hub::Hub(const string &address, const string &encoding):
    BookEntry(Entry::HUB, address, "hub.glade", address),
    client(NULL),
    historyIndex(0),
    totalShared(0),
    address(address),
    encoding(encoding),
    scrollToBottom(TRUE),
    PasswordDialog(FALSE),
    WaitingPassword(FALSE),
    ImgLimit(0)
{
    // Configure the dialog
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("userListCheckButton")), TRUE);

    // Initialize nick treeview
    nickView.setView(GTK_TREE_VIEW(getWidget("nickView")), true, "hub");
    nickView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING_TEXT_COLOR, 100, "Icon", "NickColor");
    nickView.insertColumn(_("Shared"), G_TYPE_INT64, TreeView::SIZE, 75);
    nickView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 85);
    nickView.insertColumn(_("Tag"), G_TYPE_STRING, TreeView::STRING, 100);
    nickView.insertColumn(_("Connection"), G_TYPE_STRING, TreeView::STRING, 85);
    nickView.insertColumn("IP", G_TYPE_STRING, TreeView::STRING, 85);
    nickView.insertColumn(_("eMail"), G_TYPE_STRING, TreeView::STRING, 90);
    nickView.insertHiddenColumn("Icon", G_TYPE_STRING);
    nickView.insertHiddenColumn("Nick Order", G_TYPE_STRING);
    nickView.insertHiddenColumn("Favorite", G_TYPE_STRING);
    nickView.insertHiddenColumn("CID", G_TYPE_STRING);
    nickView.insertHiddenColumn("NickColor", G_TYPE_STRING);
    nickView.finalize();
    nickStore = gtk_list_store_newv(nickView.getColCount(), nickView.getGTypes());
    gtk_tree_view_set_model(nickView.get(), GTK_TREE_MODEL(nickStore));
    g_object_unref(nickStore);

    nickSelection = gtk_tree_view_get_selection(nickView.get());
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(nickView.get()), GTK_SELECTION_MULTIPLE);
    string sort = WGETB("sort-favusers-first")? "Favorite" : "Nick Order";
    nickView.setSortColumn_gui(_("Nick"), sort);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);
    gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(nickView.get(), nickView.col(_("Nick"))), TRUE);
    gtk_tree_view_set_fixed_height_mode(nickView.get(), TRUE);
    gtk_tree_view_set_search_equal_func(nickView.get(), onNickListSearch_gui, 0,0);

    // Initialize the chat window
    if (WGETB("use-oem-monofont"))
    {
        PangoFontDescription *fontDesc = pango_font_description_new();
        pango_font_description_set_family(fontDesc, "Mono");
        gtk_widget_modify_font(getWidget("chatText"), fontDesc);
        pango_font_description_free(fontDesc);
    }

    // the reference count on the buffer is not incremented and caller of this function won't own a new reference.
    chatBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(getWidget("chatText")));

    /* initial markers */
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(chatBuffer, &iter);

    chatMark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);
    start_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
    end_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);
    tag_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, FALSE);
    emot_mark = gtk_text_buffer_create_mark(chatBuffer, NULL, &iter, TRUE);

    handCursor = gdk_cursor_new(GDK_HAND2);

#if !GTK_CHECK_VERSION(2, 12, 0)
	tips = gtk_tooltips_new();
	g_object_ref_sink(tips);
#endif
	// image magnet
	imageLoad.first = "";
	imageLoad.second = NULL;
	imageMagnet.first = "";
	imageMagnet.second = "";

    // menu
    g_object_ref_sink(getWidget("nickMenu"));
    g_object_ref_sink(getWidget("magnetMenu"));
    g_object_ref_sink(getWidget("linkMenu"));
    g_object_ref_sink(getWidget("hubMenu"));
    g_object_ref_sink(getWidget("chatCommandsMenu"));
    g_object_ref_sink(getWidget("imageMenu"));

    // Initialize the user command menu
    userCommandMenu = new UserCommandMenu(getWidget("usercommandMenu"), ::UserCommand::CONTEXT_USER);//NOTE: core 0.762
    addChild(userCommandMenu);

    // Emoticons dialog
    emotdialog = new EmoticonsDialog(getWidget("chatEntry"), getWidget("emotButton"), getWidget("emotPacksMenu"));
    if (!WGETB("emoticons-use"))
        gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
    useEmoticons = TRUE;

    // Chat commands
    g_object_set_data_full(G_OBJECT(getWidget("awayCommandItem")), "command", g_strdup("/away"), g_free);
    g_signal_connect(getWidget("awayCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("backCommandItem")), "command", g_strdup("/back"), g_free);
    g_signal_connect(getWidget("backCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("clearCommandItem")), "command", g_strdup("/clear"), g_free);
    g_signal_connect(getWidget("clearCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("favCommandItem")), "command", g_strdup("/fav"), g_free);
    g_signal_connect(getWidget("favCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("lsfuCommandItem")), "command", g_strdup("/lsfu"), g_free);
    g_signal_connect(getWidget("lsfuCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("helpCommandItem")), "command", g_strdup("/help"), g_free);
    g_signal_connect(getWidget("helpCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("joinCommandItem")), "command", g_strdup("/join"), g_free);
    g_signal_connect(getWidget("joinCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("meCommandItem")), "command", g_strdup("/me"), g_free);
    g_signal_connect(getWidget("meCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("rebuildCommandItem")), "command", g_strdup("/rebuild"), g_free);
    g_signal_connect(getWidget("rebuildCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("limitimgCommandItem")), "command", g_strdup("/limg"), g_free);
    g_signal_connect(getWidget("limitimgCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    g_object_set_data_full(G_OBJECT(getWidget("versionCommandItem")), "command", g_strdup("/version"), g_free);
    g_signal_connect(getWidget("versionCommandItem"), "activate", G_CALLBACK(onCommandClicked_gui), (gpointer)this);

    // chat commands button
    g_signal_connect(getWidget("chatCommandsButton"), "button-release-event", G_CALLBACK(onChatCommandButtonRelease_gui), (gpointer)this);

    // image menu
    g_signal_connect(getWidget("downloadImageItem"), "activate", G_CALLBACK(onDownloadImageClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeImageItem"), "activate", G_CALLBACK(onRemoveImageClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("openImageItem"), "activate", G_CALLBACK(onOpenImageClicked_gui), (gpointer)this);

    GtkAdjustment *adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(getWidget("chatScroll")));

    // Connect the signals to their callback functions.
    g_signal_connect(getContainer(), "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
    g_signal_connect(nickView.get(), "button-press-event", G_CALLBACK(onNickListButtonPress_gui), (gpointer)this);
    g_signal_connect(nickView.get(), "button-release-event", G_CALLBACK(onNickListButtonRelease_gui), (gpointer)this);
    g_signal_connect(nickView.get(), "key-release-event", G_CALLBACK(onNickListKeyRelease_gui), (gpointer)this);
    g_signal_connect(getWidget("chatEntry"), "activate", G_CALLBACK(onSendMessage_gui), (gpointer)this);
    g_signal_connect(getWidget("chatEntry"), "key-press-event", G_CALLBACK(onEntryKeyPress_gui), (gpointer)this);
    g_signal_connect(getWidget("chatText"), "motion-notify-event", G_CALLBACK(onChatPointerMoved_gui), (gpointer)this);
    g_signal_connect(getWidget("chatText"), "visibility-notify-event", G_CALLBACK(onChatVisibilityChanged_gui), (gpointer)this);
    g_signal_connect(adjustment, "value_changed", G_CALLBACK(onChatScroll_gui), (gpointer)this);
    g_signal_connect(adjustment, "changed", G_CALLBACK(onChatResize_gui), (gpointer)this);
    g_signal_connect(getWidget("nickToChatItem"), "activate", G_CALLBACK(onNickToChat_gui), (gpointer)this);
    g_signal_connect(getWidget("copyNickItem"), "activate", G_CALLBACK(onCopyNickItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("browseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("matchItem"), "activate", G_CALLBACK(onMatchItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("msgItem"), "activate", G_CALLBACK(onMsgItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("grantItem"), "activate", G_CALLBACK(onGrantItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("copyLinkItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("openLinkItem"), "activate", G_CALLBACK(onOpenLinkClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("copyhubItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("openhubItem"), "activate", G_CALLBACK(onOpenHubClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyURIClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchMagnetItem"), "activate", G_CALLBACK(onSearchMagnetClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("magnetPropertiesItem"), "activate", G_CALLBACK(onMagnetPropertiesClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("userListCheckButton"), "toggled", G_CALLBACK(onUserListToggled_gui), (gpointer)this);
    g_signal_connect(getWidget("emotButton"), "button-release-event", G_CALLBACK(onEmotButtonRelease_gui), (gpointer)this);
    g_signal_connect(getWidget("favoriteUserItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeFavoriteUserItem"), "activate", G_CALLBACK(onRemoveFavoriteUserClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("downloadBrowseItem"), "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("downloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("italicButton"), "clicked", G_CALLBACK(onItalicButtonClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("boldButton"), "clicked", G_CALLBACK(onBoldButtonClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("underlineButton"), "clicked", G_CALLBACK(onUnderlineButtonClicked_gui), (gpointer)this);

    gtk_widget_grab_focus(getWidget("chatEntry"));

    // Set the pane position
    gint panePosition = WGETI("nick-pane-position");
    if (panePosition > 10)
    {
        gint width;
        GtkWindow *window = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
        gtk_window_get_size(window, &width, NULL);
        gtk_paned_set_position(GTK_PANED(getWidget("pane")), width - panePosition);
    }

    history.push_back("");

    /* initial tags map */
    TagsMap[TAG_GENERAL] = createTag_gui("TAG_GENERAL", TAG_GENERAL);
    TagsMap[TAG_MYOWN] = createTag_gui("TAG_MYOWN", TAG_MYOWN);
    TagsMap[TAG_SYSTEM] = createTag_gui("TAG_SYSTEM", TAG_SYSTEM);
    TagsMap[TAG_STATUS] = createTag_gui("TAG_STATUS", TAG_STATUS);
    TagsMap[TAG_TIMESTAMP] = createTag_gui("TAG_TIMESTAMP", TAG_TIMESTAMP);
    /*-*/
    TagsMap[TAG_MYNICK] = createTag_gui("TAG_MYNICK", TAG_MYNICK);
    TagsMap[TAG_NICK] = createTag_gui("TAG_NICK", TAG_NICK);
    TagsMap[TAG_OPERATOR] = createTag_gui("TAG_OPERATOR", TAG_OPERATOR);
    TagsMap[TAG_FAVORITE] = createTag_gui("TAG_FAVORITE", TAG_FAVORITE);
    TagsMap[TAG_URL] = createTag_gui("TAG_URL", TAG_URL);

    BoldTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_WEIGHT", "weight", PANGO_WEIGHT_BOLD, NULL);
    UnderlineTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_UNDERLINE", "underline", PANGO_UNDERLINE_SINGLE, NULL);
    ItalicTag = gtk_text_buffer_create_tag(chatBuffer, "TAG_STYLE", "style", PANGO_STYLE_ITALIC, NULL);

    // Initialize favorite users list
    FavoriteManager::FavoriteMap map = FavoriteManager::getInstance()->getFavoriteUsers();
    FavoriteManager::FavoriteMap::const_iterator it;

    for (it = map.begin(); it != map.end(); ++it)
    {
        if (it->second.getUrl() == address)
        {
            userFavoriteMap.insert(UserMap::value_type(it->first.toBase32(), it->second.getNick()));
        }
    }

    // set default select tag (fix error show cursor in neutral space).
    selectedTag = TagsMap[TAG_GENERAL];
}

Hub::~Hub()
{
    disconnect_client();

    // Save the pane position
    gint width;
    GtkWindow *window = GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer());
    gtk_window_get_size(window, &width, NULL);
    gint panePosition = width - gtk_paned_get_position(GTK_PANED(getWidget("pane")));
    if (panePosition > 10)
        WSET("nick-pane-position", panePosition);

    if (handCursor)
    {
        gdk_cursor_unref(handCursor);
        handCursor = NULL;
    }

    delete emotdialog;

#if !GTK_CHECK_VERSION(2, 12, 0)
    g_object_unref(tips);
#endif
    g_object_unref(getWidget("nickMenu"));
    g_object_unref(getWidget("magnetMenu"));
    g_object_unref(getWidget("linkMenu"));
    g_object_unref(getWidget("hubMenu"));
    g_object_unref(getWidget("chatCommandsMenu"));
    g_object_unref(getWidget("imageMenu"));
}

void Hub::show()
{
    // Connect to the hub
    typedef Func2<Hub, string, string> F2;
    F2 *func = new F2(this, &Hub::connectClient_client, address, encoding);
    WulforManager::get()->dispatchClientFunc(func);
}

void Hub::setStatus_gui(string statusBar, string text)
{
    if (!statusBar.empty() && !text.empty())
    {
        if (statusBar == "statusMain")
            text = "[" + Util::getShortTimeString() + "] " + text;

        gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
        gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, text.c_str());
    }
}

bool Hub::findUser_gui(const string &cid, GtkTreeIter *iter)
{
    unordered_map<string, GtkTreeIter>::const_iterator it = userIters.find(cid);

    if (it != userIters.end())
    {
        if (iter)
            *iter = it->second;

        return TRUE;
    }

    return FALSE;
}

bool Hub::findNick_gui(const string &nick, GtkTreeIter *iter)
{
    unordered_map<string, string>::const_iterator it = userMap.find(nick);

    if (it != userMap.end())
        return findUser_gui(it->second, iter);

    return FALSE;
}

void Hub::updateUser_gui(ParamMap params)
{
    GtkTreeIter iter;
    int64_t shared = Util::toInt64(params["Shared"]);
    const string& cid = params["CID"];
    const string icon = "eiskaltdcpp-" + params["Icon"];
    const string &Nick = params["Nick"];
    const string &nickOrder = params["Nick Order"];
    bool favorite = userFavoriteMap.find(cid) != userFavoriteMap.end();

    if (findUser_gui(cid, &iter))
    {
        totalShared += shared - nickView.getValue<int64_t>(&iter, _("Shared"));
        string nick = nickView.getString(&iter, _("Nick"));

        if (nick != Nick)
        {
            // User has changed nick, update userMap and remove the old Nick tag
            userMap.erase(nick);
            removeTag_gui(nick);
            userMap.insert(UserMap::value_type(Nick, cid));

            // update favorite
            if (favorite)
                userFavoriteMap[cid] = Nick;
        }

        gtk_list_store_set(nickStore, &iter,
            nickView.col(_("Nick")), Nick.c_str(),
            nickView.col(_("Shared")), shared,
            nickView.col(_("Description")), params["Description"].c_str(),
            nickView.col(_("Tag")), params["Tag"].c_str(),
            nickView.col(_("Connection")), params["Connection"].c_str(),
            nickView.col("IP"), params["IP"].c_str(),
            nickView.col(_("eMail")), params["eMail"].c_str(),
            nickView.col("Icon"), icon.c_str(),
            nickView.col("Nick Order"), nickOrder.c_str(),
            nickView.col("Favorite"), favorite? ("f" + nickOrder).c_str() : nickOrder.c_str(),
            nickView.col("CID"), cid.c_str(),
            nickView.col("NickColor"), favorite? "#ff0000" : "#000000",
            -1);
    }
    else
    {
        totalShared += shared;
        userMap.insert(UserMap::value_type(Nick, cid));

        gtk_list_store_insert_with_values(nickStore, &iter, userMap.size(),
            nickView.col(_("Nick")), Nick.c_str(),
            nickView.col(_("Shared")), shared,
            nickView.col(_("Description")), params["Description"].c_str(),
            nickView.col(_("Tag")), params["Tag"].c_str(),
            nickView.col(_("Connection")), params["Connection"].c_str(),
            nickView.col("IP"), params["IP"].c_str(),
            nickView.col(_("eMail")), params["eMail"].c_str(),
            nickView.col("Icon"), icon.c_str(),
            nickView.col("Nick Order"), nickOrder.c_str(),
            nickView.col("Favorite"), favorite? ("f" + nickOrder).c_str() : nickOrder.c_str(),
            nickView.col("CID"), cid.c_str(),
            nickView.col("NickColor"), favorite? "#ff0000" : "#000000",
            -1);

        userIters.insert(UserIters::value_type(cid, iter));

        if (WGETB("show-joins"))
        {
            // Show joins in chat by default
            addStatusMessage_gui(Nick + _(" has joined"), Msg::STATUS, favorite? Sound::FAVORITE_USER_JOIN : Sound::NONE);
            string message = Nick + _(" has joined hub ") + client->getHubName();
            WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

            if (favorite)
                Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
        }
        else if (WGETB("fav-show-joins") && favorite)
        {
            // Only show joins for favorite users
            string message = Nick + _(" has joined hub ") + client->getHubName();
            addStatusMessage_gui(Nick + _(" has joined"), Msg::STATUS, Sound::FAVORITE_USER_JOIN);
            WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
            Notify::get()->showNotify("", message, Notify::FAVORITE_USER_JOIN);
        }
    }

    setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
    setStatus_gui("statusShared", Util::formatBytes(totalShared));
}

void Hub::removeUser_gui(string cid)
{
    GtkTreeIter iter;
    string nick, order;

    if (findUser_gui(cid, &iter))
    {
        order = nickView.getString(&iter, "Favorite");
        nick = nickView.getString(&iter, _("Nick"));
        totalShared -= nickView.getValue<int64_t>(&iter, _("Shared"));
        gtk_list_store_remove(nickStore, &iter);
        removeTag_gui(nick);
        userMap.erase(nick);
        userIters.erase(cid);
        setStatus_gui("statusUsers", Util::toString(userMap.size()) + _(" Users"));
        setStatus_gui("statusShared", Util::formatBytes(totalShared));

        if (WGETB("show-joins"))
        {
            // Show parts in chat by default
            string message = nick + _(" has quit hub ") + client->getHubName();
            addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, order[0] == 'f'? Sound::FAVORITE_USER_QUIT : Sound::NONE);
            WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);

            if (order[0] == 'f')
                Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
        }
        else if (WGETB("fav-show-joins") && order[0] == 'f')
        {
            // Only show parts for favorite users
            string message = nick + _(" has quit hub ") + client->getHubName();
            addStatusMessage_gui(nick + _(" has quit"), Msg::STATUS, Sound::FAVORITE_USER_QUIT);
            WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
            Notify::get()->showNotify("", message, Notify::FAVORITE_USER_QUIT);
        }
    }
}

/*
 * Remove nick tag from text view
 */
void Hub::removeTag_gui(const string &nick)
{
    GtkTextTagTable *textTagTable = gtk_text_buffer_get_tag_table(chatBuffer);
    GtkTextTag *tag = gtk_text_tag_table_lookup(textTagTable, (tagPrefix + nick).c_str());
    if (tag)
        gtk_text_tag_table_remove(textTagTable, tag);
}

void Hub::clearNickList_gui()
{
    // Remove all old nick tags from the text view
    unordered_map<string, string>::const_iterator it;
    for (it = userMap.begin(); it != userMap.end(); ++it)
        removeTag_gui(it->first);

    gtk_list_store_clear(nickStore);
    userMap.clear();
    userIters.clear();
    totalShared = 0;
    setStatus_gui("statusUsers", _("0 Users"));
    setStatus_gui("statusShared", "0 B");
}

void Hub::popupNickMenu_gui()
{
    // Build user command menu
    userCommandMenu->cleanMenu_gui();

    GtkTreeIter iter;
    GList *list = gtk_tree_selection_get_selected_rows(nickSelection, NULL);

    for (GList *i = list; i; i = i->next)
    {
        GtkTreePath *path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(nickStore), &iter, path))
        {
            userCommandMenu->addUser(nickView.getString(&iter, "CID"));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);

    userCommandMenu->addHub(client->getHubUrl());
    userCommandMenu->buildMenu_gui();

    gtk_menu_popup(GTK_MENU(getWidget("nickMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
    gtk_widget_show_all(getWidget("nickMenu"));
}

void Hub::getPassword_gui()
{
    if(!BOOLSETTING(PROMPT_PASSWORD))
    {
        addStatusMessage_gui(_("Waiting for input password (don't remove /password before your password)"), Msg::STATUS, Sound::NONE);

        gint pos = 0;
        GtkWidget *chatEntry = getWidget("chatEntry");
        gtk_editable_delete_text(GTK_EDITABLE(chatEntry), pos, -1);
        gtk_editable_insert_text(GTK_EDITABLE(chatEntry), "/password ", -1, &pos);
        gtk_editable_set_position(GTK_EDITABLE(chatEntry), pos);

        if (!WaitingPassword)
            WaitingPassword = TRUE;
        return;
    }

    if (PasswordDialog)
        return;

    // Create password dialog
    string title = client->getHubUrl(); //_("Enter hub password")
    GtkWidget *dialog = gtk_dialog_new_with_buttons(title.c_str(),
        GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
        GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_STOCK_OK,
        GTK_RESPONSE_OK,
        GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,
        NULL);
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    GtkWidget *box = gtk_vbox_new(TRUE, 0);
    GtkWidget *entry = gtk_entry_new();
    g_object_set(entry, "can-focus", TRUE, "visibility", FALSE, "activates-default", TRUE, NULL);

    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 8);

    GtkWidget *frame = gtk_frame_new(NULL);
    g_object_set(frame, "border-width", 8, NULL);

    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), _("<b>Enter your password</b>"));
    gtk_frame_set_label_widget(GTK_FRAME(frame), label);

    gtk_container_add(GTK_CONTAINER(frame), box);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), frame);

    g_object_set_data(G_OBJECT(dialog), "password-entry", (gpointer) entry);

    g_signal_connect(dialog, "response", G_CALLBACK(onPasswordDialog), (gpointer) this);
    gtk_widget_show_all(dialog);

    PasswordDialog = TRUE;
    WaitingPassword = TRUE;
}

void Hub::onPasswordDialog(GtkWidget *dialog, gint response, gpointer data)
{
    Hub *hub = (Hub *) data;
    GtkWidget *entry = (GtkWidget *) g_object_get_data(G_OBJECT(dialog), "password-entry");

    if (response == GTK_RESPONSE_OK)
    {
        string password = gtk_entry_get_text(GTK_ENTRY(entry));
        typedef Func1<Hub, string> F1;
        F1 *func = new F1(hub, &Hub::setPassword_client, password);
        WulforManager::get()->dispatchClientFunc(func);
    }
    else
        hub->client->disconnect(TRUE);

    gtk_widget_destroy(dialog);
    hub->PasswordDialog = FALSE;
    hub->WaitingPassword = FALSE;
}

void Hub::addStatusMessage_gui(string message, Msg::TypeMsg typemsg, Sound::TypeSound sound)
{
    if (!message.empty())
    {
        if (sound != Sound::NONE)
            Sound::get()->playSound(sound);

        setStatus_gui("statusMain", message);

        if (WGETB("status-in-chat"))
        {
            string line = "*** " + message;
            addMessage_gui("", line, typemsg);
        }
    }
}

void Hub::nickToChat_gui(const string &nick)
{
    if (!gtk_widget_is_focus(getWidget("chatEntry")))
        gtk_widget_grab_focus(getWidget("chatEntry"));

    gint pos = gtk_editable_get_position(GTK_EDITABLE(getWidget("chatEntry")));
    gtk_editable_insert_text(GTK_EDITABLE(getWidget("chatEntry")), (nick + (!pos? ": " : " ")).c_str(), -1, &pos);
    gtk_editable_set_position(GTK_EDITABLE(getWidget("chatEntry")), pos);
}

void Hub::addMessage_gui(string cid, string message, Msg::TypeMsg typemsg)
{
    if (message.empty())
        return;

    GtkTextIter iter;
    string line = "";

    if (BOOLSETTING(TIME_STAMPS))
        line += "[" + Util::getShortTimeString() + "] ";

    line += message + "\n";

    gtk_text_buffer_get_end_iter(chatBuffer, &iter);
    gtk_text_buffer_insert(chatBuffer, &iter, line.c_str(), line.size());

    switch (typemsg)
    {
        case Msg::MYOWN:
            tagMsg = TAG_MYOWN;
            break;

        case Msg::SYSTEM:
            tagMsg = TAG_SYSTEM;
            break;

        case Msg::STATUS:
            tagMsg = TAG_STATUS;
            break;

        case Msg::GENERAL:

        default:
            tagMsg = TAG_GENERAL;
    }

    totalEmoticons = 0;

    applyTags_gui(cid, line);

    gtk_text_buffer_get_end_iter(chatBuffer, &iter);

    // Limit size of chat text
    if (gtk_text_buffer_get_line_count(chatBuffer) > maxLines + 1)
    {
        GtkTextIter next;
        gtk_text_buffer_get_start_iter(chatBuffer, &iter);
        gtk_text_buffer_get_iter_at_line(chatBuffer, &next, 1);
        gtk_text_buffer_delete(chatBuffer, &iter, &next);
    }
}

void Hub::applyTags_gui(const string cid, const string &line)
{
    GtkTextIter start_iter;
    gtk_text_buffer_get_end_iter(chatBuffer, &start_iter);

    // apply timestamp tag
    if (BOOLSETTING(TIME_STAMPS))
    {
        gtk_text_iter_backward_chars(&start_iter,
            g_utf8_strlen(line.c_str(), -1) - g_utf8_strlen(Util::getShortTimeString().c_str(), -1) - 2);

        GtkTextIter ts_start_iter, ts_end_iter;
        ts_end_iter = start_iter;

        gtk_text_buffer_get_end_iter(chatBuffer, &ts_start_iter);
        gtk_text_iter_backward_chars(&ts_start_iter, g_utf8_strlen(line.c_str(), -1));

        gtk_text_buffer_apply_tag(chatBuffer, TagsMap[TAG_TIMESTAMP], &ts_start_iter, &ts_end_iter);
    }
    else
        gtk_text_iter_backward_chars(&start_iter, g_utf8_strlen(line.c_str(), -1));

    // apply tags: nick, link, hub-url, magnet
    GtkTextIter tag_start_iter, tag_end_iter;

    gtk_text_buffer_move_mark(chatBuffer, start_mark, &start_iter);
    gtk_text_buffer_move_mark(chatBuffer, end_mark, &start_iter);

    string tagName;
    TypeTag tagStyle = TAG_GENERAL;

    bool firstNick = FALSE;
    bool start = FALSE;

    for(;;)
    {
        do {
            gunichar ch = gtk_text_iter_get_char(&start_iter);

            if (!g_unichar_isspace(ch))
                break;

        } while (gtk_text_iter_forward_char(&start_iter));

        if(!start)
        {
            gtk_text_buffer_move_mark(chatBuffer, start_mark, &start_iter);
            gtk_text_buffer_move_mark(chatBuffer, end_mark, &start_iter);

            start = TRUE;
        }

        tag_start_iter = start_iter;

        for(;gtk_text_iter_forward_char(&start_iter);)
        {
            gunichar ch = gtk_text_iter_get_char(&start_iter);

            if (g_unichar_isspace(ch))
                break;
        }

        tag_end_iter = start_iter;

        GCallback callback = NULL;
        bool isNick = FALSE;
        bool image_tag = FALSE;
        bool bold_tag = FALSE;
        bool italic_tag = FALSE;
        bool underline_tag = FALSE;
        string image_magnet, bold_text, italic_text, underline_text;
        gchar *temp = gtk_text_iter_get_text(&tag_start_iter, &tag_end_iter);

        if (!C_EMPTY(temp))
        {
            tagName = temp;
            GtkTreeIter iter;

            // Special case: catch nicks in the form <nick> at the beginning of the line.
            if (!firstNick && tagName[0] == '<' && tagName[tagName.size() - 1] == '>')
            {
                tagName = tagName.substr(1, tagName.size() - 2);
                firstNick = TRUE;
            }

            if (findNick_gui(tagName, &iter))
            {
                isNick = TRUE;
                callback = G_CALLBACK(onNickTagEvent_gui);
                string order = nickView.getString(&iter, "Favorite");

                if (tagName == client->getMyNick())
                    tagStyle = TAG_MYNICK;
                else if (order[0] == 'f')
                    tagStyle = TAG_FAVORITE;
                else if (order[0] == 'o')
                    tagStyle = TAG_OPERATOR;
                else if (order[0] == 'u')
                    tagStyle = TAG_NICK;

                tagName = tagPrefix + tagName;
            }
            else
            {
                // Support bbCode: [i]italic-text[/i], [u]underline-text[/u]
                // [img]magnet-link[/img]

                bool notlink = FALSE;
                if (g_ascii_strncasecmp(tagName.c_str(), "[img]", 5) == 0)
                {
                    string::size_type i = tagName.rfind("[/img]");
                    if (i != string::npos)
                    {
                        image_magnet = tagName.substr(5, i - 5);
                        if (WulforUtil::isMagnet(image_magnet))
                            notlink = image_tag = TRUE;
                    }
                }
                else if (g_ascii_strncasecmp(tagName.c_str(), "[b]", 3) == 0)
                {
                    string::size_type i = tagName.rfind("[/b]");
                    if (i != string::npos)
                    {
                        bold_text = tagName.substr(3, i - 3);
                        notlink = bold_tag = TRUE;
                    }
                }
                else if (g_ascii_strncasecmp(tagName.c_str(), "[i]", 3) == 0)
                {
                    string::size_type i = tagName.rfind("[/i]");
                    if (i != string::npos)
                    {
                        italic_text = tagName.substr(3, i - 3);
                        notlink = italic_tag = TRUE;
                    }
                }
                else if (g_ascii_strncasecmp(tagName.c_str(), "[u]", 3) == 0)
                {
                    string::size_type i = tagName.rfind("[/u]");
                    if (i != string::npos)
                    {
                        underline_text = tagName.substr(3, i - 3);
                        notlink = underline_tag = TRUE;
                    }
                }

                if (!notlink)
                {
                    if (WulforUtil::isLink(tagName))
                        callback = G_CALLBACK(onLinkTagEvent_gui);
                    else if (WulforUtil::isHubURL(tagName))
                        callback = G_CALLBACK(onHubTagEvent_gui);
                    else if (WulforUtil::isMagnet(tagName))
                        callback = G_CALLBACK(onMagnetTagEvent_gui);

                    tagStyle = TAG_URL;
               }
            }
        }

        g_free(temp);

        if (image_tag)
        {
            gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
            string name, tth, target;
            int64_t size;

            if (WulforUtil::splitMagnet(image_magnet, name, size, tth))
            {
                gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);

                GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(chatBuffer, &tag_start_iter);
                GtkWidget *event_box = gtk_event_box_new();

                // Creating a visible window may cause artifacts that are visible to the user.
                gtk_event_box_set_visible_window(GTK_EVENT_BOX(event_box), FALSE);

                GtkWidget *image = gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_BUTTON);
                gtk_container_add(GTK_CONTAINER(event_box), image);
                gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(getWidget("chatText")), event_box, anchor);
                g_object_set_data_full(G_OBJECT(event_box), "magnet", g_strdup(image_magnet.c_str()), g_free);
                g_object_set_data_full(G_OBJECT(event_box), "cid", g_strdup(cid.c_str()), g_free);
                g_signal_connect(G_OBJECT(image), "expose-event", G_CALLBACK(expose), NULL);
                g_signal_connect(G_OBJECT(event_box), "event", G_CALLBACK(onImageEvent_gui), (gpointer)this);
                gtk_widget_show_all(event_box);

                imageList.insert(ImageList::value_type(image, tth));
                string text = "name: " + name + "\n" + "size: " + Util::formatBytes(size);
#if GTK_CHECK_VERSION(2, 12, 0)
                gtk_widget_set_tooltip_text(event_box, text.c_str());
#else
                gtk_tooltips_set_tip(tips, event_box, text.c_str(), text.c_str());
#endif
                g_signal_connect(G_OBJECT(image), "destroy", G_CALLBACK(onImageDestroy_gui), (gpointer)this);

                if (ImgLimit)
                {
                    if (ImgLimit > 0)
                        ImgLimit--;

                    typedef Func4<Hub, string, int64_t, string, string> F4;
                    target = Util::getPath(Util::PATH_USER_CONFIG) + "Images" + PATH_SEPARATOR_STR + tth;
                    F4 *func = new F4(this, &Hub::download_client, target, size, tth, cid);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
        }
        else if (bold_tag)
        {
            dcassert(tagMsg >= TAG_GENERAL && tagMsg < TAG_TIMESTAMP);

            gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
            gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
            gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
                                             bold_text.c_str(), bold_text.size(), BoldTag, TagsMap[tagMsg], NULL);
        }
        else if (italic_tag)
        {
            dcassert(tagMsg >= TAG_GENERAL && tagMsg < TAG_TIMESTAMP);

            gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
            gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
            gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
                                             italic_text.c_str(), italic_text.size(), ItalicTag, TagsMap[tagMsg], NULL);
        }
        else if (underline_tag)
        {
            dcassert(tagMsg >= TAG_GENERAL && tagMsg < TAG_TIMESTAMP);

            gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);
            gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
            gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
                                             underline_text.c_str(), underline_text.size(), UnderlineTag, TagsMap[tagMsg], NULL);
        }

        if (image_tag || bold_tag || italic_tag || underline_tag)
        {

            applyEmoticons_gui();

            gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, tag_mark);

            if (gtk_text_iter_is_end(&start_iter))
                return;

            start = FALSE;

            continue;
        }

        if (callback)
        {
            gtk_text_buffer_move_mark(chatBuffer, tag_mark, &tag_end_iter);

            // check for the tags in our buffer
            GtkTextTag *tag = gtk_text_tag_table_lookup (gtk_text_buffer_get_tag_table(chatBuffer), tagName.c_str());

            if (!tag)
            {
                if (isNick)
                    tag = gtk_text_buffer_create_tag(chatBuffer, tagName.c_str(), NULL);
                else
                    tag = gtk_text_buffer_create_tag(chatBuffer, tagName.c_str(), "underline", PANGO_UNDERLINE_SINGLE, NULL);

                g_signal_connect(tag, "event", callback, (gpointer)this);
            }

            /* apply tags */
            if (callback == G_CALLBACK(onMagnetTagEvent_gui) && WGETB("use-magnet-split"))
            {
                string line;

                if (WulforUtil::splitMagnet(tagName, line))
                {
                    dcassert(tagStyle == TAG_URL);

                    gtk_text_buffer_delete(chatBuffer, &tag_start_iter, &tag_end_iter);
                    gtk_text_buffer_insert_with_tags(chatBuffer, &tag_start_iter,
                                                     line.c_str(), line.size(), tag, TagsMap[tagStyle], NULL);
                }
            }
            else
            {
                dcassert(tagStyle >= TAG_MYNICK && tagStyle < TAG_LAST);

                gtk_text_buffer_apply_tag(chatBuffer, tag, &tag_start_iter, &tag_end_iter);
                gtk_text_buffer_apply_tag(chatBuffer, TagsMap[tagStyle], &tag_start_iter, &tag_end_iter);
            }

            applyEmoticons_gui();

            gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, tag_mark);

            if (gtk_text_iter_is_end(&start_iter))
                return;

            start = FALSE;
        }
        else
        {
            if (gtk_text_iter_is_end(&start_iter))
            {
                if (!gtk_text_iter_equal(&tag_start_iter, &tag_end_iter))
                    gtk_text_buffer_move_mark(chatBuffer, end_mark, &tag_end_iter);

                applyEmoticons_gui();

                break;
            }

            gtk_text_buffer_move_mark(chatBuffer, end_mark, &tag_end_iter);
        }
    }
}

void Hub::applyEmoticons_gui()
{
    GtkTextIter start_iter, end_iter;

    gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, start_mark);
    gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

    if(gtk_text_iter_equal(&start_iter, &end_iter))
        return;

    /* apply general tag */
    dcassert(tagMsg >= TAG_GENERAL && tagMsg < TAG_TIMESTAMP);
    gtk_text_buffer_apply_tag(chatBuffer, TagsMap[tagMsg], &start_iter, &end_iter);

    /* emoticons */
    if (tagMsg == TAG_SYSTEM || tagMsg == TAG_STATUS)
    {
        return;
    }
    else if (!Emoticons::get()->useEmoticons_gui())
    {
        setStatus_gui("statusMain", _(" *** Emoticons not loaded"));
        return;
    }
    else if (!useEmoticons)
    {
        setStatus_gui("statusMain", _(" *** Emoticons mode off"));
        return;
    }
    else if (totalEmoticons >= EMOTICONS_MAX)
    {
        setStatus_gui("statusMain", _(" *** Emoticons limit"));
        return;
    }

    bool search;
    gint searchEmoticons = 0;

    GtkTextIter tmp_end_iter,
        match_start,
        match_end,
        p_start,
        p_end;

    Emot::Iter p_it;
    gint set_start, new_start;
    Emot::List &list = Emoticons::get()->getPack_gui();

    /* set start mark */
    gtk_text_buffer_move_mark(chatBuffer, emot_mark, &start_iter);

    for (;;)
    {
        /* get start and end iter positions at marks */
        gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, emot_mark);
        gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

        search = FALSE;
        set_start = gtk_text_iter_get_offset(&end_iter);

        for (Emot::Iter it = list.begin(); it != list.end(); ++it)
        {
            GList *names = (*it)->getNames();

            for (GList *p = names; p != NULL; p = p->next)
            {
                if (gtk_text_iter_forward_search(&start_iter,
                    (gchar *)p->data,
                    GTK_TEXT_SEARCH_VISIBLE_ONLY,
                    &match_start,
                    &match_end,
                    &end_iter))
                {
                    if (!search)
                    {
                        search = TRUE;
                        end_iter = match_start;

                        /* set new limit search */
                        gtk_text_buffer_get_iter_at_mark(chatBuffer, &tmp_end_iter, end_mark);
                        for (int i = 1; !gtk_text_iter_equal(&end_iter, &tmp_end_iter) && i <= Emot::SIZE_NAME;
                            gtk_text_iter_forward_chars(&end_iter, 1), i++);

                    }

                    new_start = gtk_text_iter_get_offset(&match_start);

                    if (new_start < set_start)
                    {
                        set_start = new_start;

                        p_start = match_start;
                        p_end = match_end;

                        p_it = it;

                        if (gtk_text_iter_equal(&start_iter, &match_start))
                        {
                            it = list.end() - 1;
                            break;
                        }
                    }
                }
            }
        }

        if (search)
        {
            if (totalEmoticons >= EMOTICONS_MAX)
            {
                setStatus_gui("statusMain", _(" *** Emoticons limit"));
                return;
            }

            /* delete text-emoticon and insert pixbuf-emoticon */
            gtk_text_buffer_delete(chatBuffer, &p_start, &p_end);
            gtk_text_buffer_insert_pixbuf(chatBuffer, &p_start, (*p_it)->getPixbuf());

            searchEmoticons++;
            totalEmoticons++;

            /* set emoticon mark to start */
            gtk_text_buffer_move_mark(chatBuffer, emot_mark, &p_start);

            /* check full emoticons */
            gtk_text_buffer_get_iter_at_mark(chatBuffer, &start_iter, start_mark);
            gtk_text_buffer_get_iter_at_mark(chatBuffer, &end_iter, end_mark);

            if (gtk_text_iter_get_offset(&end_iter) - gtk_text_iter_get_offset(&start_iter) == searchEmoticons - 1)
                return;
        }
        else
            return;
    }
}

/*
 * Unfortunately, we can't underline the tag on mouse over since it would
 * underline all the tags with that name.
 */
void Hub::updateCursor_gui(GtkWidget *widget)
{
    gint x, y, buf_x, buf_y;
    GtkTextIter iter;
    GSList *tagList;
    GtkTextTag *newTag = NULL;

    gdk_window_get_pointer(widget->window, &x, &y, NULL);

    // Check for tags under the cursor, and change mouse cursor appropriately
    gtk_text_view_window_to_buffer_coords(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_WIDGET, x, y, &buf_x, &buf_y);
    gtk_text_view_get_iter_at_location(GTK_TEXT_VIEW(widget), &iter, buf_x, buf_y);
    tagList = gtk_text_iter_get_tags(&iter);

    if (tagList != NULL)
    {
        newTag = GTK_TEXT_TAG(tagList->data);

        if (find(TagsMap + TAG_MYNICK, TagsMap + TAG_LAST, newTag) != TagsMap + TAG_LAST)
        {
            GSList *nextList = g_slist_next(tagList);

            if (nextList != NULL)
                newTag = GTK_TEXT_TAG(nextList->data);
            else
                newTag = NULL;
        }

        g_slist_free(tagList);
    }

    if (newTag != selectedTag)
    {
        // Cursor is in transition.
        if (newTag != NULL)
        {
            // Cursor is entering a tag.
            selectedTagStr = newTag->name;

            if (find(TagsMap, TagsMap + TAG_MYNICK, newTag) == TagsMap + TAG_MYNICK)
            {
                // Cursor was in neutral space.
                gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), handCursor);
            }
            else
                gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
        }
        else
        {
            // Cursor is entering neutral space.
            gdk_window_set_cursor(gtk_text_view_get_window(GTK_TEXT_VIEW(widget), GTK_TEXT_WINDOW_TEXT), NULL);
        }
        selectedTag = newTag;
    }
}

void Hub::preferences_gui()
{
    WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
    string fore, back;
    int bold, italic;

    for (int i = TAG_FIRST; i < TAG_LAST; i++)
    {
        getSettingTag_gui(wsm, (TypeTag)i, fore, back, bold, italic);

        g_object_set(TagsMap[i],
            "foreground", fore.c_str(),
            "background", back.c_str(),
            "weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
            "style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
            NULL);
    }

    gtk_widget_queue_draw(getWidget("chatText"));
    gtk_widget_queue_draw(getWidget("nickView"));
    gtk_widget_queue_draw(getWidget("emotButton"));

    if (!WGETB("emoticons-use"))
    {
        if (GTK_WIDGET_IS_SENSITIVE(getWidget("emotButton")))
            gtk_widget_set_sensitive(getWidget("emotButton"), FALSE);
    }
    else if (!GTK_WIDGET_IS_SENSITIVE(getWidget("emotButton")))
    {
        gtk_widget_set_sensitive(getWidget("emotButton"), TRUE);
    }

    // resort users
    string sort = WGETB("sort-favusers-first")? "Favorite" : "Nick Order";
    nickView.setSortColumn_gui(_("Nick"), sort);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(nickStore), nickView.col(sort), GTK_SORT_ASCENDING);
}

void Hub::getSettingTag_gui(WulforSettingsManager *wsm, TypeTag type, string &fore, string &back, int &bold, int &italic)
{
    switch (type)
    {
        case TAG_MYOWN:

            fore = wsm->getString("text-myown-fore-color");
            back = wsm->getString("text-myown-back-color");
            bold = wsm->getInt("text-myown-bold");
            italic = wsm->getInt("text-myown-italic");
        break;

        case TAG_SYSTEM:

            fore = wsm->getString("text-system-fore-color");
            back = wsm->getString("text-system-back-color");
            bold = wsm->getInt("text-system-bold");
            italic = wsm->getInt("text-system-italic");
        break;

        case TAG_STATUS:

            fore = wsm->getString("text-status-fore-color");
            back = wsm->getString("text-status-back-color");
            bold = wsm->getInt("text-status-bold");
            italic = wsm->getInt("text-status-italic");
        break;

        case TAG_TIMESTAMP:

            fore = wsm->getString("text-timestamp-fore-color");
            back = wsm->getString("text-timestamp-back-color");
            bold = wsm->getInt("text-timestamp-bold");
            italic = wsm->getInt("text-timestamp-italic");
        break;

        case TAG_MYNICK:

            fore = wsm->getString("text-mynick-fore-color");
            back = wsm->getString("text-mynick-back-color");
            bold = wsm->getInt("text-mynick-bold");
            italic = wsm->getInt("text-mynick-italic");
        break;

        case TAG_OPERATOR:

            fore = wsm->getString("text-op-fore-color");
            back = wsm->getString("text-op-back-color");
            bold = wsm->getInt("text-op-bold");
            italic = wsm->getInt("text-op-italic");
        break;

        case TAG_FAVORITE:

            fore = wsm->getString("text-fav-fore-color");
            back = wsm->getString("text-fav-back-color");
            bold = wsm->getInt("text-fav-bold");
            italic = wsm->getInt("text-fav-italic");
        break;

        case TAG_URL:

            fore = wsm->getString("text-url-fore-color");
            back = wsm->getString("text-url-back-color");
            bold = wsm->getInt("text-url-bold");
            italic = wsm->getInt("text-url-italic");
        break;

        case TAG_NICK:

            fore = wsm->getString("text-general-fore-color");
            back = wsm->getString("text-general-back-color");
            italic = wsm->getInt("text-general-italic");

            if (wsm->getBool("text-bold-autors"))
                bold = 1;
            else
                bold = 0;
        break;

        case TAG_GENERAL:

        default:
            fore = wsm->getString("text-general-fore-color");
            back = wsm->getString("text-general-back-color");
            bold = wsm->getInt("text-general-bold");
            italic = wsm->getInt("text-general-italic");
    }
}

GtkTextTag* Hub::createTag_gui(const string &tagname, TypeTag type)
{
    WulforSettingsManager *wsm = WulforSettingsManager::getInstance();
    GtkTextTag *tag = gtk_text_tag_table_lookup(gtk_text_buffer_get_tag_table(chatBuffer), tagname.c_str());

    if (!tag)
    {
        string fore, back;
        int bold, italic;

        getSettingTag_gui(wsm, type, fore, back, bold, italic);

        tag = gtk_text_buffer_create_tag(chatBuffer, tagname.c_str(),
            "foreground", fore.c_str(),
            "background", back.c_str(),
            "weight", bold ? TEXT_WEIGHT_BOLD : TEXT_WEIGHT_NORMAL,
            "style", italic ? TEXT_STYLE_ITALIC : TEXT_STYLE_NORMAL,
            NULL);
    }

    return tag;
}

void Hub::addStatusMessage_gui(string message, Msg::TypeMsg typemsg, Sound::TypeSound sound, Notify::TypeNotify notify)
{
    if (notify == Notify::HUB_CONNECT)
            setIcon_gui(WGETS("icon-hub-online"));
    else if (notify == Notify::HUB_DISCONNECT)
            setIcon_gui(WGETS("icon-hub-offline"));
    addStatusMessage_gui(message, typemsg, sound);
    Notify::get()->showNotify("<b>" + client->getHubUrl() + ":</b> ", message, notify);
}

gboolean Hub::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    gtk_widget_grab_focus(hub->getWidget("chatEntry"));

    // fix select text
    gtk_editable_set_position(GTK_EDITABLE(hub->getWidget("chatEntry")), -1);

    return TRUE;
}

gboolean Hub::onNickListButtonPress_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->type == GDK_BUTTON_PRESS || event->type == GDK_2BUTTON_PRESS)
        hub->oldType = event->type;

    if (event->button == 3)
    {
        GtkTreePath *path;
        if (gtk_tree_view_get_path_at_pos(hub->nickView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
        {
            bool selected = gtk_tree_selection_path_is_selected(hub->nickSelection, path);
            gtk_tree_path_free(path);

            if (selected)
                return TRUE;
        }
    }

    return FALSE;
}

gboolean Hub::onNickListButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        if (event->button == 1 && hub->oldType == GDK_2BUTTON_PRESS)
        {
            if (WGETB("pm"))
                hub->onMsgItemClicked_gui(NULL, data);
            else
                hub->onBrowseItemClicked_gui(NULL, data);
        }
        else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
        {
            if (WGETB("pm"))
                hub->onBrowseItemClicked_gui(NULL, data);
            else
                hub->onMsgItemClicked_gui(NULL, data);
        }
        else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
        {
            hub->popupNickMenu_gui();
        }
    }

    return FALSE;
}

gboolean Hub::onNickListKeyRelease_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
        {
            hub->popupNickMenu_gui();
        }
        else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
        {
            hub->onBrowseItemClicked_gui(NULL, data);
        }
    }

    return FALSE;
}
/*
 * Implements a case-insensitive substring search for UTF-8 strings.
 */
gboolean Hub::onNickListSearch_gui(GtkTreeModel *model, gint column, const gchar *key, GtkTreeIter *iter, gpointer data)
{
    gboolean result = TRUE;
    gchar *nick;
    gtk_tree_model_get(model, iter, column, &nick, -1);

    gchar *keyCasefold = g_utf8_casefold(key, -1);
    gchar *nickCasefold = g_utf8_casefold(nick, -1);

    // Return false per search equal func API if the key is contained within the nick
    if (g_strstr_len(nickCasefold, -1, keyCasefold) != NULL)
        result = FALSE;

    g_free(nick);
    g_free(keyCasefold);
    g_free(nickCasefold);

    return result;
}

gboolean Hub::onEntryKeyPress_gui(GtkWidget *entry, GdkEventKey *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up)
    {
        size_t index = hub->historyIndex - 1;
        if (index >= 0 && index < hub->history.size())
        {
            hub->historyIndex = index;
            gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
        }
        return TRUE;
    }
    else if (event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
    {
        size_t index = hub->historyIndex + 1;
        if (index >= 0 && index < hub->history.size())
        {
            hub->historyIndex = index;
            gtk_entry_set_text(GTK_ENTRY(entry), hub->history[index].c_str());
        }
        return TRUE;
    }
    else if (event->keyval == GDK_Tab || event->keyval == GDK_ISO_Left_Tab)
    {
        string current;
        string::size_type start, end;
        string text(gtk_entry_get_text(GTK_ENTRY(entry)));
        int curpos = gtk_editable_get_position(GTK_EDITABLE(entry));

        // Allow tab to focus other widgets if entry is empty
        if (curpos <= 0 && text.empty())
            return FALSE;

        // Erase ": " at the end of the nick.
        if (curpos > 2 && text.substr(curpos - 2, 2) == ": ")
        {
            text.erase(curpos - 2, 2);
            curpos -= 2;
        }

        start = text.rfind(' ', curpos - 1);
        end = text.find(' ', curpos - 1);

        // Text to match starts at the beginning
        if (start == string::npos)
            start = 0;
        else
            ++start;

        if (start < end)
        {
            current = text.substr(start, end - start);

            if (hub->completionKey.empty() || Text::toLower(current).find(Text::toLower(hub->completionKey)) == string::npos)
                hub->completionKey = current;

            GtkTreeIter iter;
            bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(hub->nickStore), &iter);
            bool useNext = (current == hub->completionKey);
            string key = Text::toLower(hub->completionKey);
            string complete = hub->completionKey;

            while (valid)
            {
                string nick = hub->nickView.getString(&iter, _("Nick"));
                string::size_type tagEnd = 0;
                if (useNext && (tagEnd = Text::toLower(nick).find(key)) != string::npos)
                {
                    if (tagEnd == 0 || nick.find_first_of("]})", tagEnd - 1) == tagEnd - 1)
                    {
                        complete = nick;
                        if (start <= 0)
                            complete.append(": ");
                        break;
                    }
                }

                if (nick == current)
                    useNext = TRUE;

                valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(hub->nickStore),&iter);
            }

            text.replace(start, end - start, complete);
            gtk_entry_set_text(GTK_ENTRY(entry), text.c_str());
            gtk_editable_set_position(GTK_EDITABLE(entry), start + complete.length());
        }
        else
            hub->completionKey.clear();

        return TRUE;
    }

    hub->completionKey.clear();
    return FALSE;
}

gboolean Hub::onNickTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->type == GDK_2BUTTON_PRESS)
    {
        string tagName = tag->name;
        hub->nickToChat_gui(tagName.substr(tagPrefix.size()));

        return TRUE;
    }
    else if (event->type == GDK_BUTTON_PRESS)
    {
        GtkTreeIter nickIter;
        string tagName = tag->name;

        if (hub->findNick_gui(tagName.substr(tagPrefix.size()), &nickIter))
        {
            // Select the user in the nick list view
            GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(hub->nickStore), &nickIter);
            gtk_tree_view_scroll_to_cell(hub->nickView.get(), path, gtk_tree_view_get_column(hub->nickView.get(), hub->nickView.col(_("Nick"))), FALSE, 0.0, 0.0);
            gtk_tree_view_set_cursor(hub->nickView.get(), path, NULL, FALSE);
            gtk_tree_path_free(path);

            if (event->button.button == 3)
                hub->popupNickMenu_gui();
        }

        return TRUE;
    }

    return FALSE;
}

gboolean Hub::onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->type == GDK_BUTTON_PRESS)
    {
        switch (event->button.button)
        {
            case 1:
                onOpenLinkClicked_gui(NULL, data);
                break;
            case 3:
                // Popup uri context menu
                gtk_widget_show_all(hub->getWidget("linkMenu"));
                gtk_menu_popup(GTK_MENU(hub->getWidget("linkMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
                break;
        }
        return TRUE;
    }
    return FALSE;
}

gboolean Hub::onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->type == GDK_BUTTON_PRESS)
    {
        switch (event->button.button)
        {
            case 1:
                onOpenHubClicked_gui(NULL, data);
                break;
            case 3:
                // Popup uri context menu
                gtk_widget_show_all(hub->getWidget("hubMenu"));
                gtk_menu_popup(GTK_MENU(hub->getWidget("hubMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
                break;
        }
        return TRUE;
    }
    return FALSE;
}

gboolean Hub::onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->type == GDK_BUTTON_PRESS)
    {
        switch (event->button.button)
        {
            case 1:
                // Search for magnet
                WulforManager::get()->getMainWindow()->actionMagnet_gui(hub->selectedTagStr);
                break;
            case 3:
                // Popup magnet context menu
                gtk_widget_show_all(hub->getWidget("magnetMenu"));
                gtk_menu_popup(GTK_MENU(hub->getWidget("magnetMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
                break;
        }
        return TRUE;
    }
    return FALSE;
}

gboolean Hub::onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    hub->updateCursor_gui(widget);

    return FALSE;
}

gboolean Hub::onChatVisibilityChanged_gui(GtkWidget *widget, GdkEventVisibility *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    hub->updateCursor_gui(widget);

    return FALSE;
}

gboolean Hub::onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    switch (event->button)
    {
        case 1: //show emoticons dialog

            hub->emotdialog->showEmotDialog_gui();
        break;

        case 3: //show emoticons menu

            hub->emotdialog->buildEmotMenu_gui();

            GtkWidget *check_item = NULL;
            GtkWidget *emot_menu = hub->getWidget("emotPacksMenu");

            check_item = gtk_separator_menu_item_new();
            gtk_menu_shell_append(GTK_MENU_SHELL(emot_menu), check_item);
            gtk_widget_show(check_item);

            check_item = gtk_check_menu_item_new_with_label(_("Use Emoticons"));
            gtk_menu_shell_append(GTK_MENU_SHELL(emot_menu), check_item);

            if (hub->useEmoticons)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(check_item), TRUE);

            g_signal_connect(check_item, "activate", G_CALLBACK(onUseEmoticons_gui), data);

            gtk_widget_show_all(emot_menu);
            gtk_menu_popup(GTK_MENU(emot_menu), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
        break;
    }

    return FALSE;
}

void Hub::onChatScroll_gui(GtkAdjustment *adjustment, gpointer data)
{
    Hub *hub = (Hub *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);
    hub->scrollToBottom = value >= (adjustment->upper-adjustment->page_size);
}

void Hub::onChatResize_gui(GtkAdjustment *adjustment, gpointer data)
{
    Hub *hub = (Hub *)data;
    gdouble value = gtk_adjustment_get_value(adjustment);

    if (hub->scrollToBottom && value < (adjustment->upper-adjustment->page_size))
    {
        GtkTextIter iter;

        gtk_text_buffer_get_end_iter(hub->chatBuffer, &iter);
        gtk_text_buffer_move_mark(hub->chatBuffer, hub->chatMark, &iter);
        gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(hub->getWidget("chatText")), hub->chatMark, 0, FALSE, 0, 0);
    }
}

void Hub::onSendMessage_gui(GtkEntry *entry, gpointer data)
{
    string text = gtk_entry_get_text(entry);
    if (text.empty())
        return;

    gtk_entry_set_text(entry, "");
    Hub *hub = (Hub *)data;
    typedef Func1<Hub, string> F1;
    F1 *func;
    typedef Func2<Hub, string, bool> F2;
    F2 *func2;

    // Store line in chat history
    hub->history.pop_back();
    hub->history.push_back(text);
    hub->history.push_back("");
    hub->historyIndex = hub->history.size() - 1;
    if (hub->history.size() > maxHistory + 1)
        hub->history.erase(hub->history.begin());

    // Process special commands
    if (text[0] == '/')
    {
        string command, param;
        string::size_type separator = text.find_first_of(' ');
        if (separator != string::npos && text.size() > separator + 1)
        {
            command = text.substr(1, separator - 1);
            param = text.substr(separator + 1);
        }
        else
        {
            command = text.substr(1);
        }
        std::transform(command.begin(), command.end(), command.begin(), (int(*)(int))tolower);

        if (command == "away")
        {
            if (Util::getAway() && param.empty())
            {
                Util::setAway(FALSE);
                Util::setManualAway(FALSE);
                hub->addStatusMessage_gui(_("Away mode off"), Msg::SYSTEM, Sound::NONE);
            }
            else
            {
                Util::setAway(TRUE);
                Util::setManualAway(TRUE);
                Util::setAwayMessage(param);
                hub->addStatusMessage_gui(_("Away mode on: ") + Util::getAwayMessage(), Msg::SYSTEM, Sound::NONE);
            }
        }
        else if (command == "back")
        {
            Util::setAway(FALSE);
            hub->addStatusMessage_gui(_("Away mode off"), Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "clear")
        {
            GtkTextIter startIter, endIter;
            gtk_text_buffer_get_start_iter(hub->chatBuffer, &startIter);
            gtk_text_buffer_get_end_iter(hub->chatBuffer, &endIter);
            gtk_text_buffer_delete(hub->chatBuffer, &startIter, &endIter);
        }
        else if (command == "close")
        {
            /// @todo: figure out why this sometimes closes and reopens the tab
            WulforManager::get()->getMainWindow()->removeBookEntry_gui(hub);
        }
        else if (command == "favorite" || command == "fav")
        {
            WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::addAsFavorite_client));
        }
        else if (command == "fuser" || command == "fu")
        {
            if (hub->client->getMyNick() == param)
                return;

            if (hub->userMap.find(param) != hub->userMap.end())
            {
                string &cid = hub->userMap[param];
                if (hub->userFavoriteMap.find(cid) == hub->userFavoriteMap.end())
                {
                    Func1<Hub, string> *func = new Func1<Hub, string>(hub, &Hub::addFavoriteUser_client, cid);
                    WulforManager::get()->dispatchClientFunc(func);
                } else
                    hub->addStatusMessage_gui(param + _(" is favorite user"), Msg::STATUS, Sound::NONE);
            } else
                hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "removefu" || command == "rmfu")
        {
            if (hub->client->getMyNick() == param)
                return;

            UserMap::const_iterator it = find_if(hub->userFavoriteMap.begin(), hub->userFavoriteMap.end(),
                CompareSecond<string, string>(param));

            if (it != hub->userFavoriteMap.end())
            {
                Func1<Hub, string> *func = new Func1<Hub, string>(hub, &Hub::removeFavoriteUser_client, it->first);
                WulforManager::get()->dispatchClientFunc(func);
            }
            else
                hub->addStatusMessage_gui(param + _(" is not favorite user"), Msg::STATUS, Sound::NONE);
        }
        else if (command == "listfu" || command == "lsfu")
        {
            string list;
            for (UserMap::const_iterator it = hub->userFavoriteMap.begin(); it != hub->userFavoriteMap.end(); ++it)
            {
                list += " " + it->second;
            }
            hub->addMessage_gui("", _("User favorite list:") + (list.empty()? list = _(" empty...") : list), Msg::SYSTEM);
        }
        else if (command == "getlist")
        {
            if (hub->userMap.find(param) != hub->userMap.end())
            {
                func2 = new F2(hub, &Hub::getFileList_client, hub->userMap[param], FALSE);
                WulforManager::get()->dispatchClientFunc(func2);
            }
            else
                hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "grant")
        {
            if (hub->userMap.find(param) != hub->userMap.end())
            {
                func = new F1(hub, &Hub::grantSlot_client, hub->userMap[param]);
                WulforManager::get()->dispatchClientFunc(func);
            }
            else
                hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "emoticons" || command == "emot")
        {
            if (hub->useEmoticons)
            {
                hub->useEmoticons = FALSE;
                hub->addStatusMessage_gui(_("Emoticons mode off"), Msg::SYSTEM, Sound::NONE);
            }
            else
            {
                hub->useEmoticons = TRUE;
                hub->addStatusMessage_gui(_("Emoticons mode on"), Msg::SYSTEM, Sound::NONE);
            }
        }
        else if (command == "limitimg" || command == "limg")
        {
            int n;
            string text;
            if (param.empty())
            {
                n = hub->ImgLimit;
                if (n == 0)
                    text = _("Download image: disable");
                else if (n < 0)
                    text = _("Download image: unlimit");
                else
                    text = _("Download limit image: ") + Util::toString(n);
                hub->addMessage_gui("", text.c_str(), Msg::SYSTEM);
                return;
            }
            n = Util::toInt(param);
            hub->ImgLimit = n;
            text = _("Set download limit image: ") + Util::toString(n);
            hub->addStatusMessage_gui(text, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "version")
        {
            hub->addStatusMessage_gui(string(EISKALTDCPP_WND_TITLE)+" "+string(EISKALTDCPP_VERSION)+" ("+string(EISKALTDCPP_VERSION_SFX)+"), "+_("project home: ")+"http://code.google.com/p/eiskaltdc/", Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "help")
        {
            hub->addMessage_gui("", string(_("*** Available commands:")) + "\n\n" +
            "/away <message>\t\t - " + _("Away mode message on/off") + "\n" +
            "/back\t\t\t\t - " + _("Away mode off") + "\n" +
            "/clear\t\t\t\t - " + _("Clear chat") + "\n" +
            "/close\t\t\t\t - " + _("Close chat") + "\n" +
            "/favorite, /fav\t\t\t - " + _("Add a hub to favorites") + "\n" +
            "/fuser, /fu <nick>\t\t - " + _("Add user to favorites list") + "\n" +
            "/removefu, /rmfu <nick>\t - " + _("Remove user favorite") + "\n" +
            "/listfu, /lsfu\t\t\t - " + _("Show favorites list") + "\n" +
            "/getlist <nick>\t\t\t - " + _("Get file list") + "\n" +
            "/grant <nick>\t\t\t - " + _("Grant extra slot") + "\n" +
            "/help\t\t\t\t - " + _("Show help") + "\n" +
            "/join <address>\t\t - " + _("Connect to the hub") + "\n" +
            "/me <message>\t\t - " + _("Say a third person") + "\n" +
            "/pm <nick>\t\t\t - " + _("Private message") + "\n" +
            "/rebuild\t\t\t\t - " + _("Rebuild hash") + "\n" +
            "/refresh\t\t\t\t - " + _("Update own file list") + "\n" +
            "/userlist\t\t\t\t - " + _("User list show/hide") + "\n" +
            "/limitimg <n>, limg <n>\t - " + _("Download limit image: 0 - disable, n < 0 - unlimit, empty - info") + "\n" +
            "/version\t\t\t\t - " + _("Show version") + "\n" +
            "/emoticons, /emot\t\t - " + _("Emoticons on/off") + "\n" +
#ifdef LUA_SCRIPT
            "/luafile <file>\t\t\t - " + _("Load Lua file") + "\n" +
            "/lua <chunk>\t\t\t\t -  " + _("Execute Lua Chunk") + "\n"+
#endif
            "/sh\t\t\t\t\t - " +  _("Execute code (bash)") +"\n"+
            "/alias list\t\t\t\t - " + _("Alias List")+ "\n"+
            "/alias purge A\t\t\t - "+ _("Alias Remove A")+"\n"+
            "/alias A::uname -a\t\t - " +  _("Alias add uname -a as A")+"\n" +
            "/A\t\t\t\t\t - " + _("Alias A executing")+"\n"
            , Msg::SYSTEM);
        }
        else if (command == "ws" && !param.empty())
        {
            string msg = WSCMD(param);
            hub->addStatusMessage_gui(msg, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "dcpps" && !param.empty())
        {
            string msg = WulforSettingsManager::getInstance()->parseCoreCmd (param);
            hub->addStatusMessage_gui(msg, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "join" && !param.empty())
        {
            if (WGETB("join-open-new-window"))
            {
                // Assumption: new hub is same encoding as current hub.
                WulforManager::get()->getMainWindow()->showHub_gui(param, hub->encoding);
            }
            else
            {
                typedef Func2<Hub, string, bool> F2;
                F2 *func = new F2(hub, &Hub::redirect_client, param, TRUE);
                WulforManager::get()->dispatchClientFunc(func);
            }
        }
        else if (command == "me")
        {
            func2 = new F2(hub, &Hub::sendMessage_client, param, true);
            WulforManager::get()->dispatchClientFunc(func2);
        }
        else if (command == "pm")
        {
            if (hub->userMap.find(param) != hub->userMap.end())
                WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, hub->userMap[param], hub->client->getHubUrl());
            else
                hub->addStatusMessage_gui(_("Not found user: ") + param, Msg::SYSTEM, Sound::NONE);
        }
        else if (command == "rebuild")
        {
            WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::rebuildHashData_client));
        }
        else if (command == "refresh")
        {
            WulforManager::get()->dispatchClientFunc(new Func0<Hub>(hub, &Hub::refreshFileList_client));
        }
        else if (command == "userlist")
        {
            if (GTK_WIDGET_VISIBLE(hub->getWidget("scrolledwindow2")))
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), FALSE);
            else
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(hub->getWidget("userListCheckButton")), TRUE);
        }
        else if (command == "sh")
        {
                        FILE *pipe = popen( param.c_str(), "r" );
                        gchar *command_res;
                        gsize command_length;
                        GIOChannel* gio_chanel = g_io_channel_unix_new( fileno( pipe ) );
                        GIOStatus gio_status = g_io_channel_read_to_end( gio_chanel, &command_res, &command_length, NULL );
                        if( gio_status == G_IO_STATUS_NORMAL )
                        {
                                F2 *func = new F2( hub, &Hub::sendMessage_client, string(command_res), false );
                                WulforManager::get()->dispatchClientFunc(func);
                        }
                        g_io_channel_close( gio_chanel );
                        g_free( command_res );
                        pclose( pipe );
        }
#ifdef LUA_SCRIPT
        else if (command == "lua" ) {
            ScriptManager::getInstance()->EvaluateChunk(Text::fromT(param));
        }
        else if( command == "luafile") {
            ScriptManager::getInstance()->EvaluateFile(Text::fromT(param));
        }
#endif
        //alias patch
        else if (command == "alias" && !param.empty())
        {
            StringTokenizer<string> sl(param, ' ');
            if( sl.getTokens().size() >= 1 )
            {
                StringTokenizer<string> aliases( WGETS("custom-aliases"), '#' );

                    if( sl.getTokens().at(0) == "list" )
                    {
                        // вывод списка алиасов
                        /// List aliasu
                            if( !aliases.getTokens().empty() ) {
                                hub->addMessage_gui("", string( "Alias list:" ), Msg::SYSTEM);

                                for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i) {
                                    hub->addMessage_gui("", *i, Msg::SYSTEM);
                                }
                            }
                            else
                            {
                                hub->addStatusMessage_gui(_("Aliases not found."), Msg::SYSTEM, Sound::NONE);
                            }
                    }
                    else if( sl.getTokens().at(0) == "purge" )
                    {
                        // удаление алиаса из списка
                        ///odstraneni aliasu
                            string store(""), name("");
                                for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i) {
                                    name = i->substr( 0, i->find_first_of( "::", 0 ) );
                                    if( name.compare( sl.getTokens().at(1) ) != 0 )
                                        store = store + *i + "#";
                                    }
                                    WSET( "custom-aliases", store );

                    }
                    else
                    {
                        // добавление алиаса к списку
                        StringTokenizer<string> command( param, "::" );
                        string store(""), name("");
                        bool exists = false;
                        for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
                        {
                            name = i->substr( 0, i->find_first_of( "::", 0 ) );
                            if( name.compare( param.substr( 0, param.find_first_of( "::", 0 ) ) ) == 0 )
                            {
                                exists = true;
                                hub->addMessage_gui("", string( "This alias already exists: " + *i ), Msg::SYSTEM);
                                break;
                            }
                        }
                        if( command.getTokens().size() == 3  && !exists )
                        {
                            aliases.getTokens().push_back( param );
                            string store("");
                            for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
                            {
                                store = store + *i + "#";
                            }
                            WSET( "custom-aliases", store );
                        }
                    }
            }
        }
        else if ( !WGETS("custom-aliases").empty() )
        {
            // поиск алиаса в списке
            StringTokenizer<string> aliases( WGETS("custom-aliases"), '#' );
                string name("");
                for(StringIter i = aliases.getTokens().begin(); i != aliases.getTokens().end(); ++i)
                {
                        name = i->substr( 0, i->find_first_of( "::", 0 ) );
                        if( name.compare( command ) == 0 )
                        {
                            string exec = i->substr( i->find_first_of( "::", 0 ) + 2, i->size() );

                            if( !exec.empty() )
                            {
                                gchar *output = NULL;
                                GError *error = NULL;

                                g_spawn_command_line_sync( exec.c_str(), &output, NULL, NULL, &error);

                                if (error != NULL)
                                {
                                    //TODO: вывод ошибки на GUI
                                    printf("ERROR\n");
                                    g_error_free(error);
                                }
                                else
                                {
                                    string trash( output );
                                    g_free( output );

                                    func2 = new F2(hub, &Hub::sendMessage_client, trash, false );
                                                        WulforManager::get()->dispatchClientFunc( func2 );
                                }
                            }
                                break;
                        }

                }
        }
        // protect command
        else if (command == "password")
        {
            if (!hub->WaitingPassword)
                return;

            F1 *func = new F1(hub, &Hub::setPassword_client, param);
            WulforManager::get()->dispatchClientFunc(func);
            hub->WaitingPassword = FALSE;
        }
        else if (BOOLSETTING(SEND_UNKNOWN_COMMANDS))
        {
            func2 = new F2(hub, &Hub::sendMessage_client, text, false);
            WulforManager::get()->dispatchClientFunc(func2);
        }
        else
        {
            hub->addStatusMessage_gui(_("Unknown command '") + text + _("': type /help for a list of available commands"), Msg::SYSTEM, Sound::NONE);
        }
        #ifdef LUA_SCRIPT
            func2 = new F2(hub, &Hub::sendMessage_client, text, false);
            WulforManager::get()->dispatchClientFunc(func2);
        #endif
    }
    else
    {
        func2 = new F2(hub, &Hub::sendMessage_client, text, false);
        WulforManager::get()->dispatchClientFunc(func2);
    }
}

void Hub::onNickToChat_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string nicks;
        GtkTreeIter iter;
        GtkTreePath *path;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
                nicks += hub->nickView.getString(&iter, _("Nick")) + ", ";

            gtk_tree_path_free(path);
        }

        g_list_free(list);

        if (!nicks.empty())
        {
            nicks.erase(nicks.size() - 2);
            hub->nickToChat_gui(nicks);
        }
    }
}

void Hub::onCopyNickItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string nicks;
        GtkTreeIter iter;
        GtkTreePath *path;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                nicks += hub->nickView.getString(&iter, _("Nick")) + ' ';
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);

        if (!nicks.empty())
        {
            nicks.erase(nicks.length() - 1);
            gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), nicks.c_str(), nicks.length());
        }
    }
}

void Hub::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid;
        GtkTreeIter iter;
        GtkTreePath *path;
        typedef Func2<Hub, string, bool> F2;
        F2 *func;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                func = new F2(hub, &Hub::getFileList_client, cid, FALSE);
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::onMatchItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid;
        GtkTreeIter iter;
        GtkTreePath *path;
        typedef Func2<Hub, string, bool> F2;
        F2 *func;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                func = new F2(hub, &Hub::getFileList_client, cid, TRUE);
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::onMsgItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid;
        GtkTreeIter iter;
        GtkTreePath *path;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);
        const string &hubUrl = hub->client->getHubUrl();

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, cid, hubUrl);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::onGrantItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid;
        GtkTreeIter iter;
        GtkTreePath *path;
        typedef Func1<Hub, string> F1;
        F1 *func;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                func = new F1(hub, &Hub::grantSlot_client, cid);
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::onRemoveUserItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid;
        GtkTreeIter iter;
        GtkTreePath *path;
        typedef Func1<Hub, string> F1;
        F1 *func;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;
            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                func = new F1(hub, &Hub::removeUserFromQueue_client, cid);
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::onCopyURIClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), hub->selectedTagStr.c_str(), hub->selectedTagStr.length());
}

void Hub::onOpenLinkClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    WulforUtil::openURI(hub->selectedTagStr);
}

void Hub::onOpenHubClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    WulforManager::get()->getMainWindow()->showHub_gui(hub->selectedTagStr);
}

void Hub::onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    WulforManager::get()->getMainWindow()->addSearch_gui(hub->selectedTagStr);
}

void Hub::onDownloadClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;
    WulforManager::get()->getMainWindow()->fileToDownload_gui(hub->selectedTagStr, SETTING(DOWNLOAD_DIRECTORY));
}

gboolean Hub::onChatCommandButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (event->button == 1)
    {
        gtk_menu_popup(GTK_MENU(hub->getWidget("chatCommandsMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
    }

    return FALSE;
}

void Hub::onCommandClicked_gui(GtkWidget *widget, gpointer data)
{
    Hub *hub = (Hub *)data;

    string command = (gchar*) g_object_get_data(G_OBJECT(widget), "command");

    gint pos = 0;
    GtkWidget *chatEntry = hub->getWidget("chatEntry");
    if (!gtk_widget_is_focus(chatEntry))
        gtk_widget_grab_focus(chatEntry);
    gtk_editable_delete_text(GTK_EDITABLE(chatEntry), pos, -1);
    gtk_editable_insert_text(GTK_EDITABLE(chatEntry), command.c_str(), -1, &pos);
    gtk_editable_set_position(GTK_EDITABLE(chatEntry), pos);
}

void Hub::onUseEmoticons_gui(GtkWidget *widget, gpointer data)
{
    Hub *hub = (Hub *)data;

    hub->useEmoticons = !hub->useEmoticons;
}

void Hub::onDownloadToClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    GtkWidget *dialog = WulforManager::get()->getMainWindow()->getChooserDialog_gui();
    gtk_window_set_title(GTK_WINDOW(dialog), _("Choose a directory"));
    gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    // if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    if (response == GTK_RESPONSE_OK)
    {
        gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));

        if (temp)
        {
            string path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
            g_free(temp);

            WulforManager::get()->getMainWindow()->fileToDownload_gui(hub->selectedTagStr, path);
        }
    }
    gtk_widget_hide(dialog);
}

void Hub::onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    WulforManager::get()->getMainWindow()->propertiesMagnetDialog_gui(hub->selectedTagStr);
}

void Hub::onUserListToggled_gui(GtkWidget *widget, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (GTK_WIDGET_VISIBLE(hub->getWidget("scrolledwindow2")))
        gtk_widget_hide(hub->getWidget("scrolledwindow2"));
    else
        gtk_widget_show_all(hub->getWidget("scrolledwindow2"));
}

void Hub::onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid, nick, order;
        GtkTreeIter iter;
        GtkTreePath *path;
        typedef Func1<Hub, string> F1;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                nick = hub->nickView.getString(&iter, _("Nick"));
                order = hub->nickView.getString(&iter, "Favorite");

                if (!cid.empty() && nick != hub->client->getMyNick())
                {
                    if (order[0] == 'o' || order[0] == 'u')
                    {
                        F1 *func = new F1(hub, &Hub::addFavoriteUser_client, cid);
                        WulforManager::get()->dispatchClientFunc(func);
                    }
                    else
                        hub->addStatusMessage_gui(nick + _(" is favorite user"), Msg::STATUS, Sound::NONE);
                }
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::onRemoveFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub *)data;

    if (gtk_tree_selection_count_selected_rows(hub->nickSelection) > 0)
    {
        string cid, nick, order;
        GtkTreeIter iter;
        GtkTreePath *path;
        typedef Func1<Hub, string> F1;
        GList *list = gtk_tree_selection_get_selected_rows(hub->nickSelection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(hub->nickStore), &iter, path))
            {
                cid = hub->nickView.getString(&iter, "CID");
                nick = hub->nickView.getString(&iter, _("Nick"));
                order = hub->nickView.getString(&iter, "Favorite");

                if (!cid.empty() && nick != hub->client->getMyNick())
                {
                    if (order[0] == 'f')
                    {
                        F1 *func = new F1(hub, &Hub::removeFavoriteUser_client, cid);
                        WulforManager::get()->dispatchClientFunc(func);
                    }
                    else
                        hub->addStatusMessage_gui(nick + _(" is not favorite user"), Msg::STATUS, Sound::NONE);
                }
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void Hub::addFavoriteUser_gui(ParamMap params)
{
    const string &cid = params["CID"];

    if (userFavoriteMap.find(cid) == userFavoriteMap.end())
    {
        GtkTreeIter iter;
        const string &nick = params["Nick"];
        userFavoriteMap.insert(UserMap::value_type(cid, nick));

        // resort users
        if (findUser_gui(cid, &iter))
        {
            gtk_list_store_set(nickStore, &iter,
                nickView.col("Favorite"), ("f" + params["Order"] + nick).c_str(),
                nickView.col("NickColor"), "#ff0000",
                -1);
            removeTag_gui(nick);
        }

        string message = nick + _(" added to favorites list");
        addStatusMessage_gui(message, Msg::STATUS, Sound::NONE);
        WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
    }
}

void Hub::removeFavoriteUser_gui(ParamMap params)
{
    const string &cid = params["CID"];

    if (userFavoriteMap.find(cid) != userFavoriteMap.end())
    {
        GtkTreeIter iter;
        const string &nick = params["Nick"];
        userFavoriteMap.erase(cid);

        // resort users
        if (findUser_gui(cid, &iter))
        {
            string nickOrder = nickView.getString(&iter, "Nick Order");
            gtk_list_store_set(nickStore, &iter,
                nickView.col("Favorite"), nickOrder.c_str(),
                nickView.col("NickColor"), "#000000",
                -1);
            removeTag_gui(nick);
        }

        string message = nick + _(" removed from favorites list");
        addStatusMessage_gui(message, Msg::STATUS, Sound::NONE);
        WulforManager::get()->getMainWindow()->addPrivateStatusMessage_gui(Msg::STATUS, cid, message);
    }
}

void Hub::addPrivateMessage_gui(Msg::TypeMsg typemsg, string CID, string cid, string url, string message, bool useSetting)
{
    if (userFavoriteMap.find(CID) != userFavoriteMap.end())
        typemsg = Msg::FAVORITE;

    WulforManager::get()->getMainWindow()->addPrivateMessage_gui(typemsg, cid, url, message, useSetting);
}

void Hub::addFavoriteUser_client(const string cid)
{
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user)
    {
        FavoriteManager::getInstance()->addFavoriteUser(user);
    }
}

void Hub::removeFavoriteUser_client(const string cid)
{
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user)
    {
        FavoriteManager::getInstance()->removeFavoriteUser(user);
    }
}

void Hub::connectClient_client(string address, string encoding)
{
    dcassert(client == NULL);

    if (address.substr(0, 6) == "adc://" || address.substr(0, 7) == "adcs://")
        encoding = "UTF-8";
    else if (encoding.empty() || encoding == _("Global hub default")) // latter for 1.0.3 backwards compatability
        encoding = WGETS("default-charset");

    if (encoding == WulforUtil::ENCODING_LOCALE)
        encoding = Text::systemCharset;

    // Only pick "UTF-8" part of "UTF-8 (Unicode)".
    string::size_type i = encoding.find(' ', 0);
    if (i != string::npos)
        encoding = encoding.substr(0, i);

    client = ClientManager::getInstance()->getClient(address);
    client->setEncoding(encoding);
    client->addListener(this);
    client->connect();
    FavoriteManager::getInstance()->addListener(this);
    QueueManager::getInstance()->addListener(this);
}

void Hub::disconnect_client()
{
    if (client)
    {
        FavoriteManager::getInstance()->removeListener(this);
        QueueManager::getInstance()->removeListener(this);
        client->removeListener(this);
        client->disconnect(TRUE);
        ClientManager::getInstance()->putClient(client);
        client = NULL;
    }
}

void Hub::setPassword_client(string password)
{
    if (client && !password.empty())
    {
        client->setPassword(password);
        client->password(password);
    }
}

void Hub::sendMessage_client(string message, bool thirdPerson)
{
    if (client && !message.empty())
        client->hubMessage(message, thirdPerson);
}

void Hub::getFileList_client(string cid, bool match)
{
    string message;

    if (!cid.empty())
    {
        try
        {
            UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
            if (user)
            {
                const HintedUser hintedUser(user, client->getHubUrl());//NOTE: core 0.762
                if (user == ClientManager::getInstance()->getMe())
                {
                    // Don't download file list, open locally instead
                    WulforManager::get()->getMainWindow()->openOwnList_client(TRUE);
                }
                else if (match)
                {
                    QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);//NOTE: core 0.762
                }
                else
                {
                    QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
                }
            }
            else
            {
                message = _("User not found");
            }
        }
        catch (const Exception &e)
        {
            message = e.getError();
            LogManager::getInstance()->message(message);
        }
    }

    if (!message.empty())
    {
        typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
        F3 *func = new F3(this, &Hub::addStatusMessage_gui, message, Msg::SYSTEM, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void Hub::grantSlot_client(string cid)
{
    string message = _("User not found");

    if (!cid.empty())
    {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
        if (user)
        {
            const string hubUrl = client->getHubUrl();//NOTE: core 0.762
            UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));//NOTE: core 0.762
            message = _("Slot granted to ") + WulforUtil::getNicks(user, hubUrl);//NOTE: core 0.762
        }
    }

    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
    F3 *func = new F3(this, &Hub::addStatusMessage_gui, message, Msg::STATUS, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::removeUserFromQueue_client(string cid)
{
    if (!cid.empty())
    {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
        if (user)
            QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
    }
}

void Hub::redirect_client(string address, bool follow)
{
    if (!address.empty())
    {
        if (ClientManager::getInstance()->isConnected(address))
        {
            string error = _("Unable to connect: already connected to the requested hub");
            typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
            F3 *f3 = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
            WulforManager::get()->dispatchGuiFunc(f3);
            return;
        }

        if (follow)
        {
            // the client is dead, long live the client!
            disconnect_client();

            Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
            WulforManager::get()->dispatchGuiFunc(func);

            connectClient_client(address, encoding);
        }
    }
}

void Hub::rebuildHashData_client()
{
    HashManager::getInstance()->rebuild();
}

void Hub::refreshFileList_client()
{
    try
    {
        ShareManager::getInstance()->setDirty();
        ShareManager::getInstance()->refresh(true);
    }
    catch (const ShareException& e)
    {
    }
}

void Hub::addAsFavorite_client()
{
    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
    F3 *func;

    FavoriteHubEntry *existingHub = FavoriteManager::getInstance()->getFavoriteHubEntry(client->getHubUrl());

    if (!existingHub)
    {
        FavoriteHubEntry aEntry;
        aEntry.setServer(client->getHubUrl());
        aEntry.setName(client->getHubName());
        aEntry.setDescription(client->getHubDescription());
        aEntry.setConnect(FALSE);
        aEntry.setNick(client->getMyNick());
        aEntry.setEncoding(encoding);
        FavoriteManager::getInstance()->addFavorite(aEntry);
        func = new F3(this, &Hub::addStatusMessage_gui, _("Favorite hub added"), Msg::STATUS, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(func);
    }
    else
    {
        func = new F3(this, &Hub::addStatusMessage_gui, _("Favorite hub already exists"), Msg::STATUS, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void Hub::reconnect_client()
{
    Func0<Hub> *func = new Func0<Hub>(this, &Hub::clearNickList_gui);
    WulforManager::get()->dispatchGuiFunc(func);

    if (client)
        client->reconnect();
}

void Hub::getParams_client(ParamMap &params, Identity &id)
{
    //if (id.getUser()->isSet(User::DCPLUSPLUS))
    if(id.supports(AdcHub::ADCS_FEATURE) && id.supports(AdcHub::SEGA_FEATURE) &&
    ((id.supports(AdcHub::TCP4_FEATURE) && id.supports(AdcHub::UDP4_FEATURE)) || id.supports(AdcHub::NAT0_FEATURE)))
        params.insert(ParamMap::value_type("Icon", "dc++"));
    else
        params.insert(ParamMap::value_type("Icon", "normal"));

    if (id.getUser()->isSet(User::PASSIVE))
        params["Icon"] += "-fw";

    if (id.isOp())
    {
        params["Icon"] += "-op";
        params.insert(ParamMap::value_type("Nick Order", "o" + id.getNick()));
    }
    else
    {
        params.insert(ParamMap::value_type("Nick Order", "u" + id.getNick()));
    }

    params.insert(ParamMap::value_type("Nick", id.getNick()));
    params.insert(ParamMap::value_type("Shared", Util::toString(id.getBytesShared())));
    params.insert(ParamMap::value_type("Description", id.getDescription()));
    params.insert(ParamMap::value_type("Tag", id.getTag()));
    params.insert(ParamMap::value_type("Connection", id.getConnection()));
    params.insert(ParamMap::value_type("IP", id.getIp()));
    params.insert(ParamMap::value_type("eMail", id.getEmail()));
    params.insert(ParamMap::value_type("CID", id.getUser()->getCID().toBase32()));
}

void Hub::download_client(string target, int64_t size, string tth, string cid)
{
   string real = realFile_client(tth);
   if (!real.empty())
   {
       typedef Func2<Hub, string, string> F2;
       F2 *f2 = new F2(this, &Hub::loadImage_gui, real, tth);
       WulforManager::get()->dispatchGuiFunc(f2);

       return;
   }

   try
   {
       UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
       if (user == NULL)
           return;

       string hubUrl = client->getHubUrl();
       QueueManager::getInstance()->add(target, size, TTHValue(tth));
   }
   catch (const Exception&)
   {
       typedef Func2<Hub, string, string> F2;
       F2 *f2 = new F2(this, &Hub::loadImage_gui, target, tth);
       WulforManager::get()->dispatchGuiFunc(f2);
   }
}

string Hub::realFile_client(string tth)
{
   try
   {
       string virt = ShareManager::getInstance()->toVirtual(TTHValue(tth));
       string real = ShareManager::getInstance()->toReal(virt);

       return real;
   }
   catch (const Exception&)
   {
   }
   return "";
}

void Hub::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw()
{
   typedef Func2<Hub, string, string> F2;
   string tth = item->getTTH().toBase32();

   if (!item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST) && !item->isSet(QueueItem::FLAG_XML_BZLIST))
   {
       F2 *f2 = new F2(this, &Hub::loadImage_gui, item->getTarget(), tth);
       WulforManager::get()->dispatchGuiFunc(f2);
   }
}

void Hub::loadImage_gui(string target, string tth)
{
   if (imageLoad.first != tth)
       return;

   if (imageLoad.second == NULL)
       return;

   if (!g_file_test(target.c_str(), G_FILE_TEST_EXISTS))
   {
       string text = _("loading error");
#if GTK_CHECK_VERSION(2, 12, 0)
       gtk_widget_set_tooltip_text(imageLoad.second, text.c_str());
#else
       gtk_tooltips_set_tip(tips, imageLoad.second, text.c_str(), text.c_str());
#endif
       return;
   }

   GdkPixbuf *source = gdk_pixbuf_new_from_file(target.c_str(), NULL);

   if (!source)
   {
       string text = _("bad image");
#if GTK_CHECK_VERSION(2, 12, 0)
       gtk_widget_set_tooltip_text(imageLoad.second, text.c_str());
#else
       gtk_tooltips_set_tip(tips, imageLoad.second, text.c_str(), text.c_str());
#endif
       return;
   }

   int width = gdk_pixbuf_get_width(source);
   int height = gdk_pixbuf_get_height(source);
   GdkPixbuf *pixbuf = NULL;
   int set_w = 200, set_h = 200;
   double w, h, k;
   w = (double) set_w / width;
   h = (double) set_h / height ;
   k = MIN(w, h);

   if (k >= 1)
       pixbuf = source;
   else
   {
       pixbuf = WulforUtil::scalePixbuf(source, set_w, set_h);
       g_object_unref(source);
   }
   gtk_image_set_from_pixbuf(GTK_IMAGE(imageLoad.second), pixbuf);
   g_object_unref(pixbuf);

   // reset tips
   string name, magnet = imageMagnet.first;
   int64_t size;
   WulforUtil::splitMagnet(magnet, name, size, tth);
   string text = "name: " + name + "\n" + "size: " + Util::formatBytes(size);
#if GTK_CHECK_VERSION(2, 12, 0)
   gtk_widget_set_tooltip_text(imageLoad.second, text.c_str());
#else
   gtk_tooltips_set_tip(tips, imageLoad.second, text.c_str(), t.c_str());
#endif
   imageLoad.first = "";
   imageLoad.second = NULL;
}

void Hub::onImageDestroy_gui(GtkWidget *widget, gpointer data)
{
   Hub *hub = (Hub*) data;

   // fix crash, if entry delete...
   if (!WulforManager::get()->isEntry_gui(hub))
       return;

   ImageList::const_iterator j = hub->imageList.find(widget);

   if (j != hub->imageList.end())
   {
       // fix crash, if image menu active...
       string k = hub->imageMagnet.first;
       string l = j->second;
       string name, tth;
       int64_t size;

       WulforUtil::splitMagnet(k, name, size, tth);

       if (l == tth)
       {
           g_object_set_data(G_OBJECT(hub->getWidget("downloadImageItem")), "container", NULL);
           g_object_set_data(G_OBJECT(hub->getWidget("removeImageItem")), "container", NULL);
       }

       // erase image...
       hub->imageList.erase(j);
   }

   // fix crash...
   if (hub->imageLoad.second == widget)
   {
       hub->imageLoad.first = "";
       hub->imageLoad.second = NULL;
   }
}

gboolean Hub::onImageEvent_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
   Hub *hub = (Hub*) data;

   if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
   {
       hub->imageMagnet.first = (gchar*) g_object_get_data(G_OBJECT(widget), "magnet");
       hub->imageMagnet.second = (gchar*) g_object_get_data(G_OBJECT(widget), "cid");
       g_object_set_data(G_OBJECT(hub->getWidget("removeImageItem")), "container", (gpointer)widget);
       g_object_set_data(G_OBJECT(hub->getWidget("downloadImageItem")), "container", (gpointer)widget);
       gtk_menu_popup(GTK_MENU(hub->getWidget("imageMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
   }
   return false;
}

void Hub::onDownloadImageClicked_gui(GtkMenuItem *item, gpointer data)
{
   Hub *hub = (Hub*) data;

   string name, tth, target;
   int64_t size;
   const string magnet = hub->imageMagnet.first;
   const string cid = hub->imageMagnet.second;

   if (WulforUtil::splitMagnet(magnet, name, size, tth))
   {
       GtkWidget *container = (GtkWidget*) g_object_get_data(G_OBJECT(item), "container");

       // if image destroy
       if (container == NULL)
           return;

       GList *childs = gtk_container_get_children(GTK_CONTAINER(container));
       hub->imageLoad.first = tth;
       hub->imageLoad.second = (GtkWidget*)childs->data;
       g_list_free(childs);

       target = Util::getPath(Util::PATH_USER_CONFIG) + "Images" + PATH_SEPARATOR_STR + tth;
       typedef Func4<Hub, string, int64_t, string, string> F4;
       F4 *func = new F4(hub, &Hub::download_client, target, size, tth, cid);
       WulforManager::get()->dispatchClientFunc(func);
   }
}

void Hub::onRemoveImageClicked_gui(GtkMenuItem *item, gpointer data)
{
   Hub *hub = (Hub*) data;

   GtkWidget *container = (GtkWidget*) g_object_get_data(G_OBJECT(item), "container");

   // if image destroy
   if (container == NULL)
       return;

   GList *childs = gtk_container_get_children(GTK_CONTAINER(container));
   GtkWidget *image = (GtkWidget*)childs->data;
   g_list_free(childs);
   gtk_image_set_from_stock(GTK_IMAGE(image), GTK_STOCK_FILE, GTK_ICON_SIZE_BUTTON);

   hub->imageLoad.first = "";
   hub->imageLoad.second = NULL;
}

void Hub::onOpenImageClicked_gui(GtkMenuItem *item, gpointer data)
{
    Hub *hub = (Hub*) data;

    int64_t size;
    string name, tth;
    const string magnet = hub->imageMagnet.first;
    WulforUtil::splitMagnet(magnet, name, size, tth);
    typedef Func1<Hub, string> F1;
    F1 *f = new F1(hub, &Hub::openImage_client, tth);
    WulforManager::get()->dispatchClientFunc(f);
}

void Hub::openImage_client(string tth)
{
    string real = realFile_client(tth);
    typedef Func1<Hub, string> F1;
    F1 *f = new F1(this, &Hub::openImage_gui, real.empty() ? tth : real);
    WulforManager::get()->dispatchGuiFunc(f);
}

void Hub::openImage_gui(string target)
{
    if (!File::isAbsolute(target))
       target = Util::getPath(Util::PATH_USER_CONFIG) + "Images" + PATH_SEPARATOR_STR + target;
    WulforUtil::openURI(target);
}

gboolean Hub::expose(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GTK_WIDGET_CLASS(GTK_WIDGET_GET_CLASS(widget))->expose_event(widget, event);
    return true;
}

void Hub::onItalicButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    Hub *hub = (Hub*) data;

    hub->insertBBcodeEntry_gui("i");
}

void Hub::onBoldButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    Hub *hub = (Hub*) data;

    hub->insertBBcodeEntry_gui("b");
}

void Hub::onUnderlineButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    Hub *hub = (Hub*) data;

    hub->insertBBcodeEntry_gui("u");
}

void Hub::insertBBcodeEntry_gui(string ch)
{
    gint start_pos;
    gint end_pos;
    GtkEditable *chatEntry = GTK_EDITABLE(getWidget("chatEntry"));
    if (gtk_editable_get_selection_bounds(chatEntry, &start_pos, &end_pos))
    {
        gchar *tmp = gtk_editable_get_chars(chatEntry, start_pos, end_pos);
        string text = tmp;
        g_free(tmp);
        string::size_type a = 0, b = 0;
        string res;

        for (;;)
        {
            a = text.find_first_not_of("\r\n\t ", b);

            if (a != string::npos)
            {
                b = text.find_first_of("\r\n\t ", a);

                if (b != string::npos)
                {
                    res += "[" + ch + "]" + text.substr(a, b - a) + "[/" + ch + "] ";
                }
                else
                {
                    res += "[" + ch + "]" + text.substr(a) + "[/" + ch + "]";
                    break;
                }
            }
            else
                break;
        }

        gtk_editable_delete_text(chatEntry, start_pos, end_pos);
        gtk_editable_insert_text(chatEntry, res.c_str(), -1, &start_pos);
        gtk_editable_set_position(chatEntry, -1);
    }
    else
    {
        start_pos = gtk_editable_get_position(chatEntry);
        gtk_editable_insert_text(chatEntry, string("[" + ch + "][/" + ch + "]").c_str(), -1, &start_pos);
        gtk_editable_set_position(chatEntry, start_pos - 4);
    }
}

void Hub::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) throw()
{
    if (user.getUrl() != client->getHubUrl())
        return;

    ParamMap params;
    params.insert(ParamMap::value_type("Nick", user.getNick()));
    params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
    params.insert(ParamMap::value_type("Order", ClientManager::getInstance()->isOp(user.getUser(), user.getUrl()) ? "o" : "u"));

    Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::addFavoriteUser_gui, params);
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(FavoriteManagerListener::UserRemoved, const FavoriteUser &user) throw()
{
    if (user.getUrl() != client->getHubUrl())
        return;

    ParamMap params;
    params.insert(ParamMap::value_type("Nick", user.getNick()));
    params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

    Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::removeFavoriteUser_gui, params);
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Connecting, Client *) throw()
{
    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
    F3 *f3 = new F3(this, &Hub::addStatusMessage_gui, _("Connecting to ") + client->getHubUrl() + "...", Msg::STATUS, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Hub::on(ClientListener::Connected, Client *) throw()
{
    typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
    F4 *func = new F4(this, &Hub::addStatusMessage_gui, _("Connected"), Msg::STATUS, Sound::HUB_CONNECT, Notify::HUB_CONNECT);
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::UserUpdated, Client *, const OnlineUser &user) throw()
{
    Identity id = user.getIdentity();

    if (!id.isHidden())
    {
        ParamMap params;
        getParams_client(params, id);
        Func1<Hub, ParamMap> *func = new Func1<Hub, ParamMap>(this, &Hub::updateUser_gui, params);
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void Hub::on(ClientListener::UsersUpdated, Client *, const OnlineUserList &list) throw()
{
    Identity id;
    typedef Func1<Hub, ParamMap> F1;
    F1 *func;

    for (OnlineUserList::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        id = (*it)->getIdentity();
        if (!id.isHidden())
        {
            ParamMap params;
            getParams_client(params, id);
            func = new F1(this, &Hub::updateUser_gui, params);
            WulforManager::get()->dispatchGuiFunc(func);
        }
    }
}

void Hub::on(ClientListener::UserRemoved, Client *, const OnlineUser &user) throw()
{
    Func1<Hub, string> *func = new Func1<Hub, string>(this, &Hub::removeUser_gui, user.getUser()->getCID().toBase32());
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::Redirect, Client *, const string &address) throw()
{
    // redirect_client() crashes unless I put it into the dispatcher (why?)
    typedef Func2<Hub, string, bool> F2;
    F2 *func = new F2(this, &Hub::redirect_client, address, BOOLSETTING(AUTO_FOLLOW));
    WulforManager::get()->dispatchClientFunc(func);
}

void Hub::on(ClientListener::Failed, Client *, const string &reason) throw()
{
    Func0<Hub> *f0 = new Func0<Hub>(this, &Hub::clearNickList_gui);
    WulforManager::get()->dispatchGuiFunc(f0);

    typedef Func4<Hub, string, Msg::TypeMsg, Sound::TypeSound, Notify::TypeNotify> F4;
    F4 *f4 = new F4(this, &Hub::addStatusMessage_gui, _("Connect failed: ") + reason, Msg::SYSTEM, Sound::HUB_DISCONNECT, Notify::HUB_DISCONNECT);
    WulforManager::get()->dispatchGuiFunc(f4);
}

void Hub::on(ClientListener::GetPassword, Client *) throw()
{
    if (!client->getPassword().empty())
        client->password(client->getPassword());
    else
    {
        Func0<Hub> *func = new Func0<Hub>(this, &Hub::getPassword_gui);
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void Hub::on(ClientListener::HubUpdated, Client *) throw()
{
    typedef Func1<Hub, string> F1;
    string hubName;

    if (client->getHubName().empty())
        hubName = client->getAddress() + ":" + Util::toString(client->getPort());
    else
        hubName = client->getHubName();

    if (!client->getHubDescription().empty())
        hubName += " - " + client->getHubDescription();

    F1 *func1 = new F1(this, &BookEntry::setLabel_gui, hubName);
    WulforManager::get()->dispatchGuiFunc(func1);
}

void Hub::on(ClientListener::Message, Client*, const ChatMessage& message) throw() //NOTE: core 0.762
    {
        if (message.text.empty())
                return;

        Msg::TypeMsg typemsg;
        string cid = message.from->getIdentity().getUser()->getCID().toBase32();
        string line;

        string info=Util::formatAdditionalInfo(message.from->getIdentity().getIp(),BOOLSETTING(USE_IP),BOOLSETTING(GET_USER_COUNTRY));
        line+=info;

        if (message.thirdPerson)
                line += "* " + message.from->getIdentity().getNick() + " " +  message.text;
        else
                line += "<" + message.from->getIdentity().getNick() + "> " + message.text;

        if(message.to && message.replyTo)
        {
                //private message

                string error;
                const OnlineUser *user = (message.replyTo->getUser() == ClientManager::getInstance()->getMe())?
                        message.to : message.replyTo;

                if (message.from->getIdentity().isOp()) typemsg = Msg::OPERATOR;
                else if (message.from->getUser() == client->getMyIdentity().getUser()) typemsg = Msg::MYOWN;
                else typemsg = Msg::PRIVATE;

                if (user->getIdentity().isHub() && BOOLSETTING(IGNORE_HUB_PMS))
                {
                        error = _("Ignored private message from hub");
                        typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
                        F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
                        WulforManager::get()->dispatchGuiFunc(func);
                }
                else if (user->getIdentity().isBot() && BOOLSETTING(IGNORE_BOT_PMS))
                {
                        error = _("Ignored private message from bot ") + user->getIdentity().getNick();
                        typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
                        F3 *func = new F3(this, &Hub::addStatusMessage_gui, error, Msg::STATUS, Sound::NONE);
                        WulforManager::get()->dispatchGuiFunc(func);
                }
        else
                {
                        typedef Func6<Hub, Msg::TypeMsg, string, string, string, string, bool> F6;
                        F6 *func = new F6(this, &Hub::addPrivateMessage_gui, typemsg, message.from->getUser()->getCID().toBase32(),
                        user->getUser()->getCID().toBase32(), client->getHubUrl(), line, TRUE);
                        WulforManager::get()->dispatchGuiFunc(func);
                }
        }
        else
        {
                 // chat message

                if (message.from->getIdentity().isHub()) typemsg = Msg::STATUS;
                else if (message.from->getUser() == client->getMyIdentity().getUser()) typemsg = Msg::MYOWN;
                else typemsg = Msg::GENERAL;

        if (BOOLSETTING(FILTER_MESSAGES))
        {
                        if ((message.text.find("Hub-Security") != string::npos &&
                                message.text.find("was kicked by") != string::npos) ||
                                (message.text.find("is kicking") != string::npos && message.text.find("because:") != string::npos))
            {
                typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
                F3 *func = new F3(this, &Hub::addStatusMessage_gui, line, Msg::STATUS, Sound::NONE);
                WulforManager::get()->dispatchGuiFunc(func);

                return;
            }
        }

        if (BOOLSETTING(LOG_MAIN_CHAT))
        {
            StringMap params;
            params["message"] = line;
            client->getHubIdentity().getParams(params, "hub", false);
            params["hubURL"] = client->getHubUrl();
            client->getMyIdentity().getParams(params, "my", true);
            LOG(LogManager::CHAT, params);
        }

        typedef Func3<Hub, string, string, Msg::TypeMsg> F3;
        F3 *func = new F3(this, &Hub::addMessage_gui, cid, line, typemsg);
        WulforManager::get()->dispatchGuiFunc(func);

        // Set urgency hint if message contains user's nick
                if (WGETB("bold-hub") && message.from->getIdentity().getUser() != client->getMyIdentity().getUser())
        {
                        if (message.text.find(client->getMyIdentity().getNick()) != string::npos)
            {
                typedef Func0<Hub> F0;
                F0 *func = new F0(this, &Hub::setUrgent_gui);
                WulforManager::get()->dispatchGuiFunc(func);
            }
        }
    }
} //NOTE: core 0.762

void Hub::on(ClientListener::StatusMessage, Client *, const string &message, int /* flag */) throw()
{
    if (!message.empty())
    {
        if (BOOLSETTING(FILTER_MESSAGES))
        {
            if ((message.find("Hub-Security") != string::npos && message.find("was kicked by") != string::npos) ||
                (message.find("is kicking") != string::npos && message.find("because:") != string::npos))
            {
                typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
                F3 *func = new F3(this, &Hub::addStatusMessage_gui, message, Msg::STATUS, Sound::NONE);
                WulforManager::get()->dispatchGuiFunc(func);
                return;
            }
        }

        if (BOOLSETTING(LOG_STATUS_MESSAGES))
        {
            StringMap params;
            client->getHubIdentity().getParams(params, "hub", FALSE);
            params["hubURL"] = client->getHubUrl();
            client->getMyIdentity().getParams(params, "my", TRUE);
            params["message"] = message;
            LOG(LogManager::STATUS, params);
        }

        typedef Func3<Hub, string, string, Msg::TypeMsg> F3;
        F3 *func = new F3(this, &Hub::addMessage_gui, "", message, Msg::STATUS);
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void Hub::on(ClientListener::NickTaken, Client *) throw()
{
    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
    F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Nick already taken"), Msg::STATUS, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(func);
}

void Hub::on(ClientListener::SearchFlood, Client *, const string &msg) throw()
{
    typedef Func3<Hub, string, Msg::TypeMsg, Sound::TypeSound> F3;
    F3 *func = new F3(this, &Hub::addStatusMessage_gui, _("Search spam detected from ") + msg, Msg::STATUS, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(func);
}
