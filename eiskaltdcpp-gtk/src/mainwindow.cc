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

#include "mainwindow.hh"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <dcpp/ShareManager.h>
#include <dcpp/Text.h>
#include <dcpp/Upload.h>
#include <dcpp/Download.h>
#include <dcpp/ClientManager.h>
#include <dcpp/ADLSearch.h>
#include <dcpp/ConnectivityManager.h>
#include <dcpp/version.h>
#include <dcpp/HashManager.h>
#include "downloadqueue.hh"
#include "favoritehubs.hh"
#include "favoriteusers.hh"
#include "finishedtransfers.hh"
#include "func.hh"
#include "hub.hh"
#include "privatemessage.hh"
#include "publichubs.hh"
#include "search.hh"
#include "searchspy.hh"
#include "settingsmanager.hh"
#include "sharebrowser.hh"
#include "emoticons.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "adlsearch.hh"
#include "WulforUtil.hh"
#include "Version.h"

using namespace std;
using namespace dcpp;

MainWindow::MainWindow():
    Entry(Entry::MAIN_WINDOW, "mainwindow.glade"),
    transfers(NULL),
    lastUpdate(0),
    lastUp(0),
    lastDown(0),
    minimized(FALSE),
    timer(0),
    statusFrame(1)
{
    window = GTK_WINDOW(getWidget("mainWindow"));
    gtk_window_set_role(window, getID().c_str());

    // Configure the dialogs
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("exitDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("connectDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("flistDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("ucLineDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("exitDialog")), window);
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("connectDialog")), window);
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("flistDialog")), window);
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("ucLineDialog")), window);

    // toolbar
    setToolbarMenu_gui("addMenuItemBar", "add", "toolbar-button-add");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem1", "toolbar-button-separators");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem2", "toolbar-button-separators");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem3", "toolbar-button-separators");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem4", "toolbar-button-separators");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem5", "toolbar-button-separators");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem6", "toolbar-button-separators");
    setToolbarMenu_gui("separatorsMenuItemBar", "SeparatorToolItem7", "toolbar-button-separators");
    setToolbarMenu_gui("reconnectMenuItemBar", "reconnect", "toolbar-button-reconnect");
    setToolbarMenu_gui("connectMenuItemBar", "connect", "toolbar-button-connect");
    setToolbarMenu_gui("favHubsMenuItemBar", "favHubs", "toolbar-button-fav-hubs");
    setToolbarMenu_gui("favUsersMenuItemBar", "favUsers", "toolbar-button-fav-users");
    setToolbarMenu_gui("publicHubsMenuItemBar", "publicHubs", "toolbar-button-public-hubs");
    setToolbarMenu_gui("settingsMenuItemBar", "settings", "toolbar-button-settings");
    setToolbarMenu_gui("own_filelistMenuItemBar", "own_file_list", "toolbar-button-own-filelist");
    setToolbarMenu_gui("refreshMenuItemBar", "refresh", "toolbar-button-refresh");
    setToolbarMenu_gui("hashMenuItemBar", "hash", "toolbar-button-hash");
    setToolbarMenu_gui("searchMenuItemBar", "search", "toolbar-button-search");
    setToolbarMenu_gui("searchADLMenuItemBar", "searchADL", "toolbar-button-search-adl");
    setToolbarMenu_gui("searchSpyMenuItemBar", "searchSpy", "toolbar-button-search-spy");
    setToolbarMenu_gui("queueMenuItemBar", "queue", "toolbar-button-queue");
    setToolbarMenu_gui("finishedDownloadsMenuItemBar", "finishedDownloads", "toolbar-button-finished-downloads");
    setToolbarMenu_gui("finishedUploadsMenuItemBar", "finishedUploads", "toolbar-button-finished-uploads");
    setToolbarMenu_gui("quitMenuItemBar", "quit", "toolbar-button-quit");

    gint pos = 0;
    ToolbarStyle = 0;
    GtkBox *box = GTK_BOX(getWidget("hbox4"));
    GtkWidget *child = getWidget("toolbar1");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("leftToolbarItem")), TRUE);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("hideToolbarItem")), (WGETI("toolbar-style") == 4) ? TRUE : FALSE);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("sizeToolbarItem")), WGETB("toolbar-small"));
    if (WGETB("toolbar-small"))
        g_object_set(G_OBJECT(child), "icon-size", GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);

    if (WGETI("toolbar-position") == 0)
    {
        box = GTK_BOX(getWidget("vbox1"));
        gtk_toolbar_set_orientation(GTK_TOOLBAR(child), GTK_ORIENTATION_HORIZONTAL);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("topToolbarItem")), TRUE);
        pos = 1;
    }
    gtk_box_pack_start(box, child, FALSE, FALSE, 2);
    gtk_box_reorder_child(box, child, pos);
    g_object_unref(child);

    g_signal_connect(G_OBJECT(getWidget("sizeToolbarItem")), "toggled", G_CALLBACK(onSizeToolbarToggled_gui), (gpointer)this);
    g_signal_connect(G_OBJECT(getWidget("hideToolbarItem")), "toggled", G_CALLBACK(onHideToolbarToggled_gui), (gpointer)this);
    g_signal_connect(G_OBJECT(getWidget("topToolbarItem")), "toggled", G_CALLBACK(onTopToolbarToggled_gui), (gpointer)this);
    g_signal_connect(G_OBJECT(getWidget("leftToolbarItem")), "toggled", G_CALLBACK(onLeftToolbarToggled_gui), (gpointer)this);
    g_signal_connect(G_OBJECT(getWidget("add")), "clicked", G_CALLBACK(onAddButtonClicked_gui), (gpointer)this);

    // TTH file get dialog
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("TTHFileDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL,-1);
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("TTHFileDialog")),window);
    ///todo response@
    g_signal_connect(getWidget("TTHFileDialog"), "delete-event", G_CALLBACK(onDeleteEventMagnetDialog_gui), (gpointer)this);

    GtkWidget *menu = gtk_menu_new();
    gtk_menu_tool_button_set_menu(GTK_MENU_TOOL_BUTTON(getWidget("favHubs")), menu);
    const FavoriteHubEntryList &fh = FavoriteManager::getInstance()->getFavoriteHubs();
    gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback)gtk_widget_destroy, NULL);

    for (FavoriteHubEntryList::const_iterator it = fh.begin(); it != fh.end(); ++it)
    {
        FavoriteHubEntry *entry = *it;
        string address = entry->getServer();
        string encoding = entry->getEncoding();
        GtkWidget *item = gtk_menu_item_new_with_label(address.c_str());
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        g_object_set_data_full(G_OBJECT(item), "address", g_strdup(address.c_str()), g_free);
        g_object_set_data_full(G_OBJECT(item), "encoding", g_strdup(encoding.c_str()), g_free);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onHubClicked_gui), (gpointer)this);
    }
    gtk_widget_show_all(menu);

    // menu
    g_object_ref_sink(getWidget("statusIconMenu"));
    g_object_ref_sink(getWidget("toolbarMenu"));

    // magnet dialog
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("MagnetDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("MagnetDialog")), window);
    setChooseMagnetDialog_gui();
    g_signal_connect(getWidget("MagnetDialog"), "response", G_CALLBACK(onResponseMagnetDialog_gui), (gpointer) this);
    g_signal_connect(getWidget("MagnetDialog"), "delete-event", G_CALLBACK(onDeleteEventMagnetDialog_gui), (gpointer) this);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("transferCheckButton")), TRUE);

    // About dialog
    gchar *comments = g_strdup_printf(_("DC++ Client based on the source code of FreeDC++ and LinuxDC++\n\nEiskaltDC++ version: %s (%s)\nDC++ core version: %s"),
        EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX, DCVERSIONSTRING);

    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), comments);
    g_free(comments);

    // set logo 96x96
    GtkIconTheme *iconTheme = gtk_icon_theme_get_default();
    GdkPixbuf *logo = gtk_icon_theme_load_icon(iconTheme, "eiskaltdcpp", 128, GTK_ICON_LOOKUP_FORCE_SVG, NULL);

    if (logo != NULL)
    {
        gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), logo);
        g_object_unref(logo);
    }

    gtk_about_dialog_set_email_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
    gtk_about_dialog_set_url_hook((GtkAboutDialogActivateLinkFunc)onAboutDialogActivateLink_gui, (gpointer)this, NULL);
    // This has to be set in code in order to activate the link
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(getWidget("aboutDialog")), "http://eiskaltdc.googlecode.com");
    gtk_window_set_transient_for(GTK_WINDOW(getWidget("aboutDialog")), window);

    // Set all windows to the default icon
    gtk_window_set_default_icon_name("eiskaltdcpp");

    // All notebooks created in glade need one page.
    // In our case, this is just a placeholder, so we remove it.
    gtk_notebook_remove_page(GTK_NOTEBOOK(getWidget("book")), -1);
    g_object_set_data(G_OBJECT(getWidget("book")), "page-rotation-list", NULL);
    gtk_widget_set_sensitive(getWidget("closeMenuItem"), FALSE);

    // Connect the signals to their callback functions.
    g_signal_connect(window, "delete-event", G_CALLBACK(onCloseWindow_gui), (gpointer)this);
    g_signal_connect(window, "window-state-event", G_CALLBACK(onWindowState_gui), (gpointer)this);
    g_signal_connect(window, "focus-in-event", G_CALLBACK(onFocusIn_gui), (gpointer)this);
    g_signal_connect(window, "key-press-event", G_CALLBACK(onKeyPressed_gui), (gpointer)this);
    g_signal_connect(getWidget("book"), "switch-page", G_CALLBACK(onPageSwitched_gui), (gpointer)this);
    g_signal_connect_after(getWidget("pane"), "realize", G_CALLBACK(onPaneRealized_gui), (gpointer)this);
    g_signal_connect(getWidget("reconnect"), "clicked", G_CALLBACK(onReconnectClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("connect"), "clicked", G_CALLBACK(onConnectClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("favHubs"), "clicked", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("favUsers"), "clicked", G_CALLBACK(onFavoriteUsersClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("publicHubs"), "clicked", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("settings"), "clicked", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("own_file_list"), "clicked", G_CALLBACK(onOpenOwnListClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("refresh"), "clicked", G_CALLBACK(onRefreshFileListClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("hash"), "clicked", G_CALLBACK(onHashClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("search"), "clicked", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchSpy"), "clicked", G_CALLBACK(onSearchSpyClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("queue"), "clicked", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchADL"), "clicked", G_CALLBACK(onSearchADLClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("quit"), "clicked", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("finishedDownloads"), "clicked", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("finishedUploads"), "clicked", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("openFileListMenuItem"), "activate", G_CALLBACK(onOpenFileListClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("openOwnListMenuItem"), "activate", G_CALLBACK(onOpenOwnListClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("refreshFileListMenuItem"), "activate", G_CALLBACK(onRefreshFileListClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("quickConnectMenuItem"), "activate", G_CALLBACK(onConnectClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("reconnectMenuItem"), "activate", G_CALLBACK(onReconnectClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("settingsMenuItem"), "activate", G_CALLBACK(onPreferencesClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("closeMenuItem"), "activate", G_CALLBACK(onCloseClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("exitMenuItem"), "activate", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("favoriteHubsMenuItem"), "activate", G_CALLBACK(onFavoriteHubsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("favoriteUsersMenuItem"), "activate", G_CALLBACK(onFavoriteUsersClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("publicHubsMenuItem"), "activate", G_CALLBACK(onPublicHubsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("indexingProgressMenuItem"), "activate", G_CALLBACK(onHashClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchMenuItem"), "activate", G_CALLBACK(onSearchClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchSpyMenuItem"), "activate", G_CALLBACK(onSearchSpyClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchADLMenuItem"), "activate", G_CALLBACK(onSearchADLClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("downloadQueueMenuItem"), "activate", G_CALLBACK(onDownloadQueueClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("finishedDownloadsMenuItem"), "activate", G_CALLBACK(onFinishedDownloadsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("finishedUploadsMenuItem"), "activate", G_CALLBACK(onFinishedUploadsClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("previousTabMenuItem"), "activate", G_CALLBACK(onPreviousTabClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("nextTabMenuItem"), "activate", G_CALLBACK(onNextTabClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("aboutMenuItem"), "activate", G_CALLBACK(onAboutClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("transferCheckButton"), "toggled", G_CALLBACK(onTransferToggled_gui), (gpointer)this);
    g_signal_connect(getWidget("browseButton"), "clicked", G_CALLBACK(onBrowseMagnetButton_gui), (gpointer)this);
    g_signal_connect(getWidget("dowloadQueueRadioButton"), "toggled", G_CALLBACK(onDowloadQueueToggled_gui), (gpointer)this);
    g_signal_connect(getWidget("searchRadioButton"), "toggled", G_CALLBACK(onSearchMagnetToggled_gui), (gpointer)this);
    g_signal_connect(getWidget("showRadioButton"), "toggled", G_CALLBACK(onSearchMagnetToggled_gui), (gpointer)this);
    g_signal_connect(getWidget("setMagnetChoiceItem"), "activate", G_CALLBACK(onSetMagnetChoiceDialog_gui), (gpointer)this);
    //TTHFileDialog
    g_signal_connect(getWidget("TTHFileMenu"), "activate", G_CALLBACK(onTTHFileDialog_gui), (gpointer)this);
    g_signal_connect(getWidget("buttonfile"), "clicked", G_CALLBACK(onTTHFileButton_gui), (gpointer)this);
    //GtkWidget *buttonf = getWidget("filechooserbutton");
    //g_signal_connect(buttonf, "dialog",G_CALLBACK(onFileTTHSet), (gpointer)this);

    // Help menu
    g_object_set_data_full(G_OBJECT(getWidget("homeMenuItem")), "link",
        g_strdup("http://code.google.com/p/eiskaltdc/"), g_free);
    g_signal_connect(getWidget("homeMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

    g_object_set_data_full(G_OBJECT(getWidget("sourceMenuItem")), "link",
        g_strdup("http://github.com/negativ/eiskaltdcpp/"), g_free);
    g_signal_connect(getWidget("sourceMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

    g_object_set_data_full(G_OBJECT(getWidget("issueMenuItem")), "link",
        g_strdup("http://code.google.com/p/eiskaltdc/issues/list"), g_free);
    g_signal_connect(getWidget("issueMenuItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

    g_object_set_data_full(G_OBJECT(getWidget("wikiItem")), "link",
        g_strdup("http://code.google.com/p/eiskaltdc/w/list"), g_free);
    g_signal_connect(getWidget("wikiItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

    // Now available: ChangeLog.txt, ChangeLog_ru.txt, ChangeLog_uk.txt
    g_object_set_data_full(G_OBJECT(getWidget("changeLogItem")), "link",
        g_strdup(_("http://github.com/negativ/eiskaltdcpp/raw/master/ChangeLog.txt")), g_free);
    g_signal_connect(getWidget("changeLogItem"), "activate", G_CALLBACK(onLinkClicked_gui), NULL);

    onQuit = FALSE;

    // Load window state and position from settings manager
    gint posX = WGETI("main-window-pos-x");
    gint posY = WGETI("main-window-pos-y");
    gint sizeX = WGETI("main-window-size-x");
    gint sizeY = WGETI("main-window-size-y");

    gtk_window_move(window, posX, posY);
    gtk_window_resize(window, sizeX, sizeY);

    setMainStatus_gui(_("Welcome to ") + string(g_get_application_name()));

    loadIcons_gui();
    showTransfersPane_gui();

    // Putting this after all the resizing and moving makes the window appear
    // in the correct position instantly, looking slightly more cool
    // (seems we have rather poor standards for cool?)
    gtk_widget_show_all(GTK_WIDGET(window));

    setToolbarButton_gui();
    setTabPosition_gui(WGETI("tab-position"));
    setToolbarStyle_gui(WGETI("toolbar-style"));

    createStatusIcon_gui();

    Sound::start();
    Emoticons::start();
    Notify::start();

    if (WGETI("main-window-maximized"))
        gtk_window_maximize(window);

#ifdef LUA_SCRIPT
    ScriptManager::getInstance()->load();//aded
    // Start as late as possible, as we might (formatting.lua) need to examine settings
    string defaultluascript="startup.lua";
    ScriptManager::getInstance()->EvaluateFile(defaultluascript);
#endif
}

MainWindow::~MainWindow()
{
    QueueManager::getInstance()->removeListener(this);
    TimerManager::getInstance()->removeListener(this);
    LogManager::getInstance()->removeListener(this);

    GList *list = (GList *)g_object_get_data(G_OBJECT(getWidget("book")), "page-rotation-list");
    g_list_free(list);

    // Save window state and position
    gint posX, posY, sizeX, sizeY, transferPanePosition;
    bool maximized = TRUE;
    GdkWindowState gdkState;

    gtk_window_get_position(window, &posX, &posY);
    gtk_window_get_size(window, &sizeX, &sizeY);
    gdkState = gdk_window_get_state(GTK_WIDGET(window)->window);
    transferPanePosition = sizeY - gtk_paned_get_position(GTK_PANED(getWidget("pane")));

    if (!(gdkState & GDK_WINDOW_STATE_MAXIMIZED))
        maximized = FALSE;

    WSET("main-window-pos-x", posX);
    WSET("main-window-pos-y", posY);
    WSET("main-window-size-x", sizeX);
    WSET("main-window-size-y", sizeY);

    WSET("main-window-maximized", maximized);
    if (transferPanePosition > 10)
        WSET("transfer-pane-position", transferPanePosition);

    if (timer > 0)
        g_source_remove(timer);

    WSET("status-icon-blink-use", useStatusIconBlink);

    gtk_widget_destroy(GTK_WIDGET(window));
    g_object_unref(statusIcon);
    g_object_unref(getWidget("statusIconMenu"));
    g_object_unref(getWidget("toolbarMenu"));

    Sound::stop();
    Emoticons::stop();
    Notify::stop();
}

GtkWidget *MainWindow::getContainer()
{
    return getWidget("mainWindow");
}

void MainWindow::show()
{
    QueueManager::getInstance()->addListener(this);
    TimerManager::getInstance()->addListener(this);
    LogManager::getInstance()->addListener(this);

    typedef Func2<MainWindow, bool, int> F2;
    typedef Func0<MainWindow> F0;
    F2 *f2 = new F2(this, &MainWindow::startSocket_client, true, 0);
    WulforManager::get()->dispatchClientFunc(f2);

    F0 *f0 = new F0(this, &MainWindow::autoConnect_client);
    WulforManager::get()->dispatchClientFunc(f0);

    autoOpen_gui();
}

void MainWindow::setTitle(const string& text)
{
    string title;

    if (!text.empty())
        title = text + " - " + g_get_application_name();
    else
        title = g_get_application_name();

    gtk_window_set_title(window, title.c_str());
}

bool MainWindow::isActive_gui()
{
    return gtk_window_is_active(window);
}

void MainWindow::setUrgent_gui()
{
    gtk_window_set_urgency_hint(window, true);
}

/*
 * Create and show Transfers pane
 */
void MainWindow::showTransfersPane_gui()
{
    dcassert(transfers == NULL);

    transfers = new Transfers();
    gtk_paned_pack2(GTK_PANED(getWidget("pane")), transfers->getContainer(), TRUE, TRUE);
    addChild(transfers);
    transfers->show();
}

/*
 * Load the custom icons or the stock icons as per the setting
 */
void MainWindow::loadIcons_gui()
{
    WulforUtil::registerIcons();

    // Reset the stock IDs manually to force the icon to refresh
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("favHubs")), "icon-favorite-hubs");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("favUsers")), "icon-favorite-users");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("publicHubs")), "icon-public-hubs");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("settings")), "icon-preferences");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("own_file_list")), "icon-own-filelist");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("refresh")), "icon-refresh");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("hash")), "icon-hash");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("search")), "icon-search");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("searchSpy")), "icon-search-spy");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("queue")), "icon-queue");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("finishedDownloads")), "icon-finished-downloads");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("finishedUploads")), "icon-finished-uploads");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("searchADL")), "icon-search-adl");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("quit")), "icon-quit");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("reconnect")), "icon-reconnect");
    gtk_tool_button_set_stock_id(GTK_TOOL_BUTTON(getWidget("connect")), "icon-connect");
    gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageHubs")), "icon-public-hubs", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageDownloadSpeed")), "icon-download", GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_image_set_from_stock(GTK_IMAGE(getWidget("imageUploadSpeed")), "icon-upload", GTK_ICON_SIZE_SMALL_TOOLBAR);
}

void MainWindow::autoOpen_gui()
{
    if (WGETB("open-public"))
         showPublicHubs_gui();
    if (WGETB("open-queue"))
         showDownloadQueue_gui();
    if (WGETB("open-favorite-hubs"))
         showFavoriteHubs_gui();
    if (WGETB("open-favorite-users"))
         showFavoriteUsers_gui();
    if (WGETB("open-finished-downloads"))
         showFinishedDownloads_gui();
    if (WGETB("open-finished-uploads"))
         showFinishedUploads_gui();
    if (WGETB("open-search-spy"))
         showSearchSpy_gui();
}

void MainWindow::setToolbarMenu_gui(const string &item_key, const string &button_key, const string &key)
{
    GtkWidget *item = getWidget(item_key);
    GtkWidget *button = getWidget(button_key);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), WGETB(key));
    g_object_set_data_full(G_OBJECT(item), "key", g_strdup(key.c_str()), g_free);
    g_signal_connect(G_OBJECT(item), "toggled", G_CALLBACK(onToolToggled_gui), (gpointer)button);
}

void MainWindow::addBookEntry_gui(BookEntry *entry)
{
    addChild(entry);

    GtkWidget *page = entry->getContainer();
    GtkWidget *label = entry->getLabelBox();
    GtkWidget *closeButton = entry->getCloseButton();
    GtkWidget *tabMenuItem = entry->getTabMenuItem();

    addTabMenuItem_gui(tabMenuItem, page);

    gtk_notebook_append_page(GTK_NOTEBOOK(getWidget("book")), page, label);

    g_signal_connect(label, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
    g_signal_connect(closeButton, "button-release-event", G_CALLBACK(onButtonReleasePage_gui), (gpointer)entry);
    g_signal_connect(closeButton, "clicked", G_CALLBACK(onCloseBookEntry_gui), (gpointer)entry);

    gtk_widget_set_sensitive(getWidget("closeMenuItem"), TRUE);

    gtk_notebook_set_tab_reorderable(GTK_NOTEBOOK(getWidget("book")), page, TRUE);

    entry->show();
}

GtkWidget *MainWindow::currentPage_gui()
{
    int pageNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("book")));

    if (pageNum == -1)
        return NULL;
    else
        return gtk_notebook_get_nth_page(GTK_NOTEBOOK(getWidget("book")), pageNum);
}

void MainWindow::raisePage_gui(GtkWidget *page)
{
    int num = gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("book")), page);
    int currentNum = gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("book")));

    if (num != -1 && num != currentNum)
        gtk_notebook_set_current_page(GTK_NOTEBOOK(getWidget("book")), num);
}

void MainWindow::removeBookEntry_gui(BookEntry *entry)
{
    string entryID = entry->getID();

    string::size_type pos = entryID.find(':');
    if (pos != string::npos) entryID.erase(0, pos + 1);

    StringIter it = find(EntryList.begin(), EntryList.end(), entryID);

    if (it != EntryList.end())
        EntryList.erase(it);

    GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));
    GtkWidget *page = entry->getContainer();
    GtkWidget* menuItem = entry->getTabMenuItem();
    int num = gtk_notebook_page_num(book, page);
    removeChild(entry);

    if (num != -1)
    {
        GList *list = (GList *)g_object_get_data(G_OBJECT(book), "page-rotation-list");
        list = g_list_remove(list, (gpointer)page);
        g_object_set_data(G_OBJECT(book), "page-rotation-list", (gpointer)list);

        // if removing the current page, switch to the previous page in the rotation list
        if (num == gtk_notebook_get_current_page(book))
        {
            GList *prev = g_list_first(list);
            if (prev != NULL)
            {
                gint childNum = gtk_notebook_page_num(book, GTK_WIDGET(prev->data));
                gtk_notebook_set_current_page(book, childNum);
            }
        }
        gtk_notebook_remove_page(book, num);

        removeTabMenuItem_gui(menuItem);

        if (gtk_notebook_get_n_pages(book) == 0)
        {
            gtk_widget_set_sensitive(getWidget("closeMenuItem"), FALSE);
            setTitle(""); // Reset window title to default
        }
    }
}

void MainWindow::previousTab_gui()
{
    GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

    if (gtk_notebook_get_current_page(book) == 0)
        gtk_notebook_set_current_page(book, -1);
    else
        gtk_notebook_prev_page(book);
}

void MainWindow::nextTab_gui()
{
    GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

    if (gtk_notebook_get_n_pages(book) - 1 == gtk_notebook_get_current_page(book))
        gtk_notebook_set_current_page(book, 0);
    else
        gtk_notebook_next_page(book);
}

void MainWindow::addTabMenuItem_gui(GtkWidget* menuItem, GtkWidget* page)
{
    g_signal_connect(menuItem, "activate", G_CALLBACK(onRaisePage_gui), (gpointer)page);
    gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("tabsMenu")), menuItem);
    gtk_widget_show_all(getWidget("tabsMenu"));

    gtk_widget_set_sensitive(getWidget("previousTabMenuItem"), TRUE);
    gtk_widget_set_sensitive(getWidget("nextTabMenuItem"), TRUE);
    gtk_widget_set_sensitive(getWidget("tabMenuSeparator"), TRUE);
}

void MainWindow::removeTabMenuItem_gui(GtkWidget *menuItem)
{
    GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));

    gtk_container_remove(GTK_CONTAINER(getWidget("tabsMenu")), menuItem);

    if (gtk_notebook_get_n_pages(book) == 0)
    {
        gtk_widget_set_sensitive(getWidget("previousTabMenuItem"), FALSE);
        gtk_widget_set_sensitive(getWidget("nextTabMenuItem"), FALSE);
        gtk_widget_set_sensitive(getWidget("tabMenuSeparator"), FALSE);
    }
}

/*
 * Create status icon.
 */
void MainWindow::createStatusIcon_gui()
{
    useStatusIconBlink = WGETB("status-icon-blink-use");
    statusIcon = gtk_status_icon_new_from_icon_name("eiskaltdcpp");

    g_signal_connect(getWidget("statusIconQuitItem"), "activate", G_CALLBACK(onQuitClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("statusIconShowInterfaceItem"), "toggled", G_CALLBACK(onShowInterfaceToggled_gui), (gpointer)this);
    g_signal_connect(statusIcon, "activate", G_CALLBACK(onStatusIconActivated_gui), (gpointer)this);
    g_signal_connect(statusIcon, "popup-menu", G_CALLBACK(onStatusIconPopupMenu_gui), (gpointer)this);

    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("statusIconBlinkUseItem")), useStatusIconBlink);
    g_signal_connect(getWidget("statusIconBlinkUseItem"), "toggled", G_CALLBACK(onStatusIconBlinkUseToggled_gui), (gpointer)this);

    if (WGETB("always-tray"))
        gtk_status_icon_set_visible(statusIcon, TRUE);
    else
        gtk_status_icon_set_visible(statusIcon, FALSE);
}

void MainWindow::updateStatusIconTooltip_gui(string download, string upload)
{
    ostringstream toolTip;
    toolTip << _("Download: ") << download << endl << _("Upload: ") << upload;
    gtk_status_icon_set_tooltip(statusIcon, toolTip.str().c_str());
}

void MainWindow::setMainStatus_gui(string text, time_t t)
{
    if (!text.empty())
    {
        text = "[" + Util::getShortTimeString(t) + "] " + text;
        gtk_label_set_text(GTK_LABEL(getWidget("labelStatus")), text.c_str());
    }
}

void MainWindow::showNotification_gui(string head, string body, Notify::TypeNotify notify)
{
    Notify::get()->showNotify(head, body, notify);
}

void MainWindow::setStats_gui(string hubs, string downloadSpeed,
    string downloaded, string uploadSpeed, string uploaded)
{
    gtk_label_set_text(GTK_LABEL(getWidget("labelHubs")), hubs.c_str());
    gtk_label_set_text(GTK_LABEL(getWidget("labelDownloadSpeed")), downloadSpeed.c_str());
    gtk_label_set_text(GTK_LABEL(getWidget("labelDownloaded")), downloaded.c_str());
    gtk_label_set_text(GTK_LABEL(getWidget("labelUploadSpeed")), uploadSpeed.c_str());
    gtk_label_set_text(GTK_LABEL(getWidget("labelUploaded")), uploaded.c_str());
}

BookEntry* MainWindow::findBookEntry(const EntryType type, const string &id)
{
    Entry *entry = getChild(type, id);
    return dynamic_cast<BookEntry*>(entry);
}

void MainWindow::showDownloadQueue_gui()
{
    BookEntry *entry = findBookEntry(Entry::DOWNLOAD_QUEUE);

    if (entry == NULL)
    {
        entry = new DownloadQueue();
        addBookEntry_gui(entry);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showFavoriteHubs_gui()
{
    BookEntry *entry = findBookEntry(Entry::FAVORITE_HUBS);

    if (entry == NULL)
    {
        entry = new FavoriteHubs();
        addBookEntry_gui(entry);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showFavoriteUsers_gui()
{
    BookEntry *entry = findBookEntry(Entry::FAVORITE_USERS);

    if (entry == NULL)
    {
        entry = new FavoriteUsers();
        addBookEntry_gui(entry);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showFinishedDownloads_gui()
{
    BookEntry *entry = findBookEntry(Entry::FINISHED_DOWNLOADS);

    if (entry == NULL)
    {
        entry = FinishedTransfers::createFinishedDownloads();
        addBookEntry_gui(entry);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showFinishedUploads_gui()
{
    BookEntry *entry = findBookEntry(Entry::FINISHED_UPLOADS);

    if (entry == NULL)
    {
        entry = FinishedTransfers::createFinishedUploads();
        addBookEntry_gui(entry);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showHub_gui(string address, string encoding)
{
    BookEntry *entry = findBookEntry(Entry::HUB, address);

    if (entry == NULL)
    {
        entry = new Hub(address, encoding);
        addBookEntry_gui(entry);

        EntryList.push_back(address);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showSearchSpy_gui()
{
        BookEntry *entry = findBookEntry(Entry::SEARCH_SPY);

        if (entry == NULL)
        {
                entry = new SearchSpy();
                addBookEntry_gui(entry);
        }

        raisePage_gui(entry->getContainer());
}

void MainWindow::showSearchADL_gui()
{
        BookEntry *entry = findBookEntry(Entry::SEARCH_ADL);

        if (entry == NULL)
        {
                entry = new SearchADL();
                addBookEntry_gui(entry);
        }

        raisePage_gui(entry->getContainer());
}

void MainWindow::addPrivateMessage_gui(Msg::TypeMsg typemsg, string cid, string hubUrl, string message, bool useSetting)
{
    BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);
    bool raise = TRUE;

    // If PM is initiated by another user, use setting except if tab is already open.
    if (useSetting)
        raise = (entry == NULL) ? !BOOLSETTING(POPUNDER_PM) : FALSE;

    if (entry == NULL)
    {
        entry = new PrivateMessage(cid, hubUrl);
        addBookEntry_gui(entry);

        EntryList.push_back(cid);
    }

    if (!message.empty())
    {
        dynamic_cast<PrivateMessage*>(entry)->addMessage_gui(message, typemsg);

        bool show = FALSE;

        if (!isActive_gui())
        {
            show = TRUE;
            if (useStatusIconBlink && timer == 0)
            {
                timer = g_timeout_add(1000, animationStatusIcon_gui, (gpointer)this);
            }
        }
        else if (currentPage_gui() != entry->getContainer() && !WGETI("notify-only-not-active"))
        {
            show = TRUE;
        }

        if (show)
        {
            const int length = WGETI("notify-pm-length");

            if (length > 0 && g_utf8_strlen(message.c_str(), -1) > length)
            {
                const gchar *p = message.c_str();
                int ii = 0;

                while (*p)
                {
                    p = g_utf8_next_char(p);

                    if (++ii >= length)
                        break;
                }

                string::size_type i = string(p).size();
                string::size_type j = message.size();

                Notify::get()->showNotify("", message.substr(0, j - i) + "...", Notify::PRIVATE_MESSAGE);
            }
            else
                Notify::get()->showNotify("", message, Notify::PRIVATE_MESSAGE);
        }
    }

    if (raise)
        raisePage_gui(entry->getContainer());
}

void MainWindow::removeTimerSource_gui()
{
    if (timer > 0)
    {
        g_source_remove(timer);
        timer = 0;
        gtk_status_icon_set_from_icon_name(statusIcon, "eiskaltdcpp");
    }
}

void MainWindow::addPrivateStatusMessage_gui(Msg::TypeMsg typemsg, string cid, string message)
{
    BookEntry *entry = findBookEntry(Entry::PRIVATE_MESSAGE, cid);

    if (entry != NULL)
        dynamic_cast<PrivateMessage*>(entry)->addStatusMessage_gui(message, typemsg);
}

void MainWindow::showPublicHubs_gui()
{
    BookEntry *entry = findBookEntry(Entry::PUBLIC_HUBS);

    if (entry == NULL)
    {
        entry = new PublicHubs();
        addBookEntry_gui(entry);
    }

    raisePage_gui(entry->getContainer());
}

void MainWindow::showShareBrowser_gui(UserPtr user, string filename, string dir, bool useSetting)
{
    bool raise = useSetting ? !BOOLSETTING(POPUNDER_FILELIST) : TRUE;
    BookEntry *entry = findBookEntry(Entry::SHARE_BROWSER, user->getCID().toBase32());

    if (entry == NULL)
    {
        entry = new ShareBrowser(user, filename, dir);
        addBookEntry_gui(entry);
    }

    if (raise)
        raisePage_gui(entry->getContainer());
}

Search *MainWindow::addSearch_gui()
{
    Search *entry = new Search();
    addBookEntry_gui(entry);
    raisePage_gui(entry->getContainer());
    return entry;
}

void MainWindow::addSearch_gui(string magnet)
{
    string name;
    int64_t size;
    string tth;

    if (WulforUtil::splitMagnet(magnet, name, size, tth))
    {
        Search *s = addSearch_gui();
        s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
    }
}

void MainWindow::fileToDownload_gui(string magnet, string path)
{
    string name;
    int64_t size;
    string tth;

    if (!WulforUtil::splitMagnet(magnet, name, size, tth))
        return;
    name = path + name;

    typedef Func3<MainWindow, string, int64_t, string> F3;
    F3 *func = new F3(this, &MainWindow::addFileDownloadQueue_client, name, size, tth);
    WulforManager::get()->dispatchClientFunc(func);
}

GtkWidget* MainWindow::getChooserDialog_gui()
{
    return getWidget("flistDialog");
}

void MainWindow::actionMagnet_gui(string magnet)
{
    if (GTK_WIDGET_VISIBLE(getWidget("MagnetDialog")))
        return;

    string name, tth;
    int64_t size;
    int action = WGETI("magnet-action");
    bool split = WulforUtil::splitMagnet(magnet, name, size, tth);

    if (action == 0 && split)
    {
        Search *s = addSearch_gui();
        s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
    }
    else if (action == 1 && split)
    {
        name = WGETS("magnet-choose-dir") + name;

        if (!File::isAbsolute(name))
            name = SETTING(DOWNLOAD_DIRECTORY) + name;

        typedef Func3<MainWindow, string, int64_t, string> F3;
        F3 *func = new F3(this, &MainWindow::addFileDownloadQueue_client, name, size, tth);
        WulforManager::get()->dispatchClientFunc(func);
    }
    else if (split)
    {
        showMagnetDialog_gui(magnet, name, size, tth);
    }
}

void MainWindow::updateFavoriteHubMenu_client(const FavoriteHubEntryList &fh)
{
    ListParamPair list;
    for (FavoriteHubEntryList::const_iterator it = fh.begin(); it != fh.end(); ++it)
    {
        ParamPair param;
        FavoriteHubEntry *entry = *it;
        param.first = entry->getServer();
        param.second = entry->getEncoding();
        list.push_back(param);
    }

    Func1<MainWindow, ListParamPair> *func = new Func1<MainWindow, ListParamPair>(this, &MainWindow::updateFavoriteHubMenu_gui, list);
    WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::updateFavoriteHubMenu_gui(ListParamPair list)
{
    GtkWidget *menu = gtk_menu_tool_button_get_menu(GTK_MENU_TOOL_BUTTON(getWidget("favHubs")));
    gtk_container_foreach(GTK_CONTAINER(menu), (GtkCallback)gtk_widget_destroy, NULL);

    for (ListParamPair::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        const ParamPair &param = *it;
        string address = param.first;
        string encoding = param.second;
        GtkWidget *item = gtk_menu_item_new_with_label(address.c_str());
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
        g_object_set_data_full(G_OBJECT(item), "address", g_strdup(address.c_str()), g_free);
        g_object_set_data_full(G_OBJECT(item), "encoding", g_strdup(encoding.c_str()), g_free);
        g_signal_connect(G_OBJECT(item), "activate", G_CALLBACK(onHubClicked_gui), (gpointer)this);
    }
    gtk_widget_show_all(menu);
}

void MainWindow::onHubClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    string address = (gchar*)g_object_get_data(G_OBJECT(widget), "address");
    string encoding = (gchar*)g_object_get_data(G_OBJECT(widget), "encoding");
    mw->showHub_gui(address, encoding);
}

void MainWindow::setToolbarButton_gui()
{
    if (!WGETB("toolbar-button-separators")){
        gtk_widget_hide(getWidget("SeparatorToolItem1"));
        gtk_widget_hide(getWidget("SeparatorToolItem2"));
        gtk_widget_hide(getWidget("SeparatorToolItem3"));
        gtk_widget_hide(getWidget("SeparatorToolItem4"));
        gtk_widget_hide(getWidget("SeparatorToolItem5"));
        gtk_widget_hide(getWidget("SeparatorToolItem6"));
        gtk_widget_hide(getWidget("SeparatorToolItem7"));
    }
    if (!WGETB("toolbar-button-reconnect"))
        gtk_widget_hide(getWidget("reconnect"));
    if (!WGETB("toolbar-button-connect"))
        gtk_widget_hide(getWidget("connect"));
    if (!WGETB("toolbar-button-fav-hubs"))
        gtk_widget_hide(getWidget("favHubs"));
    if (!WGETB("toolbar-button-fav-users"))
        gtk_widget_hide(getWidget("favUsers"));
    if (!WGETB("toolbar-button-public-hubs"))
        gtk_widget_hide(getWidget("publicHubs"));
    if (!WGETB("toolbar-button-settings"))
        gtk_widget_hide(getWidget("settings"));
    if (!WGETB("toolbar-button-own-filelist"))
        gtk_widget_hide(getWidget("own_file_list"));
    if (!WGETB("toolbar-button-refresh"))
        gtk_widget_hide(getWidget("refresh"));
    if (!WGETB("toolbar-button-hash"))
        gtk_widget_hide(getWidget("hash"));
    if (!WGETB("toolbar-button-search"))
        gtk_widget_hide(getWidget("search"));
    if (!WGETB("toolbar-button-search-spy"))
        gtk_widget_hide(getWidget("searchSpy"));
    if (!WGETB("toolbar-button-queue"))
        gtk_widget_hide(getWidget("queue"));
    if (!WGETB("toolbar-button-quit"))
        gtk_widget_hide(getWidget("quit"));
    if (!WGETB("toolbar-button-finished-downloads"))
        gtk_widget_hide(getWidget("finishedDownloads"));
    if (!WGETB("toolbar-button-finished-uploads"))
        gtk_widget_hide(getWidget("finishedUploads"));
    if (!WGETB("toolbar-button-search-adl"))
        gtk_widget_hide(getWidget("searchADL"));
    if (!WGETB("toolbar-button-add"))
        gtk_widget_hide(getWidget("add"));
}

void MainWindow::setTabPosition_gui(int position)
{
    GtkPositionType tabPosition;

    switch (position)
    {
        case 0:
            tabPosition = GTK_POS_TOP;
            break;
        case 1:
            tabPosition = GTK_POS_LEFT;
            break;
        case 2:
            tabPosition = GTK_POS_RIGHT;
            break;
        case 3:
            tabPosition = GTK_POS_BOTTOM;
            break;
        default:
            tabPosition = GTK_POS_TOP;
    }

    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(getWidget("book")), tabPosition);
}

void MainWindow::setToolbarStyle_gui(int style)
{
    GtkToolbarStyle toolbarStyle;

    switch (style)
    {
        case 1:
            toolbarStyle = GTK_TOOLBAR_TEXT;
            break;
        case 2:
            toolbarStyle = GTK_TOOLBAR_BOTH;
            break;
        case 3:
            toolbarStyle = GTK_TOOLBAR_BOTH_HORIZ;
            break;
        case 4:
            gtk_widget_hide(getWidget("toolbar1"));
            return;
        default:
            toolbarStyle = GTK_TOOLBAR_ICONS;
    }

    if (style != 4)
    {
        gtk_widget_show(getWidget("toolbar1"));
        gtk_toolbar_set_style(GTK_TOOLBAR(getWidget("toolbar1")), toolbarStyle);
    }
}

bool MainWindow::getUserCommandLines_gui(const string &command, StringMap &ucParams)
{
    string name;
    string label;
    string line;
    StringMap done;
    string::size_type i = 0;
    string::size_type j = 0;
    string text = string("<b>") + _("Enter value for ") + "\'";
    MainWindow *mw = WulforManager::get()->getMainWindow();

    while ((i = command.find("%[line:", i)) != string::npos)
    {
        i += 7;
        j = command.find(']', i);
        if (j == string::npos)
            break;

        name = command.substr(i, j - i);
        if (done.find(name) == done.end())
        {
            line.clear();
            label = text + name + "\'</b>";

            gtk_label_set_label(GTK_LABEL(mw->getWidget("ucLabel")), label.c_str());
            gtk_entry_set_text(GTK_ENTRY(mw->getWidget("ucLineEntry")), "");
            gtk_widget_grab_focus(mw->getWidget("ucLineEntry"));

            gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("ucLineDialog")));

            // Fix crash, if the dialog gets programmatically destroyed.
            if (response == GTK_RESPONSE_NONE)
                return false;

            gtk_widget_hide(mw->getWidget("ucLineDialog"));

            if (response == GTK_RESPONSE_OK)
                line = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("ucLineEntry")));

            if (!line.empty())
            {
                ucParams["line:" + name] = line;
                done[name] = line;
            }
            else
                return false;
        }
        i = j + 1;
    }

    return true;
}

void MainWindow::propertiesMagnetDialog_gui(string magnet)
{
    string name;
    int64_t size;
    string tth;

    if (WulforUtil::splitMagnet(magnet, name, size, tth))
        showMagnetDialog_gui(magnet, name, size, tth);
}

void MainWindow::showMagnetDialog_gui(const string &magnet, const string &name, const int64_t size, const string &tth)
{
    gtk_window_set_title(GTK_WINDOW(getWidget("MagnetDialog")), _("Magnet Properties / Choice"));
    // entry
    gtk_entry_set_text(GTK_ENTRY(getWidget("magnetEntry")), magnet.c_str());
    gtk_entry_set_text(GTK_ENTRY(getWidget("magnetNameEntry")), name.c_str());
    gtk_entry_set_text(GTK_ENTRY(getWidget("magnetSizeEntry")), Util::formatBytes(size).c_str());
    gtk_entry_set_text(GTK_ENTRY(getWidget("exactSizeEntry")), Util::formatExactSize(size).c_str());
    gtk_entry_set_text(GTK_ENTRY(getWidget("tthEntry")), tth.c_str());
    // chooser dialog
    GtkWidget *chooser = getWidget("flistDialog");
    gtk_window_set_title(GTK_WINDOW(chooser), _("Choose a directory"));
    gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());
    // choose
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("choiceCheckButton")), FALSE);
    setChooseMagnetDialog_gui();

    gtk_widget_show_all(getWidget("MagnetDialog"));
}

void MainWindow::setChooseMagnetDialog_gui()
{
    switch (WGETI("magnet-action"))
    {
        case 0: // start a search for this file
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("searchRadioButton")), TRUE);
            gtk_widget_set_sensitive(getWidget("browseButton"), FALSE);
        break;

        case 1: // add this file to your download queue
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("dowloadQueueRadioButton")), TRUE);
            gtk_widget_set_sensitive(getWidget("browseButton"), TRUE);
        break;

        default: // show magnet dialog
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("showRadioButton")), TRUE);
            gtk_widget_set_sensitive(getWidget("browseButton"), FALSE);
    }
}

void MainWindow::onBrowseMagnetButton_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    GtkWidget *dialog = mw->getWidget("flistDialog");
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    // if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(dialog);
}

void MainWindow::onDowloadQueueToggled_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    gtk_widget_set_sensitive(mw->getWidget("browseButton"), TRUE);
}

void MainWindow::onSearchMagnetToggled_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    gtk_widget_set_sensitive(mw->getWidget("browseButton"), FALSE);
}

void MainWindow::onSetMagnetChoiceDialog_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    // magnet choice frame
    gtk_window_set_title(GTK_WINDOW(mw->getWidget("MagnetDialog")), _("Magnet Choice"));
    gtk_widget_hide(mw->getWidget("setHBox"));
    gtk_widget_hide(mw->getWidget("magnetPropertiesFrame"));
    gtk_widget_hide(mw->getWidget("hseparator1"));

    // chooser dialog
    GtkWidget *chooser = mw->getWidget("flistDialog");
    gtk_window_set_title(GTK_WINDOW(chooser), _("Choose a directory"));
    gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::fromUtf8(WGETS("magnet-choose-dir")).c_str());

    mw->setChooseMagnetDialog_gui();

    gtk_widget_show(mw->getWidget("MagnetDialog"));
}

void MainWindow::onResponseMagnetDialog_gui(GtkWidget *dialog, gint response, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    if (response == GTK_RESPONSE_OK)
    {
        string path;

        // magnet choice frame
        if (!GTK_WIDGET_VISIBLE(mw->getWidget("magnetPropertiesFrame")))
        {
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("dowloadQueueRadioButton"))))
            {
                gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
                if (temp)
                {
                    path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
                    g_free(temp);
                }
                if (!File::isAbsolute(path))
                    path = SETTING(DOWNLOAD_DIRECTORY);

                WSET("magnet-choose-dir", path);
                WSET("magnet-action", 1);
            }
            else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("searchRadioButton"))))
                WSET("magnet-action", 0);
            else
                WSET("magnet-action", -1);

            gtk_widget_hide(dialog);

            return;
        }

        // magnet properties plus choice frame
        string name, tth;
        int64_t size;
        string magnet = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("magnetEntry")));

        WulforUtil::splitMagnet(magnet, name, size, tth);
        gboolean set = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("choiceCheckButton")));

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("dowloadQueueRadioButton"))))
        {
            gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(mw->getWidget("flistDialog")));
            if (temp)
            {
                path = Text::toUtf8(temp) + G_DIR_SEPARATOR_S;
                g_free(temp);
            }

            if (!File::isAbsolute(path))
                path = SETTING(DOWNLOAD_DIRECTORY);

            if (set)
            {
                WSET("magnet-action", 1);
                WSET("magnet-choose-dir", path);
            }

            // add this file to your download queue
            typedef Func3<MainWindow, string, int64_t, string> F3;
            F3 *func = new F3(mw, &MainWindow::addFileDownloadQueue_client, path + name, size, tth);
            WulforManager::get()->dispatchClientFunc(func);
        }
        else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->getWidget("searchRadioButton"))))
        {
            if (set)
                WSET("magnet-action", 0);

            // start a search for this file
            Search *s = mw->addSearch_gui();
            s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
        }
        else if (set)
            WSET("magnet-action", -1);
    }

    gtk_widget_hide(dialog);
}

void MainWindow::addFileDownloadQueue_client(string name, int64_t size, string tth)
{
    try
    {
        if (!tth.empty())
        {
            QueueManager::getInstance()->add(name, size, TTHValue(tth), UserPtr(), "");

            // automatically search for alternative download locations
            if (BOOLSETTING(AUTO_SEARCH))
                SearchManager::getInstance()->search(tth, 0, SearchManager::TYPE_TTH, SearchManager::SIZE_DONTCARE,
                    Util::emptyString);
        }
    }
    catch (const Exception& e)
    {
        // add error to main status
        typedef Func2<MainWindow, string, time_t> F2;
        F2 *func = new F2(this, &MainWindow::setMainStatus_gui, e.getError(), time(NULL));
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void MainWindow::showMessageDialog_gui(const string primaryText, const string secondaryText)
{
    if (primaryText.empty())
        return;

    GtkWidget* dialog = gtk_message_dialog_new(window, GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "%s", primaryText.c_str());

    if (!secondaryText.empty())
        gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondaryText.c_str());

    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
    gtk_widget_show(dialog);
}

gboolean MainWindow::onWindowState_gui(GtkWidget *widget, GdkEventWindowState *event, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    if (!mw->minimized && event->new_window_state & (GDK_WINDOW_STATE_ICONIFIED | GDK_WINDOW_STATE_WITHDRAWN))
    {
        mw->minimized = TRUE;
        if (BOOLSETTING(SettingsManager::AUTO_AWAY) && !Util::getAway())
            Util::setAway(TRUE);
    }
    else if (mw->minimized && (event->new_window_state & GDK_WINDOW_STATE_MAXIMIZED ||
        event->new_window_state == 0))
    {
        mw->minimized = FALSE;
        if (BOOLSETTING(SettingsManager::AUTO_AWAY) && !Util::getManualAway())
            Util::setAway(FALSE);
    }

    return TRUE;
}

gboolean MainWindow::onFocusIn_gui(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkWidget *child = mw->currentPage_gui();

    if (child != NULL)
    {
        BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");
        entry->setActive_gui();
    }

    gtk_window_set_urgency_hint(mw->window, FALSE);
    return FALSE;
}

gboolean MainWindow::onCloseWindow_gui(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    if (mw->onQuit)
    {
        mw->onQuit = FALSE;
    }
    else if (WGETB("main-window-no-close") && WGETB("always-tray"))
    {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconShowInterfaceItem")), FALSE);

        return TRUE;
    }

    if (!WGETB("confirm-exit"))
    {
        WulforManager::get()->deleteMainWindow();
        return FALSE;
    }

    gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("exitDialog")));
    gtk_widget_hide(mw->getWidget("exitDialog"));

    if (response == GTK_RESPONSE_OK)
    {
        WulforManager::get()->deleteMainWindow();
        return FALSE;
    }

    return TRUE;
}

gboolean MainWindow::onDeleteEventMagnetDialog_gui(GtkWidget *dialog, GdkEvent *event, gpointer data)
{
    gtk_widget_hide(dialog);

    return TRUE;
}

void MainWindow::onTopToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
   MainWindow *mw = (MainWindow *)data;

   GtkWidget *parent = mw->getWidget("hbox4");
   GtkWidget *child = mw->getWidget("toolbar1");
   if (child->parent != GTK_WIDGET(parent))
       return;
   g_object_ref(child);
   gtk_container_remove(GTK_CONTAINER(parent), child);
   parent = mw->getWidget("vbox1");
   gtk_toolbar_set_orientation(GTK_TOOLBAR(child), GTK_ORIENTATION_HORIZONTAL);
   gtk_box_pack_start(GTK_BOX(parent), child, FALSE, FALSE, 2);
   gtk_box_reorder_child(GTK_BOX(parent), child, 1);
   g_object_unref(child);
   WSET("toolbar-position", 0);
}

void MainWindow::onLeftToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
   MainWindow *mw = (MainWindow *)data;

   GtkWidget *parent = mw->getWidget("vbox1");
   GtkWidget *child = mw->getWidget("toolbar1");
   if (child->parent != GTK_WIDGET(parent))
       return;
   g_object_ref(child);
   gtk_container_remove(GTK_CONTAINER(parent), child);
   parent = mw->getWidget("hbox4");
   gtk_toolbar_set_orientation(GTK_TOOLBAR(child), GTK_ORIENTATION_VERTICAL);
   gtk_box_pack_start(GTK_BOX(parent), child, FALSE, FALSE, 2);
   gtk_box_reorder_child(GTK_BOX(parent), child, 0);
   g_object_unref(child);
   WSET("toolbar-position", 1);
}

void MainWindow::onHideToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
   MainWindow *mw = (MainWindow *)data;

   gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mw->getWidget("hideToolbarItem")));
   if (active)
   {
       gtk_widget_hide(mw->getWidget("toolbar1"));
       mw->ToolbarStyle = WGETI("toolbar-style");
       WSET("toolbar-style", 4);
   }
   else
   {
       gtk_widget_show(mw->getWidget("toolbar1"));
       WSET("toolbar-style", mw->ToolbarStyle);
   }
}

void MainWindow::onSizeToolbarToggled_gui(GtkWidget *widget, gpointer data)
{
   MainWindow *mw = (MainWindow *)data;

   gboolean active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mw->getWidget("sizeToolbarItem")));
   GtkWidget *toolbar = mw->getWidget("toolbar1");
   GtkIconSize size;
   if (active)
   {
       WSET("toolbar-small", TRUE);
       size = GTK_ICON_SIZE_SMALL_TOOLBAR;
   }
   else
   {
       WSET("toolbar-small", FALSE);
       size = GTK_ICON_SIZE_LARGE_TOOLBAR;
   }
   g_object_set(G_OBJECT(toolbar), "icon-size", size, NULL);
}

gboolean MainWindow::onAddButtonClicked_gui(GtkWidget *widget, gpointer data)
{
   MainWindow *mw = (MainWindow *)data;

   gtk_menu_popup(GTK_MENU(mw->getWidget("toolbarMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
   return FALSE;
}

void MainWindow::onToolToggled_gui(GtkWidget *widget, gpointer data)
{
   string key = (gchar*) g_object_get_data(G_OBJECT(widget), "key");
   GtkWidget *button = (GtkWidget*) data;
   bool active = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));
   active ? gtk_widget_show(button) : gtk_widget_hide(button);
   WSET(key, active);
}

void MainWindow::checkToolbarMenu_gui()
{
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("addMenuItemBar")), WGETB("toolbar-button-add"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("separatorsMenuItemBar")), WGETB("toolbar-button-separators"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("reconnectMenuItemBar")), WGETB("toolbar-button-reconnect"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("connectMenuItemBar")), WGETB("toolbar-button-connect"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("favHubsMenuItemBar")), WGETB("toolbar-button-fav-hubs"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("favUsersMenuItemBar")), WGETB("toolbar-button-fav-users"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("publicHubsMenuItemBar")), WGETB("toolbar-button-public-hubs"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("settingsMenuItemBar")), WGETB("toolbar-button-settings"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("own_filelistMenuItemBar")), WGETB("toolbar-button-own-filelist"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("refreshMenuItemBar")), WGETB("toolbar-button-refresh"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("hashMenuItemBar")), WGETB("toolbar-button-hash"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("searchMenuItemBar")), WGETB("toolbar-button-search"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("searchSpyMenuItemBar")), WGETB("toolbar-button-search-spy"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("searchADLMenuItemBar")), WGETB("toolbar-button-search-adl"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("queueMenuItemBar")), WGETB("toolbar-button-queue"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("finishedDownloadsMenuItemBar")), WGETB("toolbar-button-finished-downloads"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("finishedUploadsMenuItemBar")), WGETB("toolbar-button-finished-uploads"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("quitMenuItemBar")), WGETB("toolbar-button-quit"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("addMenuItemBar")), WGETB("toolbar-button-add"));
   gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(getWidget("hideToolbarItem")), ((ToolbarStyle = WGETI("toolbar-style")) == 4) ? TRUE : FALSE);
}

gboolean MainWindow::onKeyPressed_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    if (event->state & GDK_CONTROL_MASK)
    {
        if (event->state & GDK_SHIFT_MASK && event->keyval == GDK_ISO_Left_Tab)
        {
            mw->previousTab_gui();
            return TRUE;
        }
        else if (event->keyval == GDK_Tab)
        {
            mw->nextTab_gui();
            return TRUE;
        }
        else if (event->keyval == GDK_F4)
        {
            onCloseClicked_gui(widget, data);
            return TRUE;
        }
    }
    else if (event->state & GDK_MOD1_MASK) {
        if (event->keyval == GDK_1)
            mw->onSwitchOnPage_gui(1);
        else if (event->keyval == GDK_2)
            mw->onSwitchOnPage_gui(2);
        else if (event->keyval == GDK_3)
            mw->onSwitchOnPage_gui(3);
        else if (event->keyval == GDK_4)
            mw->onSwitchOnPage_gui(4);
        else if (event->keyval == GDK_5)
            mw->onSwitchOnPage_gui(5);
        else if (event->keyval == GDK_6)
            mw->onSwitchOnPage_gui(6);
        else if (event->keyval == GDK_7)
            mw->onSwitchOnPage_gui(7);
        else if (event->keyval == GDK_8)
            mw->onSwitchOnPage_gui(8);
        else if (event->keyval == GDK_9)
            mw->onSwitchOnPage_gui(9);
        else if (event->keyval == GDK_0)
            mw->onSwitchOnPage_gui(10);
    }


    return FALSE;
}
void MainWindow::onSwitchOnPage_gui(int pageNum){
    GtkNotebook *book = GTK_NOTEBOOK(getWidget("book"));
    gtk_notebook_set_current_page(book, pageNum-1);
}
gboolean MainWindow::onButtonReleasePage_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    gint width, height;
    gdk_drawable_get_size(event->window, &width, &height);

    // If middle mouse button was released when hovering over tab label
    if (event->button == 2 && event->x >= 0 && event->y >= 0
        && event->x < width && event->y < height)
    {
        BookEntry *entry = (BookEntry *)data;
        WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
        return TRUE;
    }

    return FALSE;
}

gboolean MainWindow::animationStatusIcon_gui(gpointer data)
{
    MainWindow *mw = (MainWindow *) data;

    if (mw->isActive_gui())
    {
        gtk_status_icon_set_from_icon_name(mw->statusIcon, "eiskaltdcpp");
        mw->timer = 0;

        return FALSE;
    }

    gtk_status_icon_set_from_icon_name(mw->statusIcon, (mw->statusFrame *= -1) > 0 ? "eiskaltdcpp" : "icon_msg");

    return TRUE;
}

void MainWindow::onRaisePage_gui(GtkMenuItem *item, gpointer data)
{
    WulforManager::get()->getMainWindow()->raisePage_gui((GtkWidget *)data);
}

void MainWindow::onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
    MainWindow* mw = (MainWindow*)data;
    GtkWidget *child = gtk_notebook_get_nth_page(notebook, num);
    BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(child), "entry");

    if (entry)
    {
        // Disable "activate" signal on the tab menu item since it can cause
        // onPageSwitched_gui to be called multiple times
        GtkWidget *item = entry->getTabMenuItem();
        g_signal_handlers_block_by_func(item, (gpointer)onRaisePage_gui, child);

        entry->setActive_gui();
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(entry->getTabMenuItem()), TRUE);
        mw->setTitle(entry->getLabelText()); // Update window title with selected tab label

        g_signal_handlers_unblock_by_func(item, (gpointer)onRaisePage_gui, (gpointer)child);
    }

    GList *list = (GList *)g_object_get_data(G_OBJECT(notebook), "page-rotation-list");
    list = g_list_remove(list, (gpointer)child);
    list = g_list_prepend(list, (gpointer)child);
    g_object_set_data(G_OBJECT(notebook), "page-rotation-list", (gpointer)list);

    // Focus the tab so it will focus its children (e.g. a text entry box)
    gtk_widget_grab_focus(child);
}

void MainWindow::onPaneRealized_gui(GtkWidget *pane, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    gint position = WGETI("transfer-pane-position");

    if (position > 10)
    {
        // @todo: fix get window height when maximized
        gint height;
        gtk_window_get_size(mw->window, NULL, &height);
        gtk_paned_set_position(GTK_PANED(pane), height - position);
    }
}

void MainWindow::onConnectClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    gtk_editable_select_region(GTK_EDITABLE(mw->getWidget("connectEntry")), 0, -1);
    gtk_widget_grab_focus(mw->getWidget("connectEntry"));

    gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("connectDialog")));

    // Fix crash, if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(mw->getWidget("connectDialog"));

    if (response == GTK_RESPONSE_OK)
    {
        string address = gtk_entry_get_text(GTK_ENTRY(mw->getWidget("connectEntry")));
        mw->showHub_gui(address);
    }
}

void MainWindow::onFavoriteHubsClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showFavoriteHubs_gui();
}

void MainWindow::onFavoriteUsersClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showFavoriteUsers_gui();
}

void MainWindow::onPublicHubsClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showPublicHubs_gui();
}

void MainWindow::onPreferencesClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    typedef Func2<MainWindow, bool, int> F2;
    unsigned short tcpPort = (unsigned short)SETTING(TCP_PORT);
    unsigned short udpPort = (unsigned short)SETTING(UDP_PORT);
    int lastConn = SETTING(INCOMING_CONNECTIONS);
    bool onstart = false;
    if (mw->useStatusIconBlink != WGETB("status-icon-blink-use"))
        WSET("status-icon-blink-use", mw->useStatusIconBlink);
    bool emoticons = WGETB("emoticons-use");

    gint response = WulforManager::get()->openSettingsDialog_gui();

    if (response == GTK_RESPONSE_OK)
    {
        F2 *func = new F2(mw, &MainWindow::startSocket_client, onstart, lastConn);
        WulforManager::get()->dispatchClientFunc(func);

        if (WGETB("always-tray"))
            gtk_status_icon_set_visible(mw->statusIcon, TRUE);
        else
            gtk_status_icon_set_visible(mw->statusIcon, FALSE);

        mw->setTabPosition_gui(WGETI("tab-position"));
        mw->setToolbarStyle_gui(WGETI("toolbar-style"));

        // Reload the icons only if the setting has changed
        mw->loadIcons_gui();

        // All hubs and PMs
        for (StringIterC it = mw->EntryList.begin(); it != mw->EntryList.end(); ++it)
        {
            BookEntry *entry = mw->findBookEntry(Entry::HUB, *it);

            if (entry != NULL)
                dynamic_cast<Hub*>(entry)->preferences_gui();
            else
            {
                entry = mw->findBookEntry(Entry::PRIVATE_MESSAGE, *it);

                if (entry != NULL)
                    dynamic_cast<PrivateMessage*>(entry)->preferences_gui();
            }
        }

        // Search Spy
        BookEntry *entry = mw->findBookEntry(Entry::SEARCH_SPY);

        if (entry != NULL)
            dynamic_cast<SearchSpy *>(entry)->preferences_gui();

        // Status menu
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconBlinkUseItem")), WGETB("status-icon-blink-use"));

        // Emoticons
        if (emoticons != WGETB("emoticons-use"))
            Emoticons::get()->reloadPack_gui();

        // Toolbar
        mw->checkToolbarMenu_gui();
    }
}

void MainWindow::onTransferToggled_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkWidget *transfer = mw->transfers->getContainer();

    if (GTK_WIDGET_VISIBLE(transfer))
        gtk_widget_hide(transfer);
    else
        gtk_widget_show_all(transfer);
}

void MainWindow::onHashClicked_gui(GtkWidget *widget, gpointer data)
{
    WulforManager::get()->openHashDialog_gui();
}

void MainWindow::onSearchClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->addSearch_gui();
}

void MainWindow::onSearchSpyClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showSearchSpy_gui();
}

void MainWindow::onSearchADLClicked_gui(GtkWidget *widget, gpointer data)
{
        MainWindow *mw = (MainWindow *)data;
        mw->showSearchADL_gui();
}

void MainWindow::onDownloadQueueClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showDownloadQueue_gui();
}

void MainWindow::onFinishedDownloadsClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showFinishedDownloads_gui();
}

void MainWindow::onFinishedUploadsClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->showFinishedUploads_gui();
}

void MainWindow::onQuitClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    // fix emit signal (status-icon menu quit)
    if (GTK_WIDGET_VISIBLE(mw->getWidget("exitDialog")))
        return;

    mw->onQuit = TRUE;
    gboolean retVal; // Not interested in the value, though.
    g_signal_emit_by_name(mw->window, "delete-event", NULL, &retVal);
}

void MainWindow::onOpenFileListClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;

    GtkWidget *chooser = mw->getWidget("flistDialog");
    gtk_window_set_title(GTK_WINDOW(chooser), _("Select filelist to browse"));
    gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), Text::fromUtf8(Util::getListPath()).c_str());

    gint response = gtk_dialog_run(GTK_DIALOG(chooser));

    // if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(chooser);

    if (response == GTK_RESPONSE_OK)
    {
        gchar *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));

        if (temp)
        {
            string path = Text::toUtf8(temp);
            g_free(temp);

            UserPtr user = DirectoryListing::getUserFromFilename(path);
            if (user)
                mw->showShareBrowser_gui(user, path, "", FALSE);
            else
                mw->setMainStatus_gui(_("Unable to load file list: Invalid file list name"));
        }
    }
}

void MainWindow::onOpenOwnListClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    typedef Func1<MainWindow, bool> F1;
    F1 *func = new F1(mw, &MainWindow::openOwnList_client, FALSE);
    WulforManager::get()->dispatchClientFunc(func);

    mw->setMainStatus_gui(_("Loading file list"));
}

void MainWindow::onRefreshFileListClicked_gui(GtkWidget *widget, gpointer data)
{
    typedef Func0<MainWindow> F0;
    F0 *func = new F0((MainWindow *)data, &MainWindow::refreshFileList_client);
    WulforManager::get()->dispatchClientFunc(func);
}

void MainWindow::onReconnectClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkWidget *entryWidget = mw->currentPage_gui();

    if (entryWidget)
    {
        BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

        if (entry && entry->getType() == Entry::HUB)
        {
            Func0<Hub> *func = new Func0<Hub>(dynamic_cast<Hub *>(entry), &Hub::reconnect_client);
            WulforManager::get()->dispatchClientFunc(func);
        }
    }
}

void MainWindow::onCloseClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkWidget *entryWidget = mw->currentPage_gui();

    if (entryWidget)
    {
        BookEntry *entry = (BookEntry *)g_object_get_data(G_OBJECT(entryWidget), "entry");

        if (entry)
            mw->removeBookEntry_gui(entry);
    }
}

void MainWindow::onPreviousTabClicked_gui(GtkWidget* widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->previousTab_gui();
}

void MainWindow::onNextTabClicked_gui(GtkWidget* widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->nextTab_gui();
}

void MainWindow::onAboutClicked_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    gint response = gtk_dialog_run(GTK_DIALOG(mw->getWidget("aboutDialog")));

    // Fix crash, if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(mw->getWidget("aboutDialog"));
}

void MainWindow::onAboutDialogActivateLink_gui(GtkAboutDialog *dialog, const gchar *link, gpointer data)
{
    WulforUtil::openURI(link);
}

void MainWindow::onCloseBookEntry_gui(GtkWidget *widget, gpointer data)
{
    BookEntry *entry = (BookEntry *)data;
    WulforManager::get()->getMainWindow()->removeBookEntry_gui(entry);
}

void MainWindow::onStatusIconActivated_gui(GtkStatusIcon *statusIcon, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkCheckMenuItem *item = GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconShowInterfaceItem"));

    // Toggle the "Show Interface" check menu item. This will in turn invoke its callback.
    gboolean active = gtk_check_menu_item_get_active(item);
    gtk_check_menu_item_set_active(item, !active);
}

void MainWindow::onStatusIconPopupMenu_gui(GtkStatusIcon *statusIcon, guint button, guint time, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkMenu *menu = GTK_MENU(mw->getWidget("statusIconMenu"));
    gtk_menu_popup(menu, NULL, NULL, gtk_status_icon_position_menu, statusIcon, button, time);
}

void MainWindow::onShowInterfaceToggled_gui(GtkCheckMenuItem *item, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    GtkWindow *win = mw->window;
    static int x, y;
    static bool isMaximized, isIconified;

    if (GTK_WIDGET_VISIBLE(win))
    {
        GdkWindowState state;
        gtk_window_get_position(win, &x, &y);
        state = gdk_window_get_state(GTK_WIDGET(win)->window);
        isMaximized = (state & GDK_WINDOW_STATE_MAXIMIZED);
        isIconified = (state & GDK_WINDOW_STATE_ICONIFIED);
        gtk_widget_hide(GTK_WIDGET(win));
    }
    else
    {
        gtk_window_move(win, x, y);
        if (isMaximized) gtk_window_maximize(win);
        if (isIconified) gtk_window_iconify(win);
        gtk_widget_show(GTK_WIDGET(win));
    }
}

void MainWindow::onStatusIconBlinkUseToggled_gui(GtkWidget *widget, gpointer data)
{
    MainWindow *mw = (MainWindow *)data;
    mw->removeTimerSource_gui();

    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mw->getWidget("statusIconBlinkUseItem"))))

        mw->useStatusIconBlink = TRUE;
    else
        mw->useStatusIconBlink = FALSE;
}

void MainWindow::onLinkClicked_gui(GtkWidget *widget, gpointer data)
{
    string link = (gchar*) g_object_get_data(G_OBJECT(widget), "link");
    WulforUtil::openURI(link);
}

void MainWindow::autoConnect_client()
{
    FavoriteHubEntry *hub;
    FavoriteHubEntryList &l = FavoriteManager::getInstance()->getFavoriteHubs();
    typedef Func2<MainWindow, string, string> F2;
    F2 *func;

    for (FavoriteHubEntryList::const_iterator it = l.begin(); it != l.end(); ++it)
    {
        hub = *it;

        if (hub->getConnect())
        {
            func = new F2(this, &MainWindow::showHub_gui, hub->getServer(), hub->getEncoding());
            WulforManager::get()->dispatchGuiFunc(func);
        }
    }

    string link = WulforManager::get()->getURL();

    if (link.empty()) return;

    typedef Func1<MainWindow, string> F1;
    F1 *func1;

    if (WulforUtil::isHubURL(link) && WGETB("urlhandler"))
    {
        func = new F2(this, &MainWindow::showHub_gui, link, "");
        WulforManager::get()->dispatchGuiFunc(func);
    }
    else if (WulforUtil::isMagnet(link) && WGETB("magnet-register"))
    {
        func1 = new F1(this, &MainWindow::actionMagnet_gui, link);
        WulforManager::get()->dispatchGuiFunc(func1);
    }
}

void MainWindow::startSocket_client(bool onstart, int oldmode){
    if (onstart) {
        try {
        ConnectivityManager::getInstance()->setup(true, SettingsManager::INCOMING_DIRECT);
        } catch (const Exception& e) {
            showPortsError(e.getError());
        }
    } else {
        try {
            ConnectivityManager::getInstance()->setup(true, oldmode);
        } catch (const Exception& e) {
            showPortsError(e.getError());
        }
    }
    ClientManager::getInstance()->infoUpdated();
}
void MainWindow::showPortsError(const string& port) {
    string msg = str(F_("Unable to open %1% port. Searching or file transfers will not work correctly until you change settings or turn off any application that might be using that port.") % port);
    typedef Func2<MainWindow, string, string> F2;
    F2* func = new F2(this, &MainWindow::showMessageDialog_gui, _("Connectivity Manager: Warning"), msg);
    WulforManager::get()->dispatchGuiFunc(func);
}
void MainWindow::refreshFileList_client()
{
    try
    {
        ShareManager::getInstance()->setDirty();
        ShareManager::getInstance()->refresh(TRUE, TRUE, FALSE);
    }
    catch (const ShareException&)
    {
    }
}

void MainWindow::openOwnList_client(bool useSetting)
{
    UserPtr user = ClientManager::getInstance()->getMe();
    string path = ShareManager::getInstance()->getOwnListFile();

    typedef Func4<MainWindow, UserPtr, string, string, bool> F4;
    F4 *func = new F4(this, &MainWindow::showShareBrowser_gui, user, path, "", useSetting);
    WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(LogManagerListener::Message, time_t t, const string &message) throw()
{
    typedef Func2<MainWindow, string, time_t> F2;
    F2 *func = new F2(this, &MainWindow::setMainStatus_gui, message, t);
    WulforManager::get()->dispatchGuiFunc(func);
}

void MainWindow::on(QueueManagerListener::Finished, QueueItem *item, const string& dir, int64_t avSpeed) throw()
{
    typedef Func3<MainWindow, string, string, Notify::TypeNotify> F3;

    if (item->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
    {
        UserPtr user = item->getDownloads()[0]->getUser();
        string listName = item->getListName();

        F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("file list from "), WulforUtil::getNicks(user),
            Notify::DOWNLOAD_FINISHED_USER_LIST);
        WulforManager::get()->dispatchGuiFunc(f3);

        typedef Func4<MainWindow, UserPtr, string, string, bool> F4;
        F4 *func = new F4(this, &MainWindow::showShareBrowser_gui, user, listName, dir, TRUE);
        WulforManager::get()->dispatchGuiFunc(func);
    }
    else if (!item->isSet(QueueItem::FLAG_XML_BZLIST))
    {
        F3 *f3 = new F3(this, &MainWindow::showNotification_gui, _("<b>file:</b> "), item->getTarget(), Notify::DOWNLOAD_FINISHED);
        WulforManager::get()->dispatchGuiFunc(f3);
    }
}

void MainWindow::on(TimerManagerListener::Second, uint32_t ticks) throw()
{
    // Avoid calculating status update if it's not needed
    if (!WGETB("always-tray") && minimized)
        return;

    int64_t diff = (int64_t)((lastUpdate == 0) ? ticks - 1000 : ticks - lastUpdate);
    int64_t downBytes = 0;
    int64_t upBytes = 0;

    if (diff > 0)
    {
        int64_t downDiff = Socket::getTotalDown() - lastDown;
        int64_t upDiff = Socket::getTotalUp() - lastUp;
        downBytes = (downDiff * 1000) / diff;
        upBytes = (upDiff * 1000) / diff;
    }

    string hubs = Client::getCounts();
    string downloadSpeed = Util::formatBytes(downBytes) + "/" + _("s");
    string downloaded = Util::formatBytes(Socket::getTotalDown());
    string uploadSpeed = Util::formatBytes(upBytes) + "/" + _("s");
    string uploaded = Util::formatBytes(Socket::getTotalUp());

    lastUpdate = ticks;
    lastUp = Socket::getTotalUp();
    lastDown = Socket::getTotalDown();

    typedef Func5<MainWindow, string, string, string, string, string> F5;
    F5 *func = new F5(this, &MainWindow::setStats_gui, hubs, downloadSpeed, downloaded, uploadSpeed, uploaded);
    WulforManager::get()->dispatchGuiFunc(func);

    if (WGETB("always-tray") && !downloadSpeed.empty() && !uploadSpeed.empty())
    {
        typedef Func2<MainWindow, string, string> F2;
        F2 *f2 = new F2(this, &MainWindow::updateStatusIconTooltip_gui, downloadSpeed, uploadSpeed);
        WulforManager::get()->dispatchGuiFunc(f2);
    }
}
void MainWindow::onTTHFileDialog_gui(GtkWidget *widget, gpointer data)
{
	MainWindow *mw =(MainWindow *)data;
	GtkWidget *dialog = mw->getWidget("TTHFileDialog");
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if(response == GTK_RESPONSE_NONE)
			return;
	gtk_widget_hide(dialog);

}

void MainWindow::onTTHFileButton_gui(GtkWidget *widget , gpointer data)
{
	MainWindow *mw =(MainWindow *)data;
	GtkWidget *chooser=mw->getChooserDialog_gui();
	gtk_window_set_title(GTK_WINDOW(chooser), _("Select file to Get TTH"));
	gtk_file_chooser_set_action(GTK_FILE_CHOOSER(chooser), GTK_FILE_CHOOSER_ACTION_OPEN);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), "/home/");

	gint response = gtk_dialog_run(GTK_DIALOG(chooser));

	// if the dialog gets programmatically destroyed.
	if (response == GTK_RESPONSE_NONE)
		return;

	gtk_widget_hide(chooser);

	if (response == GTK_RESPONSE_OK)
	{
		gchar *temp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
		string TTH;
		char *buf = new char[512*1024];
		try {
		File f(Text::fromT(string(temp)),File::READ, File::OPEN);
		TigerTree tth(TigerTree::calcBlockSize(f.getSize(), 1));
		if(f.getSize() > 0) {
				size_t n = 512*1024;
				while( (n = f.read(&buf[0], n)) > 0) {
					tth.update(&buf[0], n);
					n = 512*1024;
				}
		} else {
			tth.update("", 0);
		}
		tth.finalize();

		strcpy(&TTH[0], tth.getRoot().toBase32().c_str());
		string magnetlink = "magnet:?xt=urn:tree:tiger:"+ TTH +"&xl="+Util::toString(f.getSize())+"&dn="+Util::encodeURI(Text::fromT(Util::getFileName(string(temp))));
		f.close();
		g_print("%s",TTH.c_str());
		gtk_entry_set_text(GTK_ENTRY(mw->getWidget("entrymagnet")),magnetlink.c_str());
		gtk_entry_set_text(GTK_ENTRY(mw->getWidget("entrytthfileresult")),TTH.c_str());
		} catch(...) { }
	}
}
