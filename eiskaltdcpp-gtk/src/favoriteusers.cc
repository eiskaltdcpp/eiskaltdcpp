/*
 * Copyright © 2009-2010 freedcpp, http://code.google.com/p/freedcpp
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

#include "favoriteusers.hh"

#include <dcpp/ClientManager.h>
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"
#include "message.hh"

using namespace std;
using namespace dcpp;

FavoriteUsers::FavoriteUsers():
	BookEntry(Entry::FAVORITE_USERS, _("Favorite Users"), "favoriteusers.ui")
{
#if !GTK_CHECK_VERSION(3,0,0)
	gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("status")),FALSE);
#endif

	// Configure the dialog
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("DescriptionDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// menu
	g_object_ref_sink(getWidget("menu"));

	// Initialize favorite users list treeview
	favoriteUserView.setView(GTK_TREE_VIEW(getWidget("favoriteUserView")), TRUE, "favoriteusers");
	favoriteUserView.insertColumn(_("Auto grant slot"), G_TYPE_BOOLEAN, TreeView::BOOL, 100);
	favoriteUserView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::ICON_STRING, 100, "Icon");
	favoriteUserView.insertColumn(_("Hub (last seen in, if offline)"), G_TYPE_STRING, TreeView::STRING, 200);
	favoriteUserView.insertColumn(_("Time last seen"), G_TYPE_STRING, TreeView::STRING, 120);
	favoriteUserView.insertColumn(_("Description"), G_TYPE_STRING, TreeView::STRING, 100);
	favoriteUserView.insertColumn(_("CID"), G_TYPE_STRING, TreeView::STRING, 350);
	favoriteUserView.insertHiddenColumn("URL", G_TYPE_STRING);
	favoriteUserView.insertHiddenColumn("Icon", G_TYPE_STRING);
	favoriteUserView.finalize();

	favoriteUserStore = gtk_list_store_newv(favoriteUserView.getColCount(), favoriteUserView.getGTypes());
	gtk_tree_view_set_model(favoriteUserView.get(), GTK_TREE_MODEL(favoriteUserStore));
	g_object_unref(favoriteUserStore);

	favoriteUserSelection = gtk_tree_view_get_selection(favoriteUserView.get());
	gtk_tree_selection_set_mode(favoriteUserSelection, GTK_SELECTION_MULTIPLE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(favoriteUserStore), favoriteUserView.col(_("Nick")), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(favoriteUserView.get(), favoriteUserView.col(_("Nick"))), TRUE);
	gtk_tree_view_set_fixed_height_mode(favoriteUserView.get(), TRUE);

	GList *list = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(gtk_tree_view_get_column(favoriteUserView.get(),
		favoriteUserView.col(_("Auto grant slot")))));
	GObject *renderer = (GObject *)g_list_nth_data(list, 0);
	g_signal_connect(renderer, "toggled", G_CALLBACK(onAutoGrantSlotToggled_gui), (gpointer)this);
	g_list_free(list);

	g_signal_connect(getWidget("browseItem"), "activate", G_CALLBACK(onBrowseItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("matchQueueItem"), "activate", G_CALLBACK(onMatchQueueItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("sendPMItem"), "activate", G_CALLBACK(onSendPMItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("grantSlotItem"), "activate", G_CALLBACK(onGrantSlotItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("connectItem"), "activate", G_CALLBACK(onConnectItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeFromQueueItem"), "activate", G_CALLBACK(onRemoveFromQueueItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("descriptionItem"), "activate", G_CALLBACK(onDescriptionItemClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItemClicked_gui), (gpointer)this);
	g_signal_connect(favoriteUserView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(favoriteUserView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(favoriteUserView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
}

FavoriteUsers::~FavoriteUsers()
{
	FavoriteManager::getInstance()->removeListener(this);

	gtk_widget_destroy(getWidget("DescriptionDialog"));
	g_object_unref(getWidget("menu"));
}

void FavoriteUsers::show()
{
	// Initialize favorite users list
	FavoriteManager::FavoriteMap map = FavoriteManager::getInstance()->getFavoriteUsers();
	FavoriteManager::FavoriteMap::const_iterator it;

	for (it = map.begin(); it != map.end(); ++it)
	{
		GtkTreeIter iter;
		const FavoriteUser &user = it->second;
		bool online = user.getUser()->isOnline();
		string hub = online ? WulforUtil::getHubNames(user.getUser(), user.getUrl()) : user.getUrl();//NOTE: core 0.762
		string seen = online ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen());
		string cid = user.getUser()->getCID().toBase32();

		gtk_list_store_append(favoriteUserStore, &iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(_("Auto grant slot")), user.isSet(FavoriteUser::FLAG_GRANTSLOT) ? TRUE : FALSE,
			favoriteUserView.col(_("Nick")), user.getNick().c_str(),
			favoriteUserView.col(_("Hub (last seen in, if offline)")), hub.c_str(),
			favoriteUserView.col(_("Time last seen")), seen.c_str(),
			favoriteUserView.col(_("Description")), user.getDescription().c_str(),
			favoriteUserView.col(_("CID")), cid.c_str(),
			favoriteUserView.col("URL"), user.getUrl().c_str(),
			favoriteUserView.col("Icon"), "eiskaltdcpp-normal",
			-1);

		userIters.insert(UserIters::value_type(cid, iter));
	}
	FavoriteManager::getInstance()->addListener(this);
}

gboolean FavoriteUsers::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			fu->onRemoveItemClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(fu->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean FavoriteUsers::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;
	fu->previous = event->type;

	if (event->button == 3)
	{
		GtkTreePath *path;

		if (gtk_tree_view_get_path_at_pos(fu->favoriteUserView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(fu->favoriteUserSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean FavoriteUsers::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		if (event->button == 1 && fu->previous == GDK_2BUTTON_PRESS)
		{
			if (WGETB("pm"))

				fu->onSendPMItemClicked_gui(NULL, data);
			else
				fu->onBrowseItemClicked_gui(NULL, data);
		}
		else if (event->button == 2 && event->type == GDK_BUTTON_RELEASE)
		{
			if (WGETB("pm"))

				fu->onBrowseItemClicked_gui(NULL, data);
			else
				fu->onSendPMItemClicked_gui(NULL, data);
		}
		else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
		{
			gtk_menu_popup(GTK_MENU(fu->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void FavoriteUsers::onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func3<FavoriteUsers, string, string, bool> F3;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F3 *func = new F3(fu, &FavoriteUsers::getFileList_client,
					fu->favoriteUserView.getString(&iter, _("CID")),
					fu->favoriteUserView.getString(&iter, "URL"),
					FALSE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void FavoriteUsers::onMatchQueueItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func3<FavoriteUsers, string, string, bool> F3;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F3 *func = new F3(fu, &FavoriteUsers::getFileList_client,
					fu->favoriteUserView.getString(&iter, _("CID")),
					fu->favoriteUserView.getString(&iter, "URL"),
					TRUE);
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void FavoriteUsers::onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN,
					fu->favoriteUserView.getString(&iter, _("CID")),
					fu->favoriteUserView.getString(&iter, "URL"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void FavoriteUsers::onGrantSlotItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func2<FavoriteUsers, string, string> F2;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F2 *func = new F2(fu, &FavoriteUsers::grantSlot_client,
					fu->favoriteUserView.getString(&iter, _("CID")),
					fu->favoriteUserView.getString(&iter, "URL"));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void FavoriteUsers::onConnectItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				WulforManager::get()->getMainWindow()->showHub_gui(fu->favoriteUserView.getString(&iter, "URL"));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void FavoriteUsers::onRemoveFromQueueItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		GtkTreeIter iter;
		GtkTreePath *path;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);
		typedef Func1<FavoriteUsers, string> F1;

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				F1 *func = new F1(fu, &FavoriteUsers::removeUserFromQueue_client, fu->favoriteUserView.getString(&iter, _("CID")));
				WulforManager::get()->dispatchClientFunc(func);
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);
	}
}

void FavoriteUsers::onDescriptionItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) == 1)
	{
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		if (list != NULL)
		{
			GtkTreeIter iter;
			GtkTreePath *path = (GtkTreePath *) list->data;
			string description, nick, cid;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				description = fu->favoriteUserView.getString(&iter, _("Description"));
				nick = fu->favoriteUserView.getString(&iter, _("Nick"));
				cid = fu->favoriteUserView.getString(&iter, _("CID"));
			}
			gtk_tree_path_free(path);
			g_list_free(list);

			gtk_window_set_title(GTK_WINDOW(fu->getWidget("DescriptionDialog")), nick.c_str());
			gtk_entry_set_text(GTK_ENTRY(fu->getWidget("descriptionEntry")), description.c_str());
			gint response = gtk_dialog_run(GTK_DIALOG(fu->getWidget("DescriptionDialog")));

			// Fix crash, if the dialog gets programmatically destroyed.
			if (response == GTK_RESPONSE_NONE)
			{
				return;
			}

			gtk_widget_hide(fu->getWidget("DescriptionDialog"));

			if (response != GTK_RESPONSE_OK)
				return;

			if (fu->findUser_gui(cid, &iter))
			{
				description = gtk_entry_get_text(GTK_ENTRY(fu->getWidget("descriptionEntry")));
				gtk_list_store_set(fu->favoriteUserStore, &iter, fu->favoriteUserView.col(_("Description")), description.c_str(), -1);

				typedef Func2<FavoriteUsers, string, string> F2;
				F2 *func = new F2(fu, &FavoriteUsers::setUserDescription_client, cid, description);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
	}
}

void FavoriteUsers::onRemoveItemClicked_gui(GtkMenuItem *item, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;

	if (gtk_tree_selection_count_selected_rows(fu->favoriteUserSelection) > 0)
	{
		ParamMap params;
		GtkTreeIter iter;
		GtkTreePath *path;
		typedef Func1<FavoriteUsers, string> F1;
		GList *list = gtk_tree_selection_get_selected_rows(fu->favoriteUserSelection, NULL);

		for (GList *i = list; i; i = i->next)
		{
			path = (GtkTreePath *)i->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
			{
				params.insert(ParamMap::value_type(fu->favoriteUserView.getString(&iter, _("CID")),
											fu->favoriteUserView.getString(&iter, _("Nick"))));
			}
			gtk_tree_path_free(path);
		}
		g_list_free(list);

		if (WGETB("confirm-user-removal"))
		{
			GtkWidget* dialog = gtk_message_dialog_new(GTK_WINDOW(WulforManager::get()->getMainWindow()->getContainer()),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_NONE,
				_("Are you sure you want to delete favorite user(s)?"));
			gtk_dialog_add_buttons(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_REMOVE,
				GTK_RESPONSE_YES, NULL);
			gtk_dialog_set_alternative_button_order(GTK_DIALOG(dialog), GTK_RESPONSE_YES, GTK_RESPONSE_CANCEL, -1);
			gint response = gtk_dialog_run(GTK_DIALOG(dialog));

			// Widget failed if the dialog gets programmatically destroyed.
			if (response == GTK_RESPONSE_NONE)
				return;

			gtk_widget_hide(dialog);

			if (response != GTK_RESPONSE_YES)
				return;
		}

		for (ParamMap::const_iterator it = params.begin(); it != params.end(); ++it)
		{
			F1 *func = new F1(fu, &FavoriteUsers::removeFavoriteUser_client, it->first);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void FavoriteUsers::onAutoGrantSlotToggled_gui(GtkCellRendererToggle *cell, gchar *path, gpointer data)
{
	FavoriteUsers *fu = (FavoriteUsers *)data;
	GtkTreeIter iter;

	if (gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(fu->favoriteUserStore), &iter, path))
	{
		string cid = fu->favoriteUserView.getString(&iter, _("CID"));
		gboolean grant = fu->favoriteUserView.getValue<gboolean>(&iter, _("Auto grant slot"));
		grant = !grant;
		gtk_list_store_set(fu->favoriteUserStore, &iter, fu->favoriteUserView.col(_("Auto grant slot")), grant, -1);

		typedef Func2<FavoriteUsers, string, bool> F2;
		F2 *func = new F2(fu, &FavoriteUsers::setAutoGrantSlot_client, cid, grant);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void FavoriteUsers::getFileList_client(const string cid, const string hubUrl, bool match)
{
	try
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

		if (user)
		{
			const HintedUser hintedUser(user, hubUrl);//NOTE: core 0.762
			if (match)
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_MATCH_QUEUE);//NOTE: core 0.762
			else
				QueueManager::getInstance()->addList(hintedUser, QueueItem::FLAG_CLIENT_VIEW);//NOTE: core 0.762
		}
	}
	catch (const Exception& e)
	{
		typedef Func1<FavoriteUsers, string> F1;
		F1 *func = new F1(this, &FavoriteUsers::setStatus_gui, e.getError());
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FavoriteUsers::grantSlot_client(const string cid, const string hubUrl)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
	if (user)
	{
		UploadManager::getInstance()->reserveSlot(HintedUser(user, hubUrl));//NOTE: core 0.762
		typedef Func1<FavoriteUsers, string> F1;
		F1 *func = new F1(this, &FavoriteUsers::setStatus_gui, _("Slot granted to ") + WulforUtil::getNicks(user, hubUrl));//NOTE: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FavoriteUsers::removeUserFromQueue_client(const string cid)
{
	if (!cid.empty())
	{
		UserPtr user = ClientManager::getInstance()->findUser(CID(cid));
		if (user)
			QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
	}
}

void FavoriteUsers::setAutoGrantSlot_client(const string cid, bool grant)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->setAutoGrant(user, grant);
	}
}

void FavoriteUsers::removeFavoriteUser_client(const string cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->removeFavoriteUser(user);
	}
}

void FavoriteUsers::setUserDescription_client(const string cid, const string description)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
	{
		FavoriteManager::getInstance()->setUserDescription(user, description);
	}
}

bool FavoriteUsers::findUser_gui(const string &cid, GtkTreeIter *iter)
{
	UserIters::const_iterator it = userIters.find(cid);

	if (it != userIters.end())
	{
		if (iter)
			*iter = it->second;

		return TRUE;
	}

	return FALSE;
}

void FavoriteUsers::updateFavoriteUser_gui(ParamMap params)
{
	const string &cid = params["CID"];
	GtkTreeIter iter;

	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_set(favoriteUserStore, &iter, favoriteUserView.col(_("Time last seen")), params["Time"].c_str(), -1);
	}
	else
	{
		gtk_list_store_append(favoriteUserStore, &iter);
		gtk_list_store_set(favoriteUserStore, &iter,
			favoriteUserView.col(_("Nick")), params["Nick"].c_str(),
			favoriteUserView.col(_("Hub (last seen in, if offline)")), params["Hub"].c_str(),
			favoriteUserView.col(_("Time last seen")), params["Time"].c_str(),
			favoriteUserView.col(_("Description")), params["Description"].c_str(),
			favoriteUserView.col(_("CID")), cid.c_str(),
			favoriteUserView.col("URL"), params["URL"].c_str(),
			favoriteUserView.col("Icon"), "eiskaltdcpp-normal",
			-1);

		userIters.insert(UserIters::value_type(cid, iter));
	}
}

void FavoriteUsers::removeFavoriteUser_gui(const string cid)
{
	GtkTreeIter iter;

	if (findUser_gui(cid, &iter))
	{
		gtk_list_store_remove(favoriteUserStore, &iter);
		userIters.erase(cid);
	}
}

void FavoriteUsers::setStatus_gui(const string text)
{
	if (!text.empty())
	{
		gtk_statusbar_pop(GTK_STATUSBAR(getWidget("status")), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget("status")), 0, ("[" + Util::getShortTimeString() + "] " + text).c_str());
	}
}

void FavoriteUsers::on(FavoriteManagerListener::UserAdded, const FavoriteUser &user) noexcept
{
	ParamMap params;
	bool online = user.getUser()->isOnline();
	params.insert(ParamMap::value_type("Nick", user.getNick()));
	params.insert(ParamMap::value_type("Hub", online ? WulforUtil::getHubNames(user.getUser(), user.getUrl()) : user.getUrl()));//NOTE: core 0.762
	params.insert(ParamMap::value_type("Time", online ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen())));
	params.insert(ParamMap::value_type("Description", user.getDescription()));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));
	params.insert(ParamMap::value_type("URL", user.getUrl()));

	Func1<FavoriteUsers, ParamMap> *func = new Func1<FavoriteUsers, ParamMap>(this, &FavoriteUsers::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteUsers::on(FavoriteManagerListener::UserRemoved, const FavoriteUser &user) noexcept
{
	Func1<FavoriteUsers, string> *func = new Func1<FavoriteUsers, string>(this, &FavoriteUsers::removeFavoriteUser_gui,
		user.getUser()->getCID().toBase32());
	WulforManager::get()->dispatchGuiFunc(func);
}

void FavoriteUsers::on(FavoriteManagerListener::StatusChanged, const FavoriteUser &user) noexcept
{
	ParamMap params;
	string seen = user.getUser()->isOnline() ? _("Online") : Util::formatTime("%Y-%m-%d %H:%M", user.getLastSeen());
	params.insert(ParamMap::value_type("Time", seen));
	params.insert(ParamMap::value_type("CID", user.getUser()->getCID().toBase32()));

	Func1<FavoriteUsers, ParamMap> *func = new Func1<FavoriteUsers, ParamMap>(this, &FavoriteUsers::updateFavoriteUser_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
