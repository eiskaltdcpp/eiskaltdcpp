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

#include "downloadqueue.hh"

#include <dcpp/ResourceManager.h>
#include "search.hh"
#include "settingsmanager.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

DownloadQueue::DownloadQueue():
	BookEntry(Entry::DOWNLOAD_QUEUE, _("Download Queue"), "downloadqueue.glade"),
	currentItems(0),
	totalItems(0),
	currentSize(0),
	totalSize(0)
{
	// Configure the dialogs
	File::ensureDirectory(SETTING(DOWNLOAD_DIRECTORY));
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(getWidget("dirChooserDialog")), Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)).c_str());
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("dirChooserDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

	// menu
	g_object_ref_sink(getWidget("dirMenu"));
	g_object_ref_sink(getWidget("fileMenu"));

	// Initialize directory treeview
	dirView.setView(GTK_TREE_VIEW(getWidget("dirView")));
	dirView.insertColumn("Dir", G_TYPE_STRING, TreeView::ICON_STRING, -1, "Icon");
	dirView.insertHiddenColumn("Path", G_TYPE_STRING);
	dirView.insertHiddenColumn("File Count", G_TYPE_INT);
	dirView.insertHiddenColumn("Icon", G_TYPE_STRING);
	dirView.finalize();
	dirStore = gtk_tree_store_newv(dirView.getColCount(), dirView.getGTypes());
	gtk_tree_view_set_model(dirView.get(), GTK_TREE_MODEL(dirStore));
	g_object_unref(dirStore);
	dirSelection = gtk_tree_view_get_selection(dirView.get());
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(dirStore), dirView.col("Dir"), GTK_SORT_ASCENDING);
	gtk_tree_view_set_enable_tree_lines(dirView.get(), TRUE);

	// Initialize file treeview
	fileView.setView(GTK_TREE_VIEW(getWidget("fileView")), TRUE, "downloadqueue");
	fileView.insertColumn(_("Filename"), G_TYPE_STRING, TreeView::ICON_STRING, 200, "Icon");
	fileView.insertColumn(_("Status"), G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn(_("Size"), G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn(_("Downloaded"), G_TYPE_STRING, TreeView::STRING, 150);
	fileView.insertColumn(_("Priority"), G_TYPE_STRING, TreeView::STRING, 75);
	fileView.insertColumn(_("Users"), G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn(_("Path"), G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn(_("Exact Size"), G_TYPE_STRING, TreeView::STRING, 100);
	fileView.insertColumn(_("Errors"), G_TYPE_STRING, TreeView::STRING, 200);
	fileView.insertColumn(_("Added"), G_TYPE_STRING, TreeView::STRING, 120);
	fileView.insertColumn("TTH", G_TYPE_STRING, TreeView::STRING, 125);
	fileView.insertHiddenColumn("Size Sort", G_TYPE_INT64);
	fileView.insertHiddenColumn("Downloaded Sort", G_TYPE_INT64);
	fileView.insertHiddenColumn("Target", G_TYPE_STRING);
	fileView.insertHiddenColumn("Icon", G_TYPE_STRING);
	fileView.finalize();
	fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
	gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
	g_object_unref(fileStore);
	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
	fileSelection = gtk_tree_view_get_selection(fileView.get());
	fileView.setSortColumn_gui(_("Size"), "Size Sort");
	fileView.setSortColumn_gui(_("Exact Size"), "Size Sort");
	fileView.setSortColumn_gui(_("Downloaded"), "Downloaded Sort");
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col(_("Filename")), GTK_SORT_ASCENDING);
	gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col(_("Filename"))), TRUE);

	// Connect the signals to their callback functions.
	g_signal_connect(getWidget("pausedPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("lowestPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("lowPrioritytem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("normalPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("highPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("highestPriorityItem"), "activate", G_CALLBACK(onDirPriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("moveDirItem"), "activate", G_CALLBACK(onDirMoveClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("removeDirItem"), "activate", G_CALLBACK(onDirRemoveClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("searchForAlternatesItem"), "activate", G_CALLBACK(onFileSearchAlternatesClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("moveFileItem"), "activate", G_CALLBACK(onFileMoveClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("filePausedItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileLowestPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileLowPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileNormalPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileHighPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileHighestPriorityItem"), "activate", G_CALLBACK(onFilePriorityClicked_gui), (gpointer)this);
	g_signal_connect(getWidget("fileRemoveItem"), "activate", G_CALLBACK(onFileRemoveClicked_gui), (gpointer)this);
 	g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyMagnetClicked_gui), (gpointer)this);
 	g_signal_connect(dirView.get(), "button-press-event", G_CALLBACK(onDirButtonPressed_gui), (gpointer)this);
	g_signal_connect(dirView.get(), "button-release-event", G_CALLBACK(onDirButtonReleased_gui), (gpointer)this);
	g_signal_connect(dirView.get(), "key-release-event", G_CALLBACK(onDirKeyReleased_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-press-event", G_CALLBACK(onFileButtonPressed_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "button-release-event", G_CALLBACK(onFileButtonReleased_gui), (gpointer)this);
	g_signal_connect(fileView.get(), "key-release-event", G_CALLBACK(onFileKeyReleased_gui), (gpointer)this);

	// Set the pane position
	gtk_paned_set_position(GTK_PANED(getWidget("pane")), WGETI("downloadqueue-pane-position"));
	int panePosition = WGETI("downloadqueue-pane-position");
	if (panePosition > 10)
		gtk_paned_set_position(GTK_PANED(getWidget("pane")), panePosition);
}

DownloadQueue::~DownloadQueue()
{
	QueueManager::getInstance()->removeListener(this);

	// Save the pane position
	int panePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));
	if (panePosition > 10)
		WSET("downloadqueue-pane-position", panePosition);

	gtk_widget_destroy(getWidget("dirChooserDialog"));
	g_object_unref(getWidget("dirMenu"));
	g_object_unref(getWidget("fileMenu"));
}

void DownloadQueue::show()
{
	buildList_client();
	QueueManager::getInstance()->addListener(this);
}

void DownloadQueue::buildDynamicMenu_gui()
{
	bool showMenus = FALSE;
	bool showReAddMenu = FALSE;
	int count = gtk_tree_selection_count_selected_rows(fileSelection);

	if (count == 1)
	{
		GtkTreeIter iter;
		GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
		GtkTreePath *path = (GtkTreePath *)g_list_nth_data(list, 0);

		gtk_container_foreach(GTK_CONTAINER(getWidget("browseMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("pmMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("reAddMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("removeMenu")), (GtkCallback)gtk_widget_destroy, NULL);
		gtk_container_foreach(GTK_CONTAINER(getWidget("removeAllMenu")), (GtkCallback)gtk_widget_destroy, NULL);

		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fileStore), &iter, path))
		{
			GtkWidget *menuItem;
			string target = fileView.getString(&iter, "Target");

			///@todo: Fix this. sources & badSources should not be accessible from gui thread.
			for (SourceIter it = sources[target].begin(); it != sources[target].end(); ++it)
			{
				showMenus = TRUE;
				menuItem = gtk_menu_item_new_with_label(it->first.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("browseMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onFileGetListClicked_gui), (gpointer)this);

				menuItem = gtk_menu_item_new_with_label(it->first.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("pmMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onFileSendPMClicked_gui), (gpointer)this);

				menuItem = gtk_menu_item_new_with_label(it->first.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("removeMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onFileRemoveSourceClicked_gui), (gpointer)this);

				menuItem = gtk_menu_item_new_with_label(it->first.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("removeAllMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onFileRemoveUserFromQueueClicked_gui), (gpointer)this);
			}

			for (SourceIter it = badSources[target].begin(); it != badSources[target].end(); ++it)
			{
				showReAddMenu = TRUE;
				menuItem = gtk_menu_item_new_with_label(it->first.c_str());
				gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("reAddMenu")), menuItem);
				g_signal_connect(menuItem, "activate", G_CALLBACK(onFileReAddSourceClicked_gui), (gpointer)this);
			}
		}
		gtk_tree_path_free(path);
		g_list_free(list);

		gtk_widget_set_sensitive(getWidget("searchForAlternatesItem"), TRUE);
	}
	else
		gtk_widget_set_sensitive(getWidget("searchForAlternatesItem"), FALSE);

	if (showMenus)
	{
		gtk_widget_show_all(getWidget("browseMenu"));
		gtk_widget_show_all(getWidget("pmMenu"));
		gtk_widget_show_all(getWidget("removeMenu"));
		gtk_widget_show_all(getWidget("removeAllMenu"));
	}
	if (showReAddMenu)
		gtk_widget_show_all(getWidget("reAddMenu"));

	gtk_widget_set_sensitive(getWidget("getFileListItem"), showMenus);
	gtk_widget_set_sensitive(getWidget("sendPrivateMessageItem"), showMenus);
	gtk_widget_set_sensitive(getWidget("removeSourceItem"), showMenus);
	gtk_widget_set_sensitive(getWidget("removeUserFromQueueItem"), showMenus);
	gtk_widget_set_sensitive(getWidget("reAddSourceItem"), showReAddMenu);
}

void DownloadQueue::setStatus_gui(string text, string statusItem)
{
	if (!text.empty() && !statusItem.empty())
	{
		if (statusItem == "statusMain")
			text = "[" + Util::getShortTimeString() + "] " + text;

		gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusItem)), 0);
		gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusItem)), 0, text.c_str());
	}
}

void DownloadQueue::updateStatus_gui()
{
	setStatus_gui(_("Items: ") + Util::toString(currentItems), "statusItems");
	setStatus_gui(_("Size: ") + Util::formatBytes(currentSize), "statusFileSize");
	setStatus_gui(_("Files: ") + Util::toString(totalItems), "statusFiles");
	setStatus_gui(_("Size: ") + Util::formatBytes(totalSize), "statusTotalSize");
}

void DownloadQueue::addFiles_gui(vector<StringMap> files, bool firstUpdate)
{
	if (files.size() > 0 && currentDir == files[0]["Path"] &&
	    gtk_tree_selection_get_selected(dirSelection, NULL, NULL))
	{
		if (firstUpdate)
			gtk_list_store_clear(fileStore);

		gint sortColumn;
		GtkSortType sortType;
		gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(fileStore), &sortColumn, &sortType);
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, sortType);

		for (vector<StringMap>::const_iterator it = files.begin(); it != files.end(); ++it)
			addFile_gui(*it, FALSE);

		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), sortColumn, sortType);
		gtk_tree_view_scroll_to_point(fileView.get(), 0, 0);
	}
}

void DownloadQueue::addFile_gui(StringMap params, bool updateDirs)
{
	GtkTreeIter iter;
	int64_t size = Util::toInt64(params["Size Sort"]);

	if (gtk_tree_selection_get_selected(dirSelection, NULL, NULL) && currentDir == params["Path"])
	{
		++currentItems;
		currentSize += size;

		gtk_list_store_append(fileStore, &iter);
		gtk_list_store_set(fileStore, &iter,
			fileView.col(_("Filename")), params["Filename"].c_str(),
			fileView.col(_("Users")), params["Users"].c_str(),
			fileView.col(_("Status")), params["Status"].c_str(),
			fileView.col(_("Size")), params["Size"].c_str(),
			fileView.col(_("Exact Size")), params["Exact Size"].c_str(),
			fileView.col("Size Sort"), size,
			fileView.col(_("Downloaded")), params["Downloaded"].c_str(),
			fileView.col("Downloaded Sort"), Util::toInt64(params["Downloaded Sort"]),
			fileView.col(_("Priority")), params["Priority"].c_str(),
			fileView.col(_("Path")), params["Path"].c_str(),
			fileView.col(_("Errors")), params["Errors"].c_str(),
			fileView.col(_("Added")), params["Added"].c_str(),
			fileView.col("TTH"), params["TTH"].c_str(),
			fileView.col("Target"), params["Target"].c_str(),
			fileView.col("Icon"), "icon-file",
			-1);

		if (WGETB("bold-queue"))
			setBold_gui();
	}

	if (updateDirs)
	{
		++totalItems;
		totalSize += size;

		// Ensure root node
		if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirStore), &iter))
		{
			gtk_tree_store_append(dirStore, &iter, NULL);
			gtk_tree_store_set(dirStore, &iter,
				dirView.col("Dir"), "/",
				dirView.col("Path"), "/",
				dirView.col("File Count"), 0,
				dirView.col("Icon"), "icon-directory",
				-1);
		}

		if (params["Path"].length() > 1)
			addDir_gui(params["Path"].substr(1), &iter);
	}

	updateStatus_gui();
}

void DownloadQueue::addDir_gui(const string &path, GtkTreeIter *parent)
{
	if (path.empty() || parent == NULL)
		return;

	GtkTreeIter iter;
	string::size_type i = path.find_first_of(PATH_SEPARATOR);
	const string &dir = path.substr(0, i);
	const string &fullpath = dirView.getString(parent, "Path") + dir + PATH_SEPARATOR_STR;
	bool valid = gtk_tree_model_iter_children(GTK_TREE_MODEL(dirStore), &iter, parent);

	while (valid)
	{
		if (dir == dirView.getString(&iter, "Dir"))
		{
			addDir_gui(path.substr(i + 1), &iter);

			if (fullpath == dirView.getString(parent, "Path") + path)
			{
				int count = dirView.getValue<gint>(&iter, "File Count");
				gtk_tree_store_set(dirStore, &iter, dirView.col("File Count"), ++count, -1);
			}

			return;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(dirStore), &iter);
	}

	gtk_tree_store_append(dirStore, &iter, parent);
	gtk_tree_store_set(dirStore, &iter,
		dirView.col("Dir"), dir.c_str(),
		dirView.col("Path"), fullpath.c_str(),
		dirView.col("File Count"), 0,
		dirView.col("Icon"), "icon-directory",
		-1);

	GtkTreePath *treePath = gtk_tree_model_get_path(GTK_TREE_MODEL(dirStore), parent);
	gtk_tree_view_expand_row(dirView.get(), treePath, FALSE);
	gtk_tree_path_free(treePath);

	if (fullpath == dirView.getString(parent, "Path") + path)
	{
		int count = dirView.getValue<gint>(&iter, "File Count");
		gtk_tree_store_set(dirStore, &iter, dirView.col("File Count"), ++count, -1);
	}

	addDir_gui(path.substr(i + 1), &iter);
}

void DownloadQueue::updateFile_gui(StringMap params)
{
	if (gtk_tree_selection_get_selected(dirSelection, NULL, NULL) && currentDir == params["Path"])
	{
		GtkTreeIter iter;
		GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
		bool valid = gtk_tree_model_get_iter_first(m, &iter);

		while (valid)
		{
			if (fileView.getString(&iter, "Target") == params["Target"])
			{
				gtk_list_store_set(fileStore, &iter,
					fileView.col(_("Filename")), params["Filename"].c_str(),
					fileView.col(_("Users")), params["Users"].c_str(),
					fileView.col(_("Status")), params["Status"].c_str(),
					fileView.col(_("Size")), params["Size"].c_str(),
					fileView.col(_("Exact Size")), params["Exact Size"].c_str(),
					fileView.col("Size Sort"), Util::toInt64(params["Size Sort"]),
					fileView.col(_("Downloaded")), params["Downloaded"].c_str(),
					fileView.col("Downloaded Sort"), Util::toInt64(params["Downloaded Sort"]),
					fileView.col(_("Priority")), params["Priority"].c_str(),
					fileView.col(_("Path")), params["Path"].c_str(),
					fileView.col(_("Errors")), params["Errors"].c_str(),
					fileView.col(_("Added")), params["Added"].c_str(),
					fileView.col("TTH"), params["TTH"].c_str(),
					fileView.col("Target"), params["Target"].c_str(),
					fileView.col("Icon"), "icon-file",
					-1);
				return;
			}
			valid = gtk_tree_model_iter_next(m, &iter);
		}
	}
}

void DownloadQueue::removeFile_gui(string target, int64_t size)
{
	GtkTreeIter iter;
	bool valid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(fileStore), &iter);
	string path = Util::getFilePath(target);

	while (valid)
	{
		if (target == fileView.getString(&iter, "Target"))
		{
			--currentItems;
			currentSize -= size;
			gtk_list_store_remove(fileStore, &iter);
			break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(fileStore), &iter);
	}

	--totalItems;
	totalSize -= size;

	if (path.length() > 1 && gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirStore), &iter))
		removeDir_gui(path.substr(1), &iter);

	updateStatus_gui();

	if (WGETB("bold-queue"))
		setBold_gui();
}

void DownloadQueue::removeDir_gui(const string &path, GtkTreeIter *parent)
{
	if (path.empty() || parent == NULL)
		return;

	GtkTreeIter iter;
	string curPath;
	int count = 0;
	string::size_type i = path.find_first_of(PATH_SEPARATOR);
	string dir = path.substr(0, i);
	string fullpath = dirView.getString(parent, "Path") + path;
	bool valid = gtk_tree_model_iter_children(GTK_TREE_MODEL(dirStore), &iter, parent);

	while (valid)
	{
		if (dir == dirView.getString(&iter, "Dir"))
		{
			removeDir_gui(path.substr(i + 1), &iter);

			curPath = dirView.getString(&iter, "Path");
			count = dirView.getValue<gint>(&iter, "File Count");
			if (curPath == fullpath)
				gtk_tree_store_set(dirStore, &iter, dirView.col("File Count"), --count, -1);

			// No files in leaf node
			if (count <= 0 && !gtk_tree_model_iter_has_child(GTK_TREE_MODEL(dirStore), &iter))
			{
				gtk_tree_store_remove(dirStore, &iter);
				if (currentDir == curPath)
				{
					gtk_list_store_clear(fileStore);
					currentDir.clear();
				}
			}

			break;
		}
		valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(dirStore), &iter);
	}
}

void DownloadQueue::updateFileView_gui()
{
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
	{
		string dir = dirView.getString(&iter, "Path");
		if (dir != currentDir)
		{
			gtk_list_store_clear(fileStore);
			currentDir = dir;
			currentItems = 0;
			currentSize = 0;

			typedef Func1<DownloadQueue, string> F1;
			F1 *func = new F1(this, &DownloadQueue::updateFileView_client, currentDir);
			WulforManager::get()->dispatchClientFunc(func);
		}
	}
}

void DownloadQueue::sendMessage_gui(string cid)
{
	if (!cid.empty())
		WulforManager::get()->getMainWindow()->addPrivateMessage_gui(Msg::UNKNOWN, cid);
}

gboolean DownloadQueue::onDirButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	dq->dirPrevious = event->type;

	return FALSE;
}

gboolean DownloadQueue::onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;

	if (dq->dirPrevious == GDK_BUTTON_PRESS && gtk_tree_selection_get_selected(dq->dirSelection, NULL, NULL))
	{
		if (event->button == 1)
		{
			dq->updateFileView_gui();
		}
		else if (event->button == 3)
		{
			dq->updateFileView_gui();
			gtk_menu_popup(GTK_MENU(dq->getWidget("dirMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

gboolean DownloadQueue::onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			dq->onDirRemoveClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			gtk_menu_popup(GTK_MENU(dq->getWidget("dirMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
		}
		else if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up ||
			event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
		{
			dq->updateFileView_gui();
		}
		else if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter)
		{
			GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(dq->dirStore), &iter);
			if (gtk_tree_view_row_expanded(dq->dirView.get(), path))
				gtk_tree_view_collapse_row(dq->dirView.get(), path);
			else
				gtk_tree_view_expand_row(dq->dirView.get(), path, FALSE);
			gtk_tree_path_free(path);
		}
	}

	return FALSE;
}

gboolean DownloadQueue::onFileButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;

	if (event->button == 3)
	{
		GtkTreePath *path;
		if (gtk_tree_view_get_path_at_pos(dq->fileView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
		{
			bool selected = gtk_tree_selection_path_is_selected(dq->fileSelection, path);
			gtk_tree_path_free(path);

			if (selected)
				return TRUE;
		}
	}
	return FALSE;
}

gboolean DownloadQueue::onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;

	if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
	{
		if (gtk_tree_selection_count_selected_rows(dq->fileSelection) > 0)
		{
			dq->buildDynamicMenu_gui();
			gtk_menu_popup(GTK_MENU(dq->getWidget("fileMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
			return TRUE;
		}
	}
	return FALSE;
}

gboolean DownloadQueue::onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);

	if (count > 0)
	{
		if (event->keyval == GDK_Delete || event->keyval == GDK_BackSpace)
		{
			dq->onFileRemoveClicked_gui(NULL, data);
		}
		else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
		{
			dq->buildDynamicMenu_gui();
			gtk_menu_popup(GTK_MENU(dq->getWidget("fileMenu")), NULL, NULL,
				NULL, NULL, 0, gtk_get_current_event_time());
		}
	}

	return FALSE;
}

void DownloadQueue::onDirPriorityClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		string path = dq->dirView.getString(&iter, "Path");
		QueueItem::Priority priority;

		if (item == GTK_MENU_ITEM(dq->getWidget("pausedPriorityItem")))
			priority = QueueItem::PAUSED;
		else if (item == GTK_MENU_ITEM(dq->getWidget("lowestPriorityItem")))
			priority = QueueItem::LOWEST;
		else if (item == GTK_MENU_ITEM(dq->getWidget("lowPrioritytem")))
			priority = QueueItem::LOW;
		else if (item == GTK_MENU_ITEM(dq->getWidget("highPriorityItem")))
			priority = QueueItem::HIGH;
		else if (item == GTK_MENU_ITEM(dq->getWidget("highestPriorityItem")))
			priority = QueueItem::HIGHEST;
		else
			priority = QueueItem::NORMAL;

		typedef Func2<DownloadQueue, string, QueueItem::Priority> F2;
		F2 *func = new F2(dq, &DownloadQueue::setPriorityDir_client, path, priority);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DownloadQueue::onDirMoveClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		string path = Text::fromUtf8(dq->dirView.getString(&iter, "Path"));
		GtkWidget *dialog = dq->getWidget("dirChooserDialog");
		gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path.c_str());
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		// Widget failed if the dialog gets programmatically destroyed.
		if (response == GTK_RESPONSE_NONE)
			return;

		gtk_widget_hide(dialog);

		if (response == GTK_RESPONSE_OK)
		{
			gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
			if (temp)
			{
				string target = Text::toUtf8(temp);
				g_free(temp);

				if (target[target.length() - 1] != PATH_SEPARATOR)
					target += PATH_SEPARATOR;

				typedef Func2<DownloadQueue, string, string> F2;
				F2 *func = new F2(dq, &DownloadQueue::moveDir_client, path, target);
				WulforManager::get()->dispatchClientFunc(func);
			}
		}
	}
}

void DownloadQueue::onDirRemoveClicked_gui(GtkMenuItem *menuitem, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreeIter iter;

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		string path = dq->dirView.getString(&iter, "Path");
		gtk_list_store_clear(dq->fileStore);

		typedef Func1<DownloadQueue, string> F1;
		F1 *func = new F1(dq, &DownloadQueue::removeDir_client, path);
		WulforManager::get()->dispatchClientFunc(func);
	}
}

void DownloadQueue::onFileSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	string tth;
	GtkTreePath *path;
	GtkTreeIter iter;
	Search *s;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			tth = dq->fileView.getString(&iter, "TTH");
			if (!tth.empty())
			{
				s = WulforManager::get()->getMainWindow()->addSearch_gui();
				s->putValue_gui(tth, 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onCopyMagnetClicked_gui(GtkMenuItem* item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	GtkTreePath *path;
	GtkTreeIter iter;
	string magnets, magnet, filename, tth;
	int64_t size;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			filename = dq->fileView.getString(&iter, _("Filename"));
			size = dq->fileView.getValue<int64_t>(&iter, "Size Sort");
			tth = dq->fileView.getString(&iter, "TTH");
			magnet = WulforUtil::makeMagnet(filename, size, tth);

			if (!magnet.empty())
			{
				if (!magnets.empty())
					magnets += '\n';
				magnets += magnet;
			}
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);

	if (!magnets.empty())
		gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), magnets.c_str(), magnets.length());
}

void DownloadQueue::onFileMoveClicked_gui(GtkMenuItem *menuItem, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, string> F2;
	F2 *func;
	string source;
	GtkTreePath *path;
	GtkTreeIter iter;
	int count = gtk_tree_selection_count_selected_rows(dq->fileSelection);
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);
	GtkWidget *dialog = dq->getWidget("dirChooserDialog");

	if (gtk_tree_selection_get_selected(dq->dirSelection, NULL, &iter))
	{
		string filepath = Text::fromUtf8(dq->dirView.getString(&iter, "Path"));
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), filepath.c_str());
	}

	if (count == 1)
	{
		path = (GtkTreePath *)list->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			string target = Text::fromUtf8(dq->fileView.getString(&iter, _("Filename")));
			gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SAVE);
			gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), target.c_str());
			gint response = gtk_dialog_run(GTK_DIALOG(dialog));

			// Widget failed if the dialog gets programmatically destroyed.
			if (response != GTK_RESPONSE_NONE)
				gtk_widget_hide(dialog);

			if (response == GTK_RESPONSE_OK)
			{
				gchar *tmp = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
				if (tmp)
				{
					target = Text::toUtf8(tmp);
					g_free(tmp);
					source = dq->fileView.getString(&iter, "Target");
					func = new F2(dq, &DownloadQueue::move_client, source, target);
					WulforManager::get()->dispatchClientFunc(func);
				}
			}
		}
		gtk_tree_path_free(path);
	}
	else if (count > 1)
	{
		gtk_file_chooser_set_action(GTK_FILE_CHOOSER(dialog), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		// Widget failed if the dialog gets programmatically destroyed.
		if (response != GTK_RESPONSE_NONE)
			gtk_widget_hide(dialog);

		if (response == GTK_RESPONSE_OK)
		{
			gchar *tmp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(dialog));
			if (tmp)
			{
				string filename;
				string target = Text::toUtf8(tmp);
				g_free(tmp);

				if (target[target.length() - 1] != PATH_SEPARATOR)
					target += PATH_SEPARATOR;

				for (GList *i = list; i; i = i->next)
				{
					path = (GtkTreePath *)i->data;
					if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
					{
						source = dq->fileView.getString(&iter, "Target");
						filename = dq->fileView.getString(&iter, _("Filename"));
						func = new F2(dq, &DownloadQueue::move_client, source, target + filename);
						WulforManager::get()->dispatchClientFunc(func);
					}
					gtk_tree_path_free(path);
				}
			}
		}
	}
	g_list_free(list);
}

void DownloadQueue::onFilePriorityClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, QueueItem::Priority> F2;
	F2 *func;
	string target;
	GtkTreePath *path;
	GtkTreeIter iter;
	QueueItem::Priority priority;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");

			if (item == GTK_MENU_ITEM(dq->getWidget("filePausedItem")))
				priority = QueueItem::PAUSED;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileLowestPriorityItem")))
				priority = QueueItem::LOWEST;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileLowPriorityItem")))
				priority = QueueItem::LOW;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileHighPriorityItem")))
				priority = QueueItem::HIGH;
			else if (item == GTK_MENU_ITEM(dq->getWidget("fileHighestPriorityItem")))
				priority = QueueItem::HIGHEST;
			else
				priority = QueueItem::NORMAL;

			func = new F2(dq, &DownloadQueue::setPriority_client, target, priority);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onFileGetListClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, string> F2;
	F2 *func;
	string nick, target;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");
			nick = WulforUtil::getTextFromMenu(item);

			func = new F2(dq, &DownloadQueue::addList_client, target, nick);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onFileSendPMClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, string> F2;
	F2 *func;
	string nick, target;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");
			nick = WulforUtil::getTextFromMenu(item);

			func = new F2(dq, &DownloadQueue::sendMessage_client, target, nick);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onFileReAddSourceClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, string> F2;
	F2 *func;
	string nick, target;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");
			nick = WulforUtil::getTextFromMenu(item);

			func = new F2(dq, &DownloadQueue::reAddSource_client, target, nick);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onFileRemoveSourceClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, string> F2;
	F2 *func;
	string nick, target;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");
			nick = WulforUtil::getTextFromMenu(item);

			func = new F2(dq, &DownloadQueue::removeSource_client, target, nick);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onFileRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func2<DownloadQueue, string, string> F2;
	F2 *func;
	string nick, target;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");
			nick = WulforUtil::getTextFromMenu(item);

			func = new F2(dq, &DownloadQueue::removeSources_client, target, nick);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::onFileRemoveClicked_gui(GtkMenuItem *menuitem, gpointer data)
{
	DownloadQueue *dq = (DownloadQueue *)data;
	typedef Func1<DownloadQueue, string> F1;
	F1 *func;
	string target;
	GtkTreePath *path;
	GtkTreeIter iter;
	GList *list = gtk_tree_selection_get_selected_rows(dq->fileSelection, NULL);

	for (GList *i = list; i; i = i->next)
	{
		path = (GtkTreePath *)i->data;
		if (gtk_tree_model_get_iter(GTK_TREE_MODEL(dq->fileStore), &iter, path))
		{
			target = dq->fileView.getString(&iter, "Target");
			func = new F1(dq, &DownloadQueue::remove_client, target);
			WulforManager::get()->dispatchClientFunc(func);
		}
		gtk_tree_path_free(path);
	}
	g_list_free(list);
}

void DownloadQueue::buildList_client()
{
	StringMap params;
	typedef Func2<DownloadQueue, StringMap, bool> F2;
	//F2 *func;
	const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

	for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
	{
		params["Size Sort"] = Util::toString(it->second->getSize());
		params["Path"] = Util::getFilePath(*it->first);

		addFile_gui(params, TRUE);
		//func = new F2(this, &DownloadQueue::addFile_gui, params, TRUE);
		//WulforManager::get()->dispatchGuiFunc(func);
	}

	QueueManager::getInstance()->unlockQueue();
}

void DownloadQueue::move_client(string source, string target)
{
	if (!source.empty() && !target.empty())
		QueueManager::getInstance()->move(source, target);
}

void DownloadQueue::moveDir_client(string source, string target)
{
	if (!source.empty() && !target.empty() && target[target.length() - 1] == PATH_SEPARATOR)
	{
		// Can't modify QueueItem::StringMap in the loop, so we have to queue them.
		vector<string> targets;
		string *file;
		const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

		for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
		{
			file = it->first;
			if (file->length() >= source.length() && file->substr(0, source.length()) == source)
				targets.push_back(*file);
		}
		QueueManager::getInstance()->unlockQueue();

		for (vector<string>::const_iterator it = targets.begin(); it != targets.end(); ++it)
			QueueManager::getInstance()->move(*it, target + it->substr(source.length()));
	}
}

void DownloadQueue::setPriority_client(string target, QueueItem::Priority p)
{
	if (!target.empty())
		QueueManager::getInstance()->setPriority(target, p);
}

void DownloadQueue::setPriorityDir_client(string path, QueueItem::Priority p)
{
	if (!path.empty() && path[path.length() - 1] == PATH_SEPARATOR)
	{
		string *file;
		const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

		for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
		{
			file = it->first;
			if (file->length() >= path.length() && file->substr(0, path.length()) == path)
				QueueManager::getInstance()->setPriority(*file, p);
		}
		QueueManager::getInstance()->unlockQueue();
	}
}

void DownloadQueue::addList_client(string target, string nick)
{
	try
	{
		if (!target.empty() && !nick.empty() && sources.find(target) != sources.end())
		{
			SourceIter it = sources[target].find(nick);
			if (it != sources[target].end())
			{
				UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
				if (user)
					QueueManager::getInstance()->addList(user, "", QueueItem::FLAG_CLIENT_VIEW);
			}
		}
	}
	catch (const Exception &e)
	{
		typedef Func2<DownloadQueue, string, string> F2;
		F2 *func = new F2(this, &DownloadQueue::setStatus_gui, e.getError(), "statusMain");
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void DownloadQueue::sendMessage_client(string target, string nick)
{
	if (!target.empty() && !nick.empty() && sources.find(target) != sources.end())
	{
		SourceIter it = sources[target].find(nick);
		if (it != sources[target].end())
		{
			typedef Func1<DownloadQueue, string> F1;
			F1 *func = new F1(this, &DownloadQueue::sendMessage_gui, it->second);
			WulforManager::get()->dispatchGuiFunc(func);
		}
	}
}

void DownloadQueue::reAddSource_client(string target, string nick)
{
	try
	{
		if (!target.empty() && !nick.empty() && badSources.find(target) != sources.end())
		{
			SourceIter it = badSources[target].find(nick);
			if (it != badSources[target].end())
			{
				UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
				if (user)
					QueueManager::getInstance()->readd(target, user, "");
			}
		}
	}
	catch (const Exception &e)
	{
		typedef Func2<DownloadQueue, string, string> F2;
		F2 *func = new F2(this, &DownloadQueue::setStatus_gui, e.getError(), "statusMain");
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void DownloadQueue::removeSource_client(string target, string nick)
{
	if (!target.empty() && !nick.empty() && sources.find(target) != sources.end())
	{
		SourceIter it = sources[target].find(nick);
		if (it != sources[target].end())
		{
			UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
			if (user)
				QueueManager::getInstance()->removeSource(target, user, QueueItem::Source::FLAG_REMOVED);
		}
	}
}

void DownloadQueue::removeSources_client(string target, string nick)
{
	if (!target.empty() && !nick.empty() && sources.find(target) != sources.end())
	{
		SourceIter it = sources[target].find(nick);
		if (it != sources[target].end())
		{
			UserPtr user = ClientManager::getInstance()->findUser(CID(it->second));
			if (user)
				QueueManager::getInstance()->removeSource(user, QueueItem::Source::FLAG_REMOVED);
		}
	}
}

void DownloadQueue::remove_client(string target)
{
	if (!target.empty())
		QueueManager::getInstance()->remove(target);
}

void DownloadQueue::removeDir_client(string path)
{
	if (!path.empty())
	{
		string *file;
		vector<string> targets;
		const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

		for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
		{
			file = it->first;
			if (file->length() >= path.length() && file->substr(0, path.length()) == path)
				targets.push_back(*file);
		}
		QueueManager::getInstance()->unlockQueue();

		for (vector<string>::const_iterator it = targets.begin(); it != targets.end(); ++it)
			QueueManager::getInstance()->remove(*it);
	}
}

void DownloadQueue::updateFileView_client(string path)
{
	if (!path.empty())
	{
		vector<StringMap> files;
		const QueueItem::StringMap &ll = QueueManager::getInstance()->lockQueue();

		for (QueueItem::StringMap::const_iterator it = ll.begin(); it != ll.end(); ++it)
		{
			if (it->first->length() >= path.length() && it->first->substr(0, it->first->rfind('/') + 1) == path)
			{
				StringMap params;
				getQueueParams_client(it->second, params);
				files.push_back(params);
			}
		}
		QueueManager::getInstance()->unlockQueue();

		// Updating gui is smoother if we do it in large chunks.
		typedef Func2<DownloadQueue, vector<StringMap>, bool> F2;
		F2 *func = new F2(this, &DownloadQueue::addFiles_gui, files, TRUE);
		WulforManager::get()->dispatchGuiFunc(func);
	}
}

void DownloadQueue::getQueueParams_client(QueueItem *item, StringMap &params)
{
	string nick;
	map<string, string> source;
	int online = 0;

	params["Filename"] = item->getTargetFileName();
	params["Path"] = Util::getFilePath(item->getTarget());
	params["Target"] = item->getTarget();

	params["Users"] = "";
	for (QueueItem::SourceConstIter it = item->getSources().begin(); it != item->getSources().end(); ++it)
	{
		if (it->getUser()->isOnline())
			++online;

		if (params["Users"].size() > 0)
			params["Users"] += ", ";

		nick = WulforUtil::getNicks(it->getUser());
		source[nick] = it->getUser()->getCID().toBase32();
		params["Users"] += nick;
	}
	if (params["Users"].empty())
		params["Users"] = _("No users");
	sources[item->getTarget()] = source;

	// Status
	if (item->isWaiting())
		params["Status"] = Util::toString(online) + _(" of ") + Util::toString(item->getSources().size()) + _(" user(s) online");
	else
		params["Status"] = _("Running...");

	// Size
	params["Size Sort"] = Util::toString(item->getSize());
	if (item->getSize() < 0)
	{
		params["Size"] = _("Unknown");
		params["Exact Size"] = _("Unknown");
	}
	else
	{
		params["Size"] = Util::formatBytes(item->getSize());
		params["Exact Size"] = Util::formatExactSize(item->getSize());
	}

	// Downloaded
	params["Downloaded Sort"] = Util::toString(item->getDownloadedBytes());
	if (item->getSize() > 0)
	{
		double percent = (double)item->getDownloadedBytes() * 100.0 / (double)item->getSize();
		params["Downloaded"] = Util::formatBytes(item->getDownloadedBytes()) + " (" + Util::toString(percent) + "%)";
	}
	else
	{
		params["Downloaded"] = "0 B (0.00%)";
	}

	// Priority
	switch (item->getPriority())
	{
		case QueueItem::PAUSED:
			params["Priority"] = _("Paused");
			break;
		case QueueItem::LOWEST:
			params["Priority"] = _("Lowest");
			break;
		case QueueItem::LOW:
			params["Priority"] = _("Low");
			break;
		case QueueItem::HIGH:
			params["Priority"] = _("High");
			break;
		case QueueItem::HIGHEST:
			params["Priority"] = _("Highest");
			break;
		default:
			params["Priority"] = _("Normal");
	}

	// Error
	source.clear();
	params["Errors"] = "";
	for (QueueItem::SourceConstIter it = item->getBadSources().begin(); it != item->getBadSources().end(); ++it)
	{
		nick = WulforUtil::getNicks(it->getUser());
		source[nick] = it->getUser()->getCID().toBase32();

		if (!it->isSet(QueueItem::Source::FLAG_REMOVED))
		{
			if (params["Errors"].size() > 0)
				params["Errors"] += ", ";
			params["Errors"] += nick + " (";

			if (it->isSet(QueueItem::Source::FLAG_FILE_NOT_AVAILABLE))
				params["Errors"] += _("File not available");
			else if (it->isSet(QueueItem::Source::FLAG_PASSIVE))
				params["Errors"] += _("Passive user");
			else if (it->isSet(QueueItem::Source::FLAG_CRC_FAILED))
				params["Errors"] += _("CRC32 inconsistency (SFV-Check)");
			else if (it->isSet(QueueItem::Source::FLAG_BAD_TREE))
				params["Errors"] += _("Full tree does not match TTH root");
			else if (it->isSet(QueueItem::Source::FLAG_SLOW_SOURCE))
				params["Errors"] += _("Source too slow");
			else if (it->isSet(QueueItem::Source::FLAG_NO_TTHF))
				params["Errors"] += _("Remote client does not fully support TTH - cannot download");

			params["Errors"] += ")";
		}
	}
	if (params["Errors"].empty())
		params["Errors"] = _("No errors");
	badSources[item->getTarget()] = source;

	// Added
	params["Added"] = Util::formatTime("%Y-%m-%d %H:%M", item->getAdded());

	// TTH
	params["TTH"] = item->getTTH().toBase32();
}

void DownloadQueue::on(QueueManagerListener::Added, QueueItem *item) throw()
{
	StringMap params;
	getQueueParams_client(item, params);

	typedef Func2<DownloadQueue, StringMap, bool> F2;
	F2 *func = new F2(this, &DownloadQueue::addFile_gui, params, TRUE);
	WulforManager::get()->dispatchGuiFunc(func);
}

void DownloadQueue::on(QueueManagerListener::Moved, QueueItem *item, const string &oldTarget) throw()
{
	// Remove the old file
	typedef Func2<DownloadQueue, string, int64_t> F2a;
	F2a *func1 = new F2a(this, &DownloadQueue::removeFile_gui, oldTarget, item->getSize());
	WulforManager::get()->dispatchGuiFunc(func1);

	// Add the new file
	StringMap params;
	getQueueParams_client(item, params);

	typedef Func2<DownloadQueue, StringMap, bool> F2b;
	F2b *func2 = new F2b(this, &DownloadQueue::addFile_gui, params, TRUE);
	WulforManager::get()->dispatchGuiFunc(func2);
}

void DownloadQueue::on(QueueManagerListener::Removed, QueueItem *item) throw()
{
	typedef Func2<DownloadQueue, string, int64_t> F2;
	F2 *func = new F2(this, &DownloadQueue::removeFile_gui, item->getTarget(), item->getSize());
	WulforManager::get()->dispatchGuiFunc(func);
}

void DownloadQueue::on(QueueManagerListener::SourcesUpdated, QueueItem *item) throw()
{
	StringMap params;
	getQueueParams_client(item, params);

	typedef Func1<DownloadQueue, StringMap> F1;
	F1 *func = new F1(this, &DownloadQueue::updateFile_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}

void DownloadQueue::on(QueueManagerListener::StatusUpdated, QueueItem *item) throw()
{
	StringMap params;
	getQueueParams_client(item, params);

	typedef Func1<DownloadQueue, StringMap> F1;
	F1 *func = new F1(this, &DownloadQueue::updateFile_gui, params);
	WulforManager::get()->dispatchGuiFunc(func);
}
