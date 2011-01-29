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

#include "finishedtransfers.hh"
#include "previewmenu.hh"
#include <dcpp/Text.h>
#include <dcpp/ClientManager.h>
#include "wulformanager.hh"
#include "settingsmanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

FinishedTransfers* FinishedTransfers::createFinishedDownloads()
{
	return new FinishedTransfers(Entry::FINISHED_DOWNLOADS, _("Finished Downloads"), false);
}

FinishedTransfers* FinishedTransfers::createFinishedUploads()
{
	return new FinishedTransfers(Entry::FINISHED_UPLOADS, _("Finished Uploads"), true);
}

FinishedTransfers::FinishedTransfers(const EntryType type, const string &title, bool isUpload):
	BookEntry(type, title, "finishedtransfers.glade"),
	isUpload(isUpload),
	totalFiles(0),
	totalUsers(0),
	totalBytes(0),
	totalTime(0)
{
	// menu
	g_object_ref_sink(getWidget("menu"));

	// Initialize transfer treeview
	fileView.setView(GTK_TREE_VIEW(getWidget("fileView")), true, "finished");
	fileView.insertColumn(_("Time"), G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn(_("Filename"), G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn(_("Path"), G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn(_("Nicks"), G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn(_("Transferred"), G_TYPE_INT64, TreeView::SIZE, 100);
	fileView.insertColumn(_("Speed"), G_TYPE_INT64, TreeView::SPEED, 100);
	fileView.insertColumn(_("CRC Checked"), G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertHiddenColumn("Target", G_TYPE_STRING);
	fileView.insertHiddenColumn("Elapsed Time", G_TYPE_INT64);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col(_("Time"))), TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col(_("Time")), GTK_SORT_ASCENDING);
	gtk_tree_view_set_fixed_height_mode(fileView.get(), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);

	// Initialize user treeview
	userView.setView(GTK_TREE_VIEW(getWidget("userView")), true, "finished");
	userView.insertColumn(_("Time"), G_TYPE_STRING, TreeView::STRING, 150);
	userView.insertColumn(_("Nick"), G_TYPE_STRING, TreeView::STRING, 100);
	userView.insertColumn(_("Hub"), G_TYPE_STRING, TreeView::STRING, 200);
	userView.insertColumn(_("Files"), G_TYPE_STRING, TreeView::STRING, 100);
	userView.insertColumn(_("Transferred"), G_TYPE_INT64, TreeView::SIZE, 100);
	userView.insertColumn(_("Speed"), G_TYPE_INT64, TreeView::SPEED, 100);
	userView.insertHiddenColumn("CID", G_TYPE_STRING);
	userView.insertHiddenColumn("Elapsed Time", G_TYPE_INT64);
	userView.finalize();
	userStore = gtk_list_store_newv(userView.getColCount(), userView.getGTypes());
	gtk_tree_view_set_model(userView.get(), GTK_TREE_MODEL(userStore));
	g_object_unref(userStore);
	userSelection = gtk_tree_view_get_selection(userView.get());
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(userView.get(), userView.col(_("Time"))), TRUE);
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(userStore), userView.col(_("Time")), GTK_SORT_ASCENDING);
	gtk_tree_view_set_fixed_height_mode(userView.get(), TRUE);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(userView.get()), GTK_SELECTION_MULTIPLE);

	// Initialize the preview menu
	appsPreviewMenu = new PreviewMenu(getWidget("appsPreviewMenu"));

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("openFolderItem"), "activate", G_CALLBACK(onOpenFolder_gui), (gpointer)this);
	g_signal_connect(getWidget("removeItem"), "activate", G_CALLBACK(onRemoveItems_gui), (gpointer)this);
	g_signal_connect(getWidget("removeAllItem"), "activate", G_CALLBACK(onRemoveAll_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
	g_signal_connect(userView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
	g_signal_connect(userView.get(), "button-release-event", G_CALLBACK(onButtonReleased_gui), (gpointer)this);
	g_signal_connect(userView.get(), "key-release-event", G_CALLBACK(onKeyReleased_gui), (gpointer)this);
    g_signal_connect_after(getWidget("finishedbook"), "switch-page", G_CALLBACK(onPageSwitched_gui), (gpointer)this);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(getWidget("showOnlyFullFilesCheckButton")), BOOLSETTING(FINISHED_DL_ONLY_FULL));

    if (type == Entry::FINISHED_DOWNLOADS)
       g_signal_connect(getWidget("showOnlyFullFilesCheckButton"), "toggled", G_CALLBACK(onShowOnlyFullFilesToggled_gui), (gpointer)this);
    else
       gtk_widget_hide(getWidget("showOnlyFullFilesCheckButton"));
}

FinishedTransfers::~FinishedTransfers()
{
	FinishedManager::getInstance()->removeListener(this);
	g_object_unref(getWidget("menu"));
	delete appsPreviewMenu;
    int active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("showOnlyFullFilesCheckButton")));
    SettingsManager::getInstance()->set(SettingsManager::FINISHED_DL_ONLY_FULL, active);
}

void FinishedTransfers::show()
{
	initializeList_client();
	//Func0<FinishedTransfers> *func = new Func0<FinishedTransfers>(this, &FinishedTransfers::initializeList_client);
	//WulforManager::get()->dispatchClientFunc(func);
	FinishedManager::getInstance()->addListener(this);
}

bool FinishedTransfers::findUser_gui(GtkTreeIter* iter, const string& cid)
{
	bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(userStore), iter);

	while (valid)
	{
		if (userView.getString(iter, "CID") == cid)
			return TRUE;

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(userStore), iter);
	}

	return FALSE;
}

bool FinishedTransfers::findFile_gui(GtkTreeIter* iter, const string& item)
{
	bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fileStore), iter);

	while (valid)
	{
		if (fileView.getString(iter, "Target") == item)
			return TRUE;

		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fileStore), iter);
	}

	return FALSE;
}

void FinishedTransfers::addUser_gui(StringMap params, bool update)
{
	GtkTreeIter iter;
	int64_t transferred = Util::toInt64(params["Transferred"]);
	int64_t speed = Util::toInt64(params["Speed"]);
	int64_t time = Util::toInt64(params["Elapsed Time"]);
	int64_t addTime = time;
	int64_t addSize = transferred;

	if (!findUser_gui(&iter, params["CID"]))
	{
		gtk_list_store_append(userStore, &iter);
		totalUsers++;
	}
	else
	{
		addSize = transferred - userView.getValue<int64_t>(&iter, _("Transferred"));
		addTime = time - userView.getValue<int64_t>(&iter, "Elapsed Time");
	}

	gtk_list_store_set(userStore, &iter,
		userView.col(_("Time")), params["Time"].c_str(),
		userView.col(_("Nick")), params["Nick"].c_str(),
		userView.col(_("Hub")), params["Hub"].c_str(),
		userView.col(_("Files")), params["Files"].c_str(),
		userView.col(_("Transferred")), transferred,
		userView.col(_("Speed")), speed,
		userView.col("CID"), params["CID"].c_str(),
		userView.col("Elapsed Time"), time,
		-1);

	totalBytes += addSize;
	totalTime += addTime;

	if (update)
	{
		updateStatus_gui();

		if ((!isUpload && WGETB("bold-finished-downloads")) ||
		     (isUpload && WGETB("bold-finished-uploads")))
		{
			setBold_gui();
		}
	}
}

void FinishedTransfers::addFile_gui(StringMap params, bool update)
{
   if (!isUpload && params["full"] == "0" &&
       gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(getWidget("showOnlyFullFilesCheckButton"))))
   {
       return;
   }

	GtkTreeIter iter;
	int64_t transferred = Util::toInt64(params["Transferred"]);
	int64_t speed = Util::toInt64(params["Speed"]);
	int64_t time = Util::toInt64(params["Elapsed Time"]);

	if (!findFile_gui(&iter, params["Target"]))
	{
		gtk_list_store_append(fileStore, &iter);
		totalFiles++;
	}
	gtk_list_store_set(fileStore, &iter,
		fileView.col(_("Time")), params["Time"].c_str(),
		fileView.col(_("Filename")), params["Filename"].c_str(),
		fileView.col(_("Path")), params["Path"].c_str(),
		fileView.col(_("Nicks")), params["Nicks"].c_str(),
		fileView.col(_("Transferred")), transferred,
		fileView.col(_("Speed")), speed,
		fileView.col(_("CRC Checked")), params["CRC Checked"].c_str(),
		fileView.col("Target"), params["Target"].c_str(),
		fileView.col("Elapsed Time"), time,
		-1);

	if (update)
	{
		updateStatus_gui();

		if ((!isUpload && WGETB("bold-finished-downloads")) ||
		     (isUpload && WGETB("bold-finished-uploads")))
		{
			setBold_gui();
		}
	}
}

void FinishedTransfers::updateStatus_gui()
{
	string status;
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(getWidget("finishedbook"))) ==
			gtk_notebook_page_num(GTK_NOTEBOOK(getWidget("finishedbook")), getWidget("viewWindowFile")))
    {
        status = Util::toString(totalFiles) + _(" files");

        if (getType() == Entry::FINISHED_DOWNLOADS)
            gtk_widget_show(getWidget("showOnlyFullFilesCheckButton"));
        else
            gtk_widget_hide(getWidget("showOnlyFullFilesCheckButton"));
    }
	else
    {
        status = Util::toString(totalUsers) + _(" users");
        gtk_widget_hide(getWidget("showOnlyFullFilesCheckButton"));
    }

	string size = Util::formatBytes(totalBytes);
	string speed = Util::formatBytes((totalTime > 0) ? totalBytes * ((int64_t)1000) / totalTime : 0) + "/s";

	gtk_statusbar_push(GTK_STATUSBAR(getWidget("totalItems")), 0, status.c_str());
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("totalSize")), 0, size.c_str());
	gtk_statusbar_push(GTK_STATUSBAR(getWidget("averageSpeed")), 0, speed.c_str());
}

gboolean FinishedTransfers::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	GtkTreeSelection *selection;
	TreeView *view;
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(ft->getWidget("finishedbook"))) ==
			gtk_notebook_page_num(GTK_NOTEBOOK(ft->getWidget("finishedbook")), ft->getWidget("viewWindowFile")))
	{
		selection = ft->fileSelection;
		view = &ft->fileView;
	}
	else
	{
		selection = ft->userSelection;
		view = &ft->userView;
	}


	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(view->get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(selection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}


	return FALSE;
}

gboolean FinishedTransfers::onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	GtkTreeSelection *selection;
	TreeView *view;
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(ft->getWidget("finishedbook"))) ==
			gtk_notebook_page_num(GTK_NOTEBOOK(ft->getWidget("finishedbook")), ft->getWidget("viewWindowFile")))
	{
		selection = ft->fileSelection;
		view = &ft->fileView;
	}
	else
	{
		selection = ft->userSelection;
		view = &ft->userView;
	}
	int count = gtk_tree_selection_count_selected_rows(selection);

	if (event->button == 3 && count > 0)
	{
		ft->appsPreviewMenu->cleanMenu_gui();

		if (view == &ft->fileView)
		{
			GtkTreeIter iter;
			GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);
			GtkTreePath *path = (GtkTreePath *)list->data;

			if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
			{
				string target = ft->fileView.getString(&iter, "Target");

				if (ft->appsPreviewMenu->buildMenu_gui(target))
					gtk_widget_set_sensitive(ft->getWidget("appsPreviewItem"), TRUE);
				else gtk_widget_set_sensitive(ft->getWidget("appsPreviewItem"), FALSE);
			}

			gtk_tree_path_free(path);
			g_list_free(list);

			gtk_widget_set_sensitive(ft->getWidget("openFolderItem"), TRUE);
		}
		else
		{
			gtk_widget_set_sensitive(ft->getWidget("appsPreviewItem"), FALSE);
			gtk_widget_set_sensitive(ft->getWidget("openFolderItem"), FALSE);
		}

		gtk_widget_show_all(ft->getWidget("menu"));
		gtk_menu_popup(GTK_MENU(ft->getWidget("menu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
	}

	return FALSE;
}

gboolean FinishedTransfers::onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	GtkTreeSelection *selection;
	TreeView *view;
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(ft->getWidget("finishedbook"))) ==
			gtk_notebook_page_num(GTK_NOTEBOOK(ft->getWidget("finishedbook")), ft->getWidget("viewWindowFile")))
	{
		selection = ft->fileSelection;
		view = &ft->fileView;
	}
	else
	{
		selection = ft->userSelection;
		view = &ft->userView;
	}
	int count = gtk_tree_selection_count_selected_rows(selection);

	if (count > 0)
	{
		if (view == &ft->fileView && (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter))
		{
			onOpen_gui(NULL, data);
		}
		else if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			onRemoveItems_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			ft->appsPreviewMenu->cleanMenu_gui();

			if (view == &ft->fileView)
			{
				GtkTreeIter iter;
				GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);
				GtkTreePath *path = (GtkTreePath *)list->data;

				if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
				{
					string target = ft->fileView.getString(&iter, "Target");

					if (ft->appsPreviewMenu->buildMenu_gui(target))
						gtk_widget_set_sensitive(ft->getWidget("appsPreviewItem"), TRUE);
					else gtk_widget_set_sensitive(ft->getWidget("appsPreviewItem"), FALSE);
				}

				gtk_tree_path_free(path);
				g_list_free(list);

				gtk_widget_set_sensitive(ft->getWidget("openFolderItem"), TRUE);
			}
			else
			{
				gtk_widget_set_sensitive(ft->getWidget("appsPreviewItem"), FALSE);
				gtk_widget_set_sensitive(ft->getWidget("openFolderItem"), FALSE);
			}

			gtk_widget_show_all(ft->getWidget("menu"));
			gtk_menu_popup(GTK_MENU(ft->getWidget("menu")), NULL, NULL, NULL, NULL, 1, event->time);
		}
	}

	return FALSE;
}

void FinishedTransfers::onShowOnlyFullFilesToggled_gui(GtkWidget *widget, gpointer data)
{
    FinishedTransfers *ft = (FinishedTransfers *)data;
    StringMap params;
    gtk_list_store_clear(ft->fileStore);

    FinishedManager::getInstance()->lockLists();

    const FinishedManager::MapByFile &list = FinishedManager::getInstance()->getMapByFile(ft->isUpload);

    for (FinishedManager::MapByFile::const_iterator it = list.begin(); it != list.end(); ++it)
    {
        params.clear();
        ft->getFinishedParams_client(it->second, it->first, params);
        ft->addFile_gui(params, FALSE);
    }

    FinishedManager::getInstance()->unLockLists();
}

void FinishedTransfers::onOpen_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	int count = gtk_tree_selection_count_selected_rows(ft->fileSelection);

	if (count <= 0)
		return;

	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);
	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
		{
			string target = ft->fileView.getString(&iter, "Target");
			if (!target.empty())
				WulforUtil::openURI(target);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

}

void FinishedTransfers::onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data)
{
	((FinishedTransfers*)data)->updateStatus_gui(); // Switch the total count between users and files
}

void FinishedTransfers::onOpenFolder_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	int count = gtk_tree_selection_count_selected_rows(ft->fileSelection);

	if (count <= 0)
		return;

	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(ft->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(ft->fileStore), &iter, path))
		{
			string target = ft->fileView.getString(&iter, _("Path"));
			if (!target.empty())
				WulforUtil::openURI(target);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void FinishedTransfers::onRemoveItems_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;
	GtkTreeSelection *selection;
	TreeView *view;
	GtkListStore *store;
	if(gtk_notebook_get_current_page(GTK_NOTEBOOK(ft->getWidget("finishedbook"))) ==
			gtk_notebook_page_num(GTK_NOTEBOOK(ft->getWidget("finishedbook")), ft->getWidget("viewWindowFile")))
	{
		selection = ft->fileSelection;
		view = &ft->fileView;
		store = ft->fileStore;
	}
	else
	{
		selection = ft->userSelection;
		view = &ft->userView;
		store = ft->userStore;
	}

	string target;
	GtkTreeIter iter;
	GtkTreePath *path;
	GList *list = gtk_tree_selection_get_selected_rows(selection, NULL);

	typedef Func1<FinishedTransfers, string> F1;
	F1 *func;

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(store), &iter, path))
		{
			if (view == &ft->fileView)
			{
				target = view->getString(&iter, "Target");
				func = new F1(ft, &FinishedTransfers::removeFile_client, target);
			}
			else
			{
				target = view->getString(&iter, "CID");
				func = new F1(ft, &FinishedTransfers::removeUser_client, target);
			}


			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	ft->updateStatus_gui();		// Why? model won't change until after the _client call.
}

void FinishedTransfers::removeUser_gui(string target)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(userStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (target == userView.getString(&iter, "CID"))
		{
			gtk_list_store_remove(userStore, &iter);
			totalUsers--;
			updateStatus_gui();
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FinishedTransfers::removeFile_gui(string target)
{
	GtkTreeIter iter;
	GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
	bool valid = gtk_tree_model_get_iter_first(m, &iter);

	while (valid)
	{
		if (target == fileView.getString(&iter, "Target"))
		{
			totalBytes -= fileView.getValue<gint64>(&iter, _("Transferred"));
			totalTime -= fileView.getValue<gint64>(&iter, "Elapsed Time");
			gtk_list_store_remove(fileStore, &iter);
			totalFiles--;
			updateStatus_gui();
			return;
		}
		valid = gtk_tree_model_iter_next(m, &iter);
	}
}

void FinishedTransfers::onRemoveAll_gui(GtkMenuItem *item, gpointer data)
{
	FinishedTransfers *ft = (FinishedTransfers *)data;

	gtk_list_store_clear(ft->userStore);
	gtk_list_store_clear(ft->fileStore);
	ft->totalBytes = 0;
	ft->totalTime = 0;
	ft->totalFiles = 0;
	ft->totalUsers = 0;
	ft->updateStatus_gui();

	typedef Func0<FinishedTransfers> F0;
	F0 *func = new F0(ft, &FinishedTransfers::removeAll_client);
	WulforManager::get()->dispatchClientFunc(func);
}

void FinishedTransfers::initializeList_client()
{
	StringMap params;
	typedef Func2<FinishedTransfers, StringMap, bool> F2;
	//F2 *func;
	FinishedManager::getInstance()->lockLists();
	const FinishedManager::MapByFile &list = FinishedManager::getInstance()->getMapByFile(isUpload);
	const FinishedManager::MapByUser &user = FinishedManager::getInstance()->getMapByUser(isUpload);

	for (FinishedManager::MapByFile::const_iterator it = list.begin(); it != list.end(); ++it)
	{
		params.clear();
		getFinishedParams_client(it->second, it->first, params);
		addFile_gui(params, FALSE);
		//func = new F2(this, &FinishedTransfers::addItem_gui, params, FALSE);
		//WulforManager::get()->dispatchGuiFunc(func);
	}

	for (FinishedManager::MapByUser::const_iterator uit = user.begin(); uit != user.end(); ++uit)
	{
		params.clear();
		getFinishedParams_client(uit->second, uit->first, params);
		addUser_gui(params, FALSE);
		//func = new F2(this, &FinishedTransfers::addItem_gui, params, FALSE);
		//WulforManager::get()->dispatchGuiFunc(func);
	}

	FinishedManager::getInstance()->unLockLists();

	updateStatus_gui();
	//WulforManager::get()->dispatchGuiFunc(new Func0<FinishedTransfers>(this, &FinishedTransfers::updateStatus_gui));
}

void FinishedTransfers::getFinishedParams_client(const FinishedFileItemPtr& item, const string& file, StringMap &params)
{
	string nicks;
	params["Filename"] = Util::getFileName(file);
	params["Time"] = Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime());
	params["Path"] = Util::getFilePath(file);
	for (HintedUserList::const_iterator it = item->getUsers().begin(); it != item->getUsers().end(); ++it)//NOTE: core 0.762
	{
			nicks += WulforUtil::getNicks(it->user->getCID(), it->hint) + ", ";//NOTE: core 0.762
	}
	params["Nicks"] = nicks.substr(0, nicks.length() - 2);
	// item->getFileSize() seems to return crap. I guess there's no way to get
	// the real file size with this core version? Only the transferred part (I guess
	// the size could be asked from QueueManager (if the file isn't complete)?)
	params["Transferred"] = Util::toString(item->getTransferred());
	params["Speed"] = Util::toString(item->getAverageSpeed());
	params["CRC Checked"] = item->getCrc32Checked() ? _("Yes") : _("No");
	params["Target"] = file;
	params["Elapsed Time"] = Util::toString(item->getMilliSeconds());

    if (item->isFull())
        params["full"] = "1";
    else
        params["full"] = "0";
}

void FinishedTransfers::getFinishedParams_client(const FinishedUserItemPtr &item, const HintedUser &user, StringMap &params)//NOTE: core 0.762
{
	string files;
	params["Time"] = Util::formatTime("%Y-%m-%d %H:%M:%S", item->getTime());
	params["Nick"] = WulforUtil::getNicks(user);//NOTE: core 0.762
	params["Hub"] = WulforUtil::getHubNames(user);//NOTE: core 0.762
	for (StringList::const_iterator it = item->getFiles().begin(); it != item->getFiles().end(); ++it)
	{
		files += *it + ", ";
	}
	params["Files"] = files.substr(0, files.length() - 2);
	params["Transferred"] = Util::toString(item->getTransferred());
	params["Speed"] = Util::toString(item->getAverageSpeed());
	params["CID"] = user.user->getCID().toBase32();//NOTE: core 0.762
	params["Elapsed Time"] = Util::toString(item->getMilliSeconds());
}

void FinishedTransfers::removeUser_client(string cid)
{
	UserPtr user = ClientManager::getInstance()->findUser(CID(cid));

	if (user)
		// ignore the hint url user...
		FinishedManager::getInstance()->remove(isUpload, HintedUser(user, ""));//NOTE: core 0.762

}

void FinishedTransfers::removeFile_client(string target)
{

	if (!target.empty())
		FinishedManager::getInstance()->remove(isUpload, target);

}

void FinishedTransfers::removeAll_client()
{
	FinishedManager::getInstance()->removeAll(isUpload);
}

void FinishedTransfers::on(FinishedManagerListener::AddedFile, bool upload, const string& file, const FinishedFileItemPtr& item) throw()
{
	if (isUpload == upload)
	{
		StringMap params;
		getFinishedParams_client(item, file, params);

		typedef Func2<FinishedTransfers, StringMap, bool> F2;
		F2* func = new F2(this, &FinishedTransfers::addFile_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

//NOTE: core 0.762
void FinishedTransfers::on(FinishedManagerListener::AddedUser, bool upload, const HintedUser &user, const FinishedUserItemPtr &item) throw()
{
	if (isUpload == upload)
	{
		StringMap params;
		getFinishedParams_client(item, user, params);

		typedef Func2<FinishedTransfers, StringMap, bool> F2;
		F2 *func = new F2(this, &FinishedTransfers::addUser_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FinishedTransfers::on(FinishedManagerListener::UpdatedFile, bool upload, const string& file, const FinishedFileItemPtr& item) throw()
{
	if (isUpload == upload)
	{
		StringMap params;
		getFinishedParams_client(item, file, params);

		typedef Func2<FinishedTransfers, StringMap, bool> F2;
		F2 *func = new F2(this, &FinishedTransfers::addFile_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FinishedTransfers::on(FinishedManagerListener::UpdatedUser, bool upload, const HintedUser &user) throw()//NOTE: core 0.762
{
	if (isUpload == upload)
	{
		const FinishedManager::MapByUser &umap = FinishedManager::getInstance()->getMapByUser(isUpload);
		FinishedManager::MapByUser::const_iterator userit = umap.find(user);
		if (userit == umap.end())
			return;

		const FinishedUserItemPtr &item = userit->second;

		StringMap params;
		getFinishedParams_client(item, user, params);

		typedef Func2<FinishedTransfers, StringMap, bool> F2;
		F2 *func = new F2(this, &FinishedTransfers::addUser_gui, params, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FinishedTransfers::on(FinishedManagerListener::RemovedFile, bool upload, const string& item) throw()
{
	if (isUpload == upload)
	{
		typedef Func1<FinishedTransfers, string> F1;
		F1 *func = new F1(this, &FinishedTransfers::removeFile_gui, item);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void FinishedTransfers::on(FinishedManagerListener::RemovedUser, bool upload, const HintedUser &user) throw()//NOTE: core 0.762
{
	if (isUpload == upload)
	{
		typedef Func1<FinishedTransfers, string> F1;
		F1 *func = new F1(this, &FinishedTransfers::removeUser_gui, user.user->getCID().toBase32());//NOTE: core 0.762
		WulforManager::get()->dispatchGuiFunc(func);
	}
}
