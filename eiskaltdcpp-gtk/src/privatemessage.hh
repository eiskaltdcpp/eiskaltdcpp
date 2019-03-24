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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, compiling, linking, and/or
 * using OpenSSL with this program is allowed.
 */

#pragma once

#include <dcpp/stdinc.h>
#include <dcpp/ClientManagerListener.h>
#include "bookentry.hh"
#include "message.hh"

class WulforSettingsManager;
class EmoticonsDialog;

class PrivateMessage:
        public BookEntry,
        public dcpp::ClientManagerListener
{
public:
    PrivateMessage(const std::string &_cid, const std::string &_hubUrl);
    virtual ~PrivateMessage();
    virtual void show();

    // GUI functions
    void addMessage_gui(std::string message, Msg::TypeMsg typemsg);
    void addStatusMessage_gui(std::string message, Msg::TypeMsg typemsg);
    void preferences_gui();
    bool getIsOffline() { return offline;}

private:
    // GUI functions
    void setStatus_gui(std::string text);
    void addLine_gui(Msg::TypeMsg typemsg, const std::string &line);
    void applyTags_gui(const std::string &line);
    void applyEmoticons_gui();
    void getSettingTag_gui(WulforSettingsManager *wsm, const Tag::TypeTag type, std::string &fore, std::string &back, bool &bold, bool &italic);
    GtkTextTag* createTag_gui(const std::string &tagname, Tag::TypeTag type);
    void updateCursor(GtkWidget *widget);
    void updateOnlineStatus_gui(bool online);

    // GUI callbacks
    static gboolean onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data);
    static gboolean onKeyPress_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
    static gboolean onLinkTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
    static gboolean onHubTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
    static gboolean onMagnetTagEvent_gui(GtkTextTag *tag, GObject *textView, GdkEvent *event, GtkTextIter *iter, gpointer data);
    static gboolean onChatPointerMoved_gui(GtkWidget *widget, GdkEventMotion *event, gpointer data);
    static gboolean onChatVisibilityChanged_gui(GtkWidget* widget, GdkEventVisibility* event, gpointer data);
    static gboolean onEmotButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
    static void onChatScroll_gui(GtkAdjustment *adjustment, gpointer data);
    static void onChatResize_gui(GtkAdjustment *adjustment, gpointer data);
    static void onSendMessage_gui(GtkEntry *entry, gpointer data);
    static void onCopyURIClicked_gui(GtkMenuItem *item, gpointer data);
    static void onOpenLinkClicked_gui(GtkMenuItem *item, gpointer data);
    static void onOpenHubClicked_gui(GtkMenuItem *item, gpointer data);
    static void onSearchMagnetClicked_gui(GtkMenuItem *item, gpointer data);
    static void onMagnetPropertiesClicked_gui(GtkMenuItem *item, gpointer data);
    static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
    static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
    static void onCommandClicked_gui(GtkWidget *widget, gpointer data);
    static void onUseEmoticons_gui(GtkWidget *widget, gpointer data);
    static gboolean onChatCommandButtonRelease_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

    // Client functions
    void sendMessage_client(std::string message);
    void addFavoriteUser_client();
    void removeFavoriteUser_client();
    void getFileList_client();
    void grantSlot_client();

    // client callback
    virtual void on(dcpp::ClientManagerListener::UserConnected, const dcpp::UserPtr& aUser) noexcept;
    virtual void on(dcpp::ClientManagerListener::UserDisconnected, const dcpp::UserPtr& aUser) noexcept;

    GtkTextBuffer *messageBuffer;
    GtkTextMark *mark, *start_mark, *end_mark, *tag_mark, *emot_mark;
    std::string cid;
    std::string hubUrl;
    bool isBot;
    std::vector<std::string> history;
    int historyIndex;
    bool sentAwayMessage;
    static const int maxLines = 500; ///@todo: make these preferences
    static const int maxHistory = 20;
    GdkCursor* handCursor;
    std::string selectedTagStr;
    GtkTextTag* selectedTag;
    bool scrollToBottom;
    GtkTextTag *TagsMap[Tag::TAG_LAST];
    Tag::TypeTag tagMsg, tagNick;
    bool useEmoticons;
    gint totalEmoticons;
    EmoticonsDialog *emotdialog;
    bool offline;
};
