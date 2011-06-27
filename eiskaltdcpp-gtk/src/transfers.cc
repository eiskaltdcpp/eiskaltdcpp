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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <iterator>
#include "transfers.hh"
#include "WulforUtil.hh"
#include "wulformanager.hh"
#include "settingsmanager.hh"
#include "func.hh"
#include "previewmenu.hh"
#include "UserCommandMenu.hh"
#include <dcpp/Download.h>
#include <dcpp/Upload.h>
#include <dcpp/ClientManager.h>
#include <dcpp/TimerManager.h>
#include <dcpp/FavoriteManager.h>

using namespace std;
using namespace dcpp;

Transfers::Transfers() :
    Entry(Entry::TRANSFERS, "transfers.ui")
{
    // menu
    g_object_ref_sink(getWidget("transferMenu"));

    // Initialize the user command menu
    userCommandMenu = new UserCommandMenu(getWidget("userCommandMenu"), ::UserCommand::CONTEXT_USER);//NOTE: core 0.762
    addChild(userCommandMenu);

    // Initialize the preview menu
    appsPreviewMenu = new PreviewMenu(getWidget("appsPreviewMenu"));

    // Initialize transfer treeview
    transferView.setView(GTK_TREE_VIEW(getWidget("transfers")), TRUE, "transfers");
    transferView.insertColumn(_("User"), G_TYPE_STRING, TreeView::ICON_STRING, 150, "Icon");
    transferView.insertColumn(_("Hub Name"), G_TYPE_STRING, TreeView::STRING, 100);
    transferView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::PROGRESS, 250, "Progress");
    transferView.insertColumn(_("Time Left"), G_TYPE_INT64, TreeView::TIME_LEFT, 85);
    transferView.insertColumn(_("Speed"), G_TYPE_INT64, TreeView::SPEED, 125);
    transferView.insertColumn(_("Filename"), G_TYPE_STRING, TreeView::STRING, 200);
    transferView.insertColumn(_("Size"), G_TYPE_INT64, TreeView::SIZE, 125);
    transferView.insertColumn(_("Path"), G_TYPE_STRING, TreeView::STRING, 200);
    transferView.insertColumn(_("IP"), G_TYPE_STRING, TreeView::STRING, 175);
    transferView.insertHiddenColumn("Icon", G_TYPE_STRING);
    transferView.insertHiddenColumn("Progress", G_TYPE_INT);
    transferView.insertHiddenColumn("Sort Order", G_TYPE_STRING);
    transferView.insertHiddenColumn("CID", G_TYPE_STRING);
    transferView.insertHiddenColumn("Download Position", G_TYPE_INT64); // For keeping track of and calculating parent pos
    transferView.insertHiddenColumn("Failed", G_TYPE_BOOLEAN);
    transferView.insertHiddenColumn("Target", G_TYPE_STRING);
    transferView.insertHiddenColumn("tmpTarget", G_TYPE_STRING);
    transferView.insertHiddenColumn("Download", G_TYPE_BOOLEAN);
    transferView.insertHiddenColumn("Hub URL", G_TYPE_STRING);
    transferView.finalize();

    transferStore = gtk_tree_store_newv(transferView.getColCount(), transferView.getGTypes());
    gtk_tree_view_set_model(transferView.get(), GTK_TREE_MODEL(transferStore));
    g_object_unref(transferStore);
    transferSelection = gtk_tree_view_get_selection(transferView.get());
    gtk_tree_selection_set_mode(transferSelection, GTK_SELECTION_MULTIPLE);
    transferView.setSortColumn_gui(_("User"), "Sort Order");
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(transferStore), transferView.col("Sort Order"), GTK_SORT_ASCENDING);
    gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(transferView.get(), transferView.col(_("User"))), TRUE);
    gtk_tree_view_set_fixed_height_mode(transferView.get(), TRUE);

    g_signal_connect(transferView.get(), "button-press-event", G_CALLBACK(onTransferButtonPressed_gui), (gpointer)this);
    g_signal_connect(transferView.get(), "button-release-event", G_CALLBACK(onTransferButtonReleased_gui), (gpointer)this);
    g_signal_connect(getWidget("getFileListItem"), "activate", G_CALLBACK(onGetFileListClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("sendPrivateMessageItem"), "activate", G_CALLBACK(onPrivateMessageClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("addToFavoritesItem"), "activate", G_CALLBACK(onAddFavoriteUserClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("grantExtraSlotItem"), "activate", G_CALLBACK(onGrantExtraSlotClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeUserItem"), "activate", G_CALLBACK(onRemoveUserFromQueueClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("forceAttemptItem"), "activate", G_CALLBACK(onForceAttemptClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("closeConnectionItem"), "activate", G_CALLBACK(onCloseConnectionClicked_gui), (gpointer)this);
}

Transfers::~Transfers()
{
    QueueManager::getInstance()->removeListener(this);
    DownloadManager::getInstance()->removeListener(this);
    UploadManager::getInstance()->removeListener(this);
    ConnectionManager::getInstance()->removeListener(this);

    g_object_unref(getWidget("transferMenu"));
    delete appsPreviewMenu;
}

void Transfers::show()
{
    QueueManager::getInstance()->addListener(this);
    DownloadManager::getInstance()->addListener(this);
    UploadManager::getInstance()->addListener(this);
    ConnectionManager::getInstance()->addListener(this);
}

void Transfers::popupTransferMenu_gui()
{
    // Build user command menu
    appsPreviewMenu->cleanMenu_gui();
    userCommandMenu->cleanMenu_gui();

    GtkTreePath *path;
    GtkTreeIter iter;
    GList *list = gtk_tree_selection_get_selected_rows(transferSelection, NULL);
    string target = "";

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(transferStore), &iter);

            do
            {
                if (target.empty())
                    target = transferView.getString(&iter, "tmpTarget");

                string cid = transferView.getString(&iter, "CID");
                string hubUrl = transferView.getString(&iter, "Hub URL");//NOTE: core 0.762
                userCommandMenu->addUser(cid);
                userCommandMenu->addHub(WulforUtil::getHubAddress(CID(cid), hubUrl));//NOTE: core 0.762
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);

    userCommandMenu->buildMenu_gui();

    if (appsPreviewMenu->buildMenu_gui(target))
        gtk_widget_set_sensitive(getWidget("appsPreviewItem"), TRUE);
    else gtk_widget_set_sensitive(getWidget("appsPreviewItem"), FALSE);

    gtk_menu_popup(GTK_MENU(getWidget("transferMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
    gtk_widget_show_all(getWidget("transferMenu"));
}

void Transfers::onGetFileListClicked_gui(GtkMenuItem *item, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func2<Transfers, string, string> F2;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                string cid = tr->transferView.getString(&iter, "CID");
                string hubUrl = tr->transferView.getString(&iter, "Hub URL");
                if (!cid.empty())
                {
                    F2 *func = new F2(tr, &Transfers::getFileList_client, cid, hubUrl);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func2<Transfers, string, string> F2;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                string cid = tr->transferView.getString(&iter, "CID");
                string hubUrl = tr->transferView.getString(&iter, "Hub URL");
                if (!cid.empty())
                {
                    F2 *func = new F2(tr, &Transfers::matchQueue_client, cid, hubUrl);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    string cid;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                cid = tr->transferView.getString(&iter, "CID");
                if (!cid.empty())
                    WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, cid);
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    string cid;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func1<Transfers, string > F1;
    F1 *func;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                cid = tr->transferView.getString(&iter, "CID");
                if (!cid.empty())
                {
                    func = new F1(tr, &Transfers::addFavoriteUser_client, cid);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func2<Transfers, string, string> F2;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                string cid = tr->transferView.getString(&iter, "CID");
                string hubUrl = tr->transferView.getString(&iter, "Hub URL");
                if (!cid.empty())
                {
                    F2 *func = new F2(tr, &Transfers::grantExtraSlot_client, cid, hubUrl);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    string cid;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func1<Transfers, string > F1;
    F1 *func;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                cid = tr->transferView.getString(&iter, "CID");
                if (!cid.empty())
                {
                    func = new F1(tr, &Transfers::removeUserFromQueue_client, cid);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onForceAttemptClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    string cid;
    GtkTreeIter iter;
    GtkTreePath *path;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func1<Transfers, string> F1;
    F1 *func;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            cid = tr->transferView.getString(&iter, "CID");
            gtk_tree_store_set(tr->transferStore, &iter, tr->transferView.col(_("Status")), _("Connecting (forced)..."), -1);

            func = new F1(tr, &Transfers::forceAttempt_client, cid);
            WulforManager::get()->dispatchClientFunc(func);
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void Transfers::onCloseConnectionClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    string cid;
    GtkTreeIter iter;
    GtkTreePath *path;
    bool download;
    GList *list = gtk_tree_selection_get_selected_rows(tr->transferSelection, NULL);
    typedef Func2<Transfers, string, bool> F2;
    F2 *func;

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(tr->transferStore), &iter, path))
        {
            bool parent = gtk_tree_model_iter_has_child(GTK_TREE_MODEL(tr->transferStore), &iter);

            do
            {
                cid = tr->transferView.getString(&iter, "CID");
                if (!cid.empty())
                {
                    gtk_tree_store_set(tr->transferStore, &iter, tr->transferView.col(_("Status")), _("Closing connection..."), -1);
                    download = tr->transferView.getValue<gboolean>(&iter, "Download");

                    func = new F2(tr, &Transfers::closeConnection_client, cid, download);
                    WulforManager::get()->dispatchClientFunc(func);
                }
            }
            while (parent && WulforUtil::getNextIter_gui(GTK_TREE_MODEL(tr->transferStore), &iter, TRUE, FALSE));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

gboolean Transfers::onTransferButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    Transfers *tr = (Transfers *)data;

    if (event->button == 3)
    {
        GtkTreePath *path;
        if (gtk_tree_view_get_path_at_pos(tr->transferView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
        {
            bool selected = gtk_tree_selection_path_is_selected(tr->transferSelection, path);
            gtk_tree_path_free(path);

            if (selected)
                return TRUE;
        }
    }

    return FALSE;
}

gboolean Transfers::onTransferButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    Transfers *tr = (Transfers *)data;
    int count = gtk_tree_selection_count_selected_rows(tr->transferSelection);

    if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
        tr->popupTransferMenu_gui();

    return FALSE;
}

bool Transfers::findParent_gui(const string& target, GtkTreeIter* iter)
{
    dcassert(!target.empty());
    GtkTreeModel *m = GTK_TREE_MODEL(transferStore);
    bool valid = gtk_tree_model_get_iter_first(m, iter);

    while (valid)
    {
        if (transferView.getValue<gboolean>(iter, "Download") &&
                target == transferView.getString(iter, "Target") &&
                transferView.getString(iter, "CID").empty())
            return TRUE;

        valid = WulforUtil::getNextIter_gui(m, iter, FALSE, FALSE);
    }

    return FALSE;
}

bool Transfers::findTransfer_gui(const string& cid, bool download, GtkTreeIter* iter)
{
    GtkTreeModel *m = GTK_TREE_MODEL(transferStore);
    bool valid = gtk_tree_model_get_iter_first(m, iter);

    while (valid)
    {
        if (cid == transferView.getString(iter, "CID") && !cid.empty())
        {
            if (download && transferView.getValue<gboolean>(iter, "Download"))
                return TRUE;
            if (!download && !transferView.getValue<gboolean>(iter, "Download"))
                return TRUE;
        }
        valid = WulforUtil::getNextIter_gui(m, iter, TRUE, TRUE);
    }

    return FALSE;
}

void Transfers::addConnection_gui(StringMap params, bool download)
{
    GtkTreeIter iter;
    dcassert(params.find("CID") != params.end());
    dcassert(findTransfer_gui(params["CID"], download, &iter) == FALSE);    // shouldn't fail, if it's already there we've forgot to remove it or dcpp core sends more than one Connection::Added

    gtk_tree_store_append(transferStore, &iter, NULL);
    gtk_tree_store_set(transferStore, &iter,
        transferView.col(_("User")), params["User"].c_str(),
        transferView.col(_("Hub Name")), params["Hub Name"].c_str(),
        transferView.col(_("Status")), params["Status"].c_str(),
        transferView.col("CID"), params["CID"].c_str(),
        transferView.col("Icon"), download ? "icon-download" : "icon-upload",
        transferView.col("Download"), download,
        transferView.col("Hub URL"), params["Hub URL"].c_str(),
        -1);
}

void Transfers::removeConnection_gui(const string cid, bool download)
{
    GtkTreeIter iter;
    GtkTreeIter parent;
    bool valid = findTransfer_gui(cid, download, &iter);

    if (valid)
    {
        if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(transferStore), &parent, &iter))
        {
            gtk_tree_store_remove(transferStore, &iter);

            if (!gtk_tree_model_iter_has_child(GTK_TREE_MODEL(transferStore), &parent))
                gtk_tree_store_remove(transferStore, &parent);
            else
                updateParent_gui(&parent);
        }
        else
        {
            gtk_tree_store_remove(transferStore, &iter);
        }
    }
}

void Transfers::updateParent_gui(GtkTreeIter* iter)
{
    int active = 0;
    GtkTreeIter child;
    string users;
    set<string> hubs;
    bool valid;
    int64_t speed = 0;
    int64_t position = 0;
    int64_t totalSize = 0;
    int64_t timeLeft = 0;
    double progress = 0.0;
    ostringstream stream;
    ostringstream tmpHubs;

    position = transferView.getValue<int64_t>(iter, "Download Position");
    totalSize = transferView.getValue<int64_t>(iter, _("Size"));

    // Get Totals
    if (gtk_tree_model_iter_has_child(GTK_TREE_MODEL(transferStore), iter))
    {
        child = *iter;
        valid = WulforUtil::getNextIter_gui(GTK_TREE_MODEL(transferStore), &child, TRUE, FALSE);
        while (valid)
        {
            if (transferView.getValue<int>(&child, "Failed") == 0 &&
                transferView.getString(&child, "Sort Order").substr(0,1) == "d")
            {
                active++;
                speed += transferView.getValue<int64_t>(&child, _("Speed"));
                position += transferView.getValue<int64_t>(&child, "Download Position");
            }
            users += transferView.getString(&child, _("User")) + string(", ");
            hubs.insert(transferView.getString(&child, _("Hub Name")));
            valid = WulforUtil::getNextIter_gui(GTK_TREE_MODEL(transferStore), &child, TRUE, FALSE);
        }
    }

    if (totalSize > 0)
        progress = (double)(position * 100.0) / totalSize;
    if (speed > 0)
        timeLeft = (totalSize - position) / speed;

    stream << setiosflags(ios::fixed) << setprecision(1);

    if (transferView.getValue<gboolean>(iter, "Failed") == 0)
    {
        if (active)
            stream << _("Downloaded ");
        else
            stream << _("Waiting for slot ");

        stream << Util::formatBytes(position) << " (" << progress;
        stream << _("%) from ") << active << "/" << gtk_tree_model_iter_n_children(GTK_TREE_MODEL(transferStore), iter) << _(" user(s)");
    }
    else
    {
        stream << transferView.getString(iter, _("Status"));
    }

    std::copy(hubs.begin(), hubs.end(), std::ostream_iterator<string>(tmpHubs, ", "));

    gtk_tree_store_set(transferStore, iter,
        transferView.col(_("User")), users.substr(0, users.length()-2).c_str(),
        transferView.col(_("Hub Name")), tmpHubs.str().substr(0, tmpHubs.str().length()-2).c_str(),
        transferView.col(_("Speed")), speed,
        transferView.col(_("Time Left")), timeLeft,
        transferView.col(_("Status")), stream.str().c_str(),
        transferView.col("Progress"), static_cast<int>(progress),
        transferView.col("Sort Order"), active ? (string("d").append(users)).c_str() : (string("w").append(users)).c_str(),
        -1);
}

void Transfers::updateTransfer_gui(StringMap params, bool download, Sound::TypeSound sound)
{
    dcassert(!params["CID"].empty());

    GtkTreeIter iter, parent;
    if (!findTransfer_gui(params["CID"], download, &iter))
    {
        int invalid_transfer_CID_not_found = 0;
        dcdebug(_("Transfers::updateTransfer, CID not found %s\n"), params["CID"].c_str());
        // Transfer not found. Usually this *shouldn't* happen, but I guess it's possible since tick updates are sent by TimerManager
        // and removing is handled by dl manager.

        return;
    }

    int failed = transferView.getValue<int>(&iter, "Failed");
    if (failed && params.find("Failed") != params.end())
        failed = Util::toInt(params["Failed"]);

    if (failed) // Transfer had failed already. We won't update the transfer before the fail status changes.
        return;
    for (StringMap::const_iterator it = params.begin(); it != params.end(); ++it)
    {
        if (it->first == "Size" || it->first ==  "Speed" || it->first == "Download Position" || it->first ==  "Time Left")
            gtk_tree_store_set(transferStore, &iter, transferView.col(_(it->first.c_str())), Util::toInt64(it->second), -1);
        else if (it->first == "Progress" || it->first == "Failed")
            gtk_tree_store_set(transferStore, &iter, transferView.col(it->first), Util::toInt(it->second), -1);
        else if (!it->second.empty())
            gtk_tree_store_set(transferStore, &iter, transferView.col(_(it->first.c_str())), it->second.c_str(), -1);
    }
    if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(transferStore), &parent, &iter))
        updateParent_gui(&parent);
    playSound_gui(sound);
}

void Transfers::updateFilePosition_gui(const string cid, int64_t filePosition)
{
    GtkTreeIter iter;
    GtkTreeIter parent;

    if (!findTransfer_gui(cid, TRUE, &iter))
        return;

    if (gtk_tree_model_iter_parent(GTK_TREE_MODEL(transferStore), &parent, &iter))
    {
        gtk_tree_store_set(transferStore, &parent,
            transferView.col("Download Position"), filePosition,
            -1);
        updateParent_gui(&parent);
    }
}

void Transfers::initTransfer_gui(StringMap params)
{
    dcassert(!params["CID"].empty() && !params["Target"].empty());

    bool oldParentValid = FALSE;
    bool newParentValid = FALSE;
    bool needParent;
    GtkTreeIter iter;
    GtkTreeIter oldParent;
    GtkTreeIter newParent;
    GtkTreeIter newIter;

    // We could use && BOOLSETTING(SEGMENTED_DL) here to group only when segmented is enabled,
    // but then the transfer should be worked out to display the whole size of the file. As
    // it currently only shows the size of a transfer (and always starts from 0)
    needParent = params["Filename"] != string(_("File list"));

    if (!findTransfer_gui(params["CID"], TRUE, &iter))
    {
        int connection_not_found = 0;
        dcassert(connection_not_found); // not really fatal only annoying as the dl can't be seen, can be ignored in release build
        return;
    }

    oldParentValid = gtk_tree_model_iter_parent(GTK_TREE_MODEL(transferStore), &oldParent, &iter);
    if (needParent)
    {
        newParentValid = findParent_gui(params["Target"], &newParent);

        if (newParentValid)
        {
            string target = oldParentValid ? transferView.getString(&oldParent, "Target") : transferView.getString(&iter, "Target");
            if (target != transferView.getString(&newParent, "Target")) // is the file changing
            {
                newIter = WulforUtil::copyRow_gui(transferStore, &iter, &newParent);
                gtk_tree_store_remove(transferStore, &iter);
                iter = newIter;
            }
            else
            {
                //NOTE: set update parent status if removed download
                if (transferView.getValue<int>(&newParent, "Failed"))
                {
                    gtk_tree_store_set(transferStore, &newParent,
                        transferView.col("Failed"), FALSE,
                        -1);
                }

                oldParentValid = FALSE; // Don't update the parentRow twice, since old and new are the same (and definately don't remove twice)
            }
        }
        else
        {
            string filename = params["Filename"];
            if (filename.find(_("TTH: ")) != string::npos)
                filename = filename.substr((string(_("TTH: "))).length());
            gtk_tree_store_append(transferStore, &newParent, NULL);
            newParentValid = TRUE;
            gtk_tree_store_set(transferStore, &newParent,
                transferView.col(_("Filename")), filename.c_str(),
                transferView.col(_("Path")), params["Path"].c_str(),
                transferView.col(_("Size")), Util::toInt64(params["File Size"]),
                transferView.col("Icon"), "icon-download",
                transferView.col("Download"), TRUE,
                transferView.col("Target"), params["Target"].c_str(),
                -1);

            newIter = WulforUtil::copyRow_gui(transferStore, &iter, &newParent);
            gtk_tree_store_remove(transferStore, &iter);
            iter = newIter;
        }

        gtk_tree_store_set(transferStore, &newParent,
                transferView.col(_("Size")), Util::toInt64(params["File Size"]),
                transferView.col("Download Position"), Util::toInt64(params["File Position"]),
                -1);
    }
    else if (oldParentValid) // No need for parent, but we have one anyway => move row to top-level
    {
        newIter = WulforUtil::copyRow_gui(transferStore, &iter, NULL);
        gtk_tree_store_remove(transferStore, &iter);
        iter = newIter;
    }

    if (oldParentValid)
    {
        if (gtk_tree_model_iter_n_children(GTK_TREE_MODEL(transferStore), &oldParent) == 0)
            gtk_tree_store_remove(transferStore, &oldParent);
        else
            updateParent_gui(&oldParent);
    }
    if (newParentValid)
    {
        updateParent_gui(&newParent);
    }
}

void Transfers::finishParent_gui(const string target, const string status, Sound::TypeSound sound)
{
    GtkTreeIter iter;
    if (findParent_gui(target, &iter))
    {
        if (!gtk_tree_model_iter_has_child(GTK_TREE_MODEL(transferStore), &iter))
            return;

        if (transferView.getValue<gboolean>(&iter, "Failed") == 0)
        {
            gtk_tree_store_set(transferStore, &iter,
                transferView.col(_("Status")), status.c_str(),
                transferView.col("Failed"), (gboolean)1,
                transferView.col(_("Speed")), (uint64_t)-1,
                -1);
        }
    }

    playSound_gui(sound);
}

void Transfers::playSound_gui(Sound::TypeSound sound)
{
    if (sound != Sound::NONE)
        Sound::get()->playSound(sound);
}

void Transfers::getFileList_client(string cid, string hubUrl)
{
    try
    {
        if (!cid.empty() && !hubUrl.empty())
        {
            UserPtr user = ClientManager::getInstance()->getUser(CID(cid));
            QueueManager::getInstance()->addList(HintedUser(user, hubUrl), QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
        }
    }
    catch (const Exception&)
    {
    }
}

void Transfers::matchQueue_client(string cid, string hubUrl)
{
    try
    {
        if (!cid.empty() && !hubUrl.empty())
        {
            UserPtr user = ClientManager::getInstance()->getUser(CID(cid));
            QueueManager::getInstance()->addList(HintedUser(user, hubUrl), QueueItem::FLAG_MATCH_QUEUE);//NOTE: core 0.762
        }
    }
    catch (const Exception&)
    {
    }
}

void Transfers::addFavoriteUser_client(string cid)
{
    if (!cid.empty())
    {
        UserPtr user = ClientManager::getInstance()->getUser(CID(cid));
        FavoriteManager::getInstance()->addFavoriteUser(user);
    }
}

void Transfers::grantExtraSlot_client(string cid, string hubUrl)
{
    if (!cid.empty() && !hubUrl.empty())
    {
        UserPtr user = ClientManager::getInstance()->getUser(CID(cid));
        UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));//NOTE: core 0.762
    }
}

void Transfers::removeUserFromQueue_client(string cid)
{
    if (!cid.empty())
    {
        UserPtr user = ClientManager::getInstance()->getUser(CID(cid));
        QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
    }
}

void Transfers::forceAttempt_client(string cid)
{
    if (!cid.empty())
    {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
        ConnectionManager::getInstance()->force(user);
    }
}

void Transfers::closeConnection_client(string cid, bool download)
{
    if (!cid.empty())
    {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
        ConnectionManager::getInstance()->disconnect(user, download);
    }
}

void Transfers::getParams_client(StringMap& params, ConnectionQueueItem* cqi)
{
    // NOTE: const HintedUser& getUser() const { return user; }
    const HintedUser &user = cqi->getUser();

    params["CID"] = user.user->getCID().toBase32();//NOTE: core 0.762
    params["User"] = WulforUtil::getNicks(user);//NOTE: core 0.762
    params["Hub Name"] = WulforUtil::getHubNames(user);//NOTE: core 0.762
    params["Failed"] = "0";
    params["Hub URL"] = user.hint;//NOTE: core 0.762
}

void Transfers::getParams_client(StringMap& params, Transfer* tr)
{
    // NOTE: const HintedUser getHintedUser() const;
    const HintedUser user = tr->getHintedUser();//NOTE: core 0.762
    double percent = 0.0;

    params["CID"] = user.user->getCID().toBase32();
    if (tr->getType() == Transfer::TYPE_FULL_LIST || tr->getType() == Transfer::TYPE_PARTIAL_LIST)
        params["Filename"] = _("File list");
    else if (tr->getType() == Transfer::TYPE_TREE)
        params["Filename"] = _("TTH: ") + Util::getFileName(tr->getPath());
    else
        params["Filename"] = Util::getFileName(tr->getPath());
    params["User"] = WulforUtil::getNicks(user);//NOTE: core 0.762
    params["Hub Name"] = WulforUtil::getHubNames(user);//NOTE: core 0.762
    params["Path"] = Util::getFilePath(tr->getPath());
    params["Size"] = Util::toString(tr->getSize());
    params["Download Position"] = Util::toString(tr->getPos());
    params["Speed"] = Util::toString(tr->getAverageSpeed());
    if (tr->getSize() > 0)
        percent = static_cast<double>(tr->getPos() * 100.0)/ tr->getSize();
    params["Progress"] = Util::toString(static_cast<int>(percent));
    params["IP"] = tr->getUserConnection().getRemoteIp();
    params["Time Left"] = tr->getSecondsLeft() > 0 ? Util::toString(tr->getSecondsLeft()) : "-1";
    params["Target"] = tr->getPath();
    params["Hub URL"] = tr->getUserConnection().getHubUrl();
}

void Transfers::on(DownloadManagerListener::Requesting, Download* dl) throw()
{
    StringMap params;

    getParams_client(params, dl);

    params["File Size"] = Util::toString(QueueManager::getInstance()->getSize(dl->getPath()));
    params["File Position"] = Util::toString(QueueManager::getInstance()->getPos(dl->getPath()));
    params["Sort Order"] = "w" + params["User"];
    params["Status"] = _("Requesting");
    params["Failed"] = "0";

    typedef Func1<Transfers, StringMap> F1;
    F1* f1 = new F1(this, &Transfers::initTransfer_gui, params);
    WulforManager::get()->dispatchGuiFunc(f1);
}

void Transfers::on(DownloadManagerListener::Starting, Download* dl) throw()
{
    StringMap params;

    getParams_client(params, dl);
    params["Status"] = _("Download starting...");
    params["Sort Order"] = "d" + params["User"];
    params["Failed"] = "0";
    params["tmpTarget"] = dl->getTempTarget();

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, TRUE, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(DownloadManagerListener::Tick, const DownloadList& dls) throw()
{
    for (DownloadList::const_iterator it = dls.begin(); it != dls.end(); ++it)
    {
        Download* dl = *it;
        StringMap params;
        ostringstream stream;

        getParams_client(params, *it);

        if (dl->getUserConnection().isSecure())
        {
            if (dl->getUserConnection().isTrusted())
                stream << _("[S]");
            else
                stream << _("[U]");
        }
        if (dl->isSet(Download::FLAG_TTH_CHECK))
            stream << _("[T]");
        if (dl->isSet(Download::FLAG_ZDOWNLOAD))
            stream << _("[Z]");

        stream << setiosflags(ios::fixed) << setprecision(1);
        stream << " " << _("Downloaded ") << Util::formatBytes(dl->getPos()) << " (" << params["Progress"]
        << "%) in " << Util::formatSeconds((GET_TICK() - dl->getStart()) / 1000);
        params["Status"] = stream.str();

        typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
        F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, TRUE, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(f3);
    }
}

void Transfers::on(DownloadManagerListener::Complete, Download* dl) throw()
{
    StringMap params;

    getParams_client(params, dl);
    params["Status"] = _("Download complete...");
    params["Sort Order"] = "w" + params["User"];
    params["Speed"] = "-1";

    int64_t pos = QueueManager::getInstance()->getPos(dl->getPath()) + dl->getPos();

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, TRUE, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);

    typedef Func2<Transfers, const string, int64_t> F2b;
    F2b* f2b = new F2b(this, &Transfers::updateFilePosition_gui, params["CID"], pos);
    WulforManager::get()->dispatchGuiFunc(f2b);
}

void Transfers::on(DownloadManagerListener::Failed, Download* dl, const string& reason) throw()
{
    onFailed(dl, reason);
}

void Transfers::on(QueueManagerListener::CRCFailed, Download* dl, const string& reason) throw()
{
    onFailed(dl, reason);
}

void Transfers::onFailed(Download* dl, const string& reason) {
    StringMap params;
    getParams_client(params, dl);
    params["Status"] = reason;
    params["Sort Order"] = "w" + params["User"];
    params["Failed"] = "1";
    params["Speed"] = "-1";
    params["Time Left"] = "-1";

    int64_t pos = QueueManager::getInstance()->getPos(dl->getPath()) + dl->getPos();

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, TRUE, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);

    typedef Func2<Transfers, const string, int64_t> F2b;
    F2b* f2b = new F2b(this, &Transfers::updateFilePosition_gui, params["CID"], pos);
    WulforManager::get()->dispatchGuiFunc(f2b);
}

void Transfers::on(ConnectionManagerListener::Added, ConnectionQueueItem* cqi) throw()
{
    StringMap params;
    getParams_client(params, cqi);
    params["Status"] = _("Connecting...");

    typedef Func2<Transfers, StringMap, bool> F2;
    F2* f2 = new F2(this, &Transfers::addConnection_gui, params, cqi->getDownload());
    WulforManager::get()->dispatchGuiFunc(f2);
}

void Transfers::on(ConnectionManagerListener::Connected, ConnectionQueueItem* cqi) throw()
{
    StringMap params;
    getParams_client(params, cqi);
    params["Status"] = _("Connected");

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, cqi->getDownload(), Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(ConnectionManagerListener::Removed, ConnectionQueueItem* cqi) throw()
{
    string cid = cqi->getUser().user->getCID().toBase32();//NOTE: core 0.762
    typedef Func2<Transfers, const string, bool> F2;
    F2* f2 = new F2(this, &Transfers::removeConnection_gui, cid, cqi->getDownload());
    WulforManager::get()->dispatchGuiFunc(f2);
}

void Transfers::on(ConnectionManagerListener::Failed, ConnectionQueueItem* cqi, const string& reason) throw()
{
    StringMap params;
    getParams_client(params, cqi);
    params["Status"] = reason;
    params["Failed"] = "1";
    params["Sort Order"] = "w" + params["User"];
    params["Speed"] = "-1";
    params["Time Left"] = "-1";

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, cqi->getDownload(), Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(ConnectionManagerListener::StatusChanged, ConnectionQueueItem* cqi) throw()
{
    StringMap params;
    getParams_client(params, cqi);

    if (cqi->getState() == ConnectionQueueItem::CONNECTING)
        params["Status"] = _("Connecting");
    else
        params["Status"] = _("Waiting to retry");
    params["Sort Order"] = "w" + params["User"];

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, cqi->getDownload(), Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(QueueManagerListener::Finished, QueueItem* qi, const string& dir, int64_t size) throw()
{
    string target = qi->getTarget();
    Sound::TypeSound sound = Sound::DOWNLOAD_FINISHED;

    if (qi->isSet(QueueItem::FLAG_CLIENT_VIEW | QueueItem::FLAG_USER_LIST))
    {
        sound = Sound::DOWNLOAD_FINISHED_USER_LIST;
    }
    else if (qi->isSet(QueueItem::FLAG_XML_BZLIST))
    {
        sound = Sound::NONE;
    }

    typedef Func3<Transfers, const string, const string, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::finishParent_gui, target, _("Download finished"), sound);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(QueueManagerListener::Removed, QueueItem* qi) throw()
{
    string target = qi->getTarget();

    typedef Func3<Transfers, const string, const string, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::finishParent_gui, target, _("Download removed"), Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(UploadManagerListener::Starting, Upload* ul) throw()
{
    StringMap params;

    getParams_client(params, ul);
    params["Status"] = _("Upload starting...");
    params["Sort Order"] = "u" + params["User"];
    params["Failed"] = "0";
    params["tmpTarget"] = _("none"); //fix open 'tmp' file

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, FALSE, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(UploadManagerListener::Tick, const UploadList& uls) throw()
{
    for (UploadList::const_iterator it = uls.begin(); it != uls.end(); ++it)
    {
        Upload* ul = *it;
        StringMap params;
        ostringstream stream;

        getParams_client(params, ul);

        if (ul->getUserConnection().isSecure())
        {
            if (ul->getUserConnection().isTrusted())
                stream << _("[S]");
            else
                stream << _("[U]");
        }
        if (ul->isSet(Upload::FLAG_ZUPLOAD))
            stream << _("[Z]");

        stream << setiosflags(ios::fixed) << setprecision(1);
        stream << " " << _("Uploaded ") << Util::formatBytes(ul->getPos()) << " (" << params["Progress"]
        << "%) in " << Util::formatSeconds((GET_TICK() - ul->getStart()) / 1000);
        params["Status"] = stream.str();

        typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
        F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, FALSE, Sound::NONE);
        WulforManager::get()->dispatchGuiFunc(f3);
    }
}

void Transfers::on(UploadManagerListener::Complete, Upload* ul) throw()
{
    StringMap params;

    getParams_client(params, ul);
    params["Status"] = _("Upload complete...");
    params["Sort Order"] = "w" + params["User"];
    params["Speed"] = "-1";

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, FALSE, Sound::UPLOAD_FINISHED);
    WulforManager::get()->dispatchGuiFunc(f3);
}

void Transfers::on(UploadManagerListener::Failed, Upload* ul, const string& reason) throw()
{
    StringMap params;
    getParams_client(params, ul);
    params["Status"] = reason;
    params["Sort Order"] = "w" + params["User"];
    params["Failed"] = "1";
    params["Speed"] = "-1";
    params["Time Left"] = "-1";

    typedef Func3<Transfers, StringMap, bool, Sound::TypeSound> F3;
    F3* f3 = new F3(this, &Transfers::updateTransfer_gui, params, FALSE, Sound::NONE);
    WulforManager::get()->dispatchGuiFunc(f3);
}
