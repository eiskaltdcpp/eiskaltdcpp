//      uploadqueue.cc
//
//      Copyright 2011 Mank <Mank1@seznam.cz>
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//
//


#include "uploadqueue.hh"
#include <dcpp/Client.h>
#include <dcpp/ClientManager.h>
#include <dcpp/FavoriteManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/UploadManager.h>
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

UploadQueue::UploadQueue():
    BookEntry(Entry::UPLOADQUEUE, _("Upload Queue"), "uploadqueue.ui")
{
#if !GTK_CHECK_VERSION(3,0,0)
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("statusbar1")),false);
#endif

    users.setView(GTK_TREE_VIEW(getWidget("viewUsers")));
    users.insertColumn("User", G_TYPE_STRING, TreeView::STRING, 80);
    users.insertColumn("File", G_TYPE_STRING, TreeView::STRING, 150);
    users.insertColumn("Hub", G_TYPE_STRING, TreeView::STRING, 80);
    users.insertColumn("CID", G_TYPE_STRING, TreeView::STRING, 80);
    users.finalize();

    store = gtk_list_store_newv(users.getColCount(),users.getGTypes());
    gtk_tree_view_set_model(users.get(), GTK_TREE_MODEL(store));
    g_object_unref(store);

    selection = gtk_tree_view_get_selection(users.get());

    g_signal_connect(getWidget("GranSlotItem"), "activate", G_CALLBACK(onGrantSlotItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItem_gui), (gpointer)this);
    g_signal_connect(getWidget("pmItem"), "activate", G_CALLBACK(onSendPMItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("BrowseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("favoriteItem"), "activate", G_CALLBACK(onFavoriteUserAddItemClicked_gui), (gpointer)this);

    g_signal_connect(users.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
    g_signal_connect(users.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
    g_signal_connect(users.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);

}
UploadQueue::~UploadQueue()
{
    UploadManager::getInstance()->removeListener(this);
    gtk_list_store_clear(store);
    g_object_unref(store);
}

void UploadQueue::show()
{
    UploadManager::getInstance()->addListener(this);
    init();
}

void UploadQueue::init()
{
    // Load queue
    const dcpp::HintedUserList _users = UploadManager::getInstance()->getWaitingUsers();
    UploadManager *up = UploadManager::getInstance();
    for (auto& uit :_users)
    {
        const UploadManager::FileSet f = up->getWaitingUserFiles((uit.user));
        StringMap params;
        for(auto& fit : f)
        {
            GtkTreeIter iter;
            getParams(fit,uit.user, params);
            addFile(params, &iter);
        }

    }
}

void UploadQueue::getParams(const string& file, UserPtr user, StringMap &params)
{
    params["file"]  = file;
    params["Nick"]  = WulforUtil::getNicks(user->getCID(), Util::emptyString);
    params["CID"]   = user->getCID().toBase32();
    params["hub"]   = WulforUtil::getHubNames(user->getCID(), Util::emptyString);
}

void UploadQueue::addFile(StringMap &params,GtkTreeIter *iter)
{

    gtk_list_store_append(store,iter);
    gtk_list_store_set(store, iter,
                       users.col("User"), params["Nick"].c_str(),
            users.col("File"),params["file"].c_str(),
            users.col("Hub") , params["hub"].c_str(),
            users.col("CID"), params["CID"].c_str(),
            -1);
    mapUsers.insert(MapUsers::value_type(params["CID"], *iter));
}

void UploadQueue::AddFile_gui(StringMap params)
{
    GtkTreeIter iter;
    g_autofree gchar *cpfile = NULL;
    auto it = mapUsers.find(params["CID"]);
    if(it != mapUsers.end())
    {
        iter = it->second;
        gtk_tree_model_get(GTK_TREE_MODEL(store),&iter,
                           1,&cpfile,-1);
        params["file"] += string(cpfile);

    }
    addFile(params,&iter);
}

void UploadQueue::removeUser(string cid)
{
    GtkTreeIter iter;
    auto it = mapUsers.find(cid);
    if(it != mapUsers.end())
    {
        iter = it->second;
        gtk_list_store_remove(store,&iter);
        mapUsers.erase(it);

    }
}

void UploadQueue::onGrantSlotItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    (void)item;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
        typedef Func1<UploadQueue, string> F2;

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
            {
                F2 *func = new F2(qp, &UploadQueue::grantSlot_client,
                                  qp->users.getString(&iter, "CID"));
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void UploadQueue::onRemoveItem_gui(GtkMenuItem *item, gpointer data)
{
    (void)item;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
        typedef Func1<UploadQueue, string> F2;

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
            {
                F2 *func = new F2(qp, &UploadQueue::removeUploadFromQueue,
                                  qp->users.getString(&iter, "CID"));
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void UploadQueue::onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    (void)item;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
            {
                WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN,
                                                                             qp->users.getString(&iter, "CID"));
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void UploadQueue::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    (void)item;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
        typedef Func1<UploadQueue, string> F1;

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
            {
                F1 *func = new F1(qp, &UploadQueue::getFileList_client,
                                  qp->users.getString(&iter, "CID"));
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

void UploadQueue::onFavoriteUserAddItemClicked_gui(GtkMenuItem *item, gpointer data)
{
    (void)item;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        GtkTreeIter iter;
        GtkTreePath *path = NULL;
        GList *list = gtk_tree_selection_get_selected_rows(qp->selection, NULL);
        typedef Func1<UploadQueue, string> F2;

        for (GList *i = list; i; i = i->next)
        {
            path = (GtkTreePath *)i->data;

            if (gtk_tree_model_get_iter(GTK_TREE_MODEL(qp->store), &iter, path))
            {
                F2 *func = new F2(qp, &UploadQueue::addFavoriteUser_client,
                                  qp->users.getString(&iter, "CID"));
                WulforManager::get()->dispatchClientFunc(func);
            }
            gtk_tree_path_free(path);
        }
        g_list_free(list);
    }
}

gboolean UploadQueue::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    (void)widget;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        if (event->keyval == GDK_KEY_Menu || (event->keyval == GDK_KEY_F10 && event->state & GDK_SHIFT_MASK))
        {
#if GTK_CHECK_VERSION(3,22,0)
            gtk_menu_popup_at_pointer(GTK_MENU(qp->getWidget("menu")),NULL);
#else
            gtk_menu_popup(GTK_MENU(qp->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
#endif
        }
    }

    return false;
}

gboolean UploadQueue::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    (void)widget;
    UploadQueue *qp = (UploadQueue *)data;
    qp->previous = event->type;

    if (event->button == 3)
    {
        GtkTreePath *path = NULL;

        if (gtk_tree_view_get_path_at_pos(qp->users.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
        {
            bool selected = gtk_tree_selection_path_is_selected(qp->selection, path);
            gtk_tree_path_free(path);

            if (selected)
                return true;
        }
    }
    return false;
}

gboolean UploadQueue::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    (void)widget;
    UploadQueue *qp = (UploadQueue *)data;

    if (gtk_tree_selection_count_selected_rows(qp->selection) > 0)
    {
        if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
        {
#if GTK_CHECK_VERSION(3,22,0)
            gtk_menu_popup_at_pointer(GTK_MENU(qp->getWidget("menu")),NULL);
#else
            gtk_menu_popup(GTK_MENU(qp->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
#endif
        }
    }

    return false;
}

void UploadQueue::grantSlot_client(const string cid)
{
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
    if (user)
    {
        UploadManager::getInstance()->reserveSlot(HintedUser(user, Util::emptyString));
    }
}

void UploadQueue::removeUploadFromQueue(const string cid)
{
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
    if (user)
    {
        UploadManager::getInstance()->clearUserFiles(user);
    }
}

void UploadQueue::getFileList_client(const std::string cid)
{
    try {
        UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
        if(user)
        {
            HintedUser hintedUser(user, Util::emptyString);
            QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);
        }
    }catch(...)
    {
        //... for now ignore it
    }

}

void UploadQueue::addFavoriteUser_client(const string cid)
{
    UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

    if (user)
    {
        FavoriteManager::getInstance()->addFavoriteUser(user);
    }
}

void UploadQueue::on(dcpp::UploadManagerListener::WaitingAddFile, const HintedUser& hUser, const string &file) noexcept
{
    StringMap params;
    getParams(file,hUser.user,params);
    typedef Func1<UploadQueue, StringMap> F1;
    F1 *func = new F1(this,&UploadQueue::AddFile_gui,params);
    WulforManager::get()->dispatchGuiFunc(func);
}

void UploadQueue::on(dcpp::UploadManagerListener::WaitingRemoveUser, const HintedUser& user) noexcept
{
    typedef Func1<UploadQueue, string> F1;
    F1 *func = new F1(this, &UploadQueue::removeUser,user.user->getCID().toBase32());
    WulforManager::get()->dispatchGuiFunc(func);

}
