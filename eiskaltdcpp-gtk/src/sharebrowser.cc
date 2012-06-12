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

#include "sharebrowser.hh"
#include <dcpp/FavoriteManager.h>
#include <dcpp/ShareManager.h>
#include <dcpp/Text.h>
#include <dcpp/ADLSearch.h>
#include "search.hh"
#include "settingsmanager.hh"
#include "UserCommandMenu.hh"
#include "wulformanager.hh"
#include "WulforUtil.hh"

using namespace std;
using namespace dcpp;

ShareBrowser::ShareBrowser(UserPtr user, const string &file, const string &initialDirectory, bool full):
    BookEntry(Entry::SHARE_BROWSER, _("List: ") + WulforUtil::getNicks(user, ""), "sharebrowser.ui", user->getCID().toBase32()),
    user(user),
    file(file),
    initialDirectory(initialDirectory),
    listing(HintedUser(user, "")),
    shareSize(0),
    currentSize(0),
    shareItems(0),
    currentItems(0),
    updateFileView(TRUE),
    skipHits(0),
    full(full)
{
#if !GTK_CHECK_VERSION(3,0,0)
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("mainStatus")),FALSE);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("itemsStatus")),FALSE);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("sizeStatus")),FALSE);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("filesStatus")),FALSE);
    gtk_statusbar_set_has_resize_grip (GTK_STATUSBAR(getWidget("totalStatus")),FALSE);
#endif

    // Use the nick from the file name in case the user is offline and core only returns CID
    nick = WulforUtil::getNicks(user, "");
    if (nick.find(user->getCID().toBase32(), 1) != string::npos)
    {
        string name = Util::getFileName(file);
        string::size_type loc = name.find('.');
        nick = name.substr(0, loc);
        setLabel_gui(_("List: ") + nick);
    }

    // Configure the dialogs
    File::ensureDirectory(SETTING(DOWNLOAD_DIRECTORY));
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("findDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(getWidget("dirChooserDialog")), Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)).c_str());
    gtk_dialog_set_alternative_button_order(GTK_DIALOG(getWidget("dirChooserDialog")), GTK_RESPONSE_OK, GTK_RESPONSE_CANCEL, -1);

    // menu
    g_object_ref_sink(getWidget("dirMenu"));
    g_object_ref_sink(getWidget("fileMenu"));

    // Set the pane position
    gtk_paned_set_position(GTK_PANED(getWidget("pane")), WGETI("sharebrowser-pane-position"));

    // Initialize the file TreeView
    fileView.setView(GTK_TREE_VIEW(getWidget("fileView")), true, "sharebrowser");
    fileView.insertColumn(_("Filename"), G_TYPE_STRING, TreeView::ICON_STRING, 400, "Icon");
    fileView.insertColumn(_("Size"), G_TYPE_STRING, TreeView::STRINGR, 80);
    fileView.insertColumn(_("Type"), G_TYPE_STRING, TreeView::STRING, 50);
    fileView.insertColumn(_("TTH"), G_TYPE_STRING, TreeView::STRING, 150);
    fileView.insertColumn(_("Exact Size"), G_TYPE_STRING, TreeView::STRINGR, 105);
    fileView.insertColumn(_("Bitrate"), G_TYPE_STRING, TreeView::STRING, 50);
    fileView.insertColumn(_("Resolution"), G_TYPE_STRING, TreeView::STRING, 70);
    fileView.insertColumn(_("Video"), G_TYPE_STRING, TreeView::STRING, 100);
    fileView.insertColumn(_("Audio"), G_TYPE_STRING, TreeView::STRING, 70);
    fileView.insertHiddenColumn("DL File", G_TYPE_POINTER);
    fileView.insertHiddenColumn("Icon", G_TYPE_STRING);
    fileView.insertHiddenColumn("Size Order", G_TYPE_INT64);
    fileView.insertHiddenColumn("File Order", G_TYPE_STRING);
    fileView.finalize();
    fileStore = gtk_list_store_newv(fileView.getColCount(), fileView.getGTypes());
    gtk_tree_view_set_model(fileView.get(), GTK_TREE_MODEL(fileStore));
    g_object_unref(fileStore);
    fileSelection = gtk_tree_view_get_selection(fileView.get());
    gtk_tree_selection_set_mode(gtk_tree_view_get_selection(fileView.get()), GTK_SELECTION_MULTIPLE);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("File Order"), GTK_SORT_ASCENDING);
    gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col(_("Filename"))), TRUE);
    gtk_tree_view_set_fixed_height_mode(fileView.get(), TRUE);
    fileView.setSortColumn_gui(_("Filename"), "File Order");
    fileView.setSortColumn_gui(_("Size"), "Size Order");
    fileView.setSortColumn_gui(_("Exact Size"), "Size Order");

    // Initialize the directory treeview
    dirView.setView(GTK_TREE_VIEW(getWidget("dirView")));
    dirView.insertColumn(_("Dir"), G_TYPE_STRING, TreeView::ICON_STRING, -1, "Icon");
    dirView.insertHiddenColumn("DL Dir", G_TYPE_POINTER);
    dirView.insertHiddenColumn("Icon", G_TYPE_STRING);
    dirView.finalize();
    dirStore = gtk_tree_store_newv(dirView.getColCount(), dirView.getGTypes());
    gtk_tree_view_set_model(dirView.get(), GTK_TREE_MODEL(dirStore));
    g_object_unref(dirStore);
    dirSelection = gtk_tree_view_get_selection(dirView.get());
    gtk_tree_view_set_enable_tree_lines(dirView.get(), TRUE);

    // Initialize the user command menus
    fileUserCommandMenu = new UserCommandMenu(getWidget("fileUserCommandMenu"), ::UserCommand::CONTEXT_FILELIST);
    addChild(fileUserCommandMenu);
    dirUserCommandMenu = new UserCommandMenu(getWidget("dirUserCommandMenu"), ::UserCommand::CONTEXT_FILELIST);
    addChild(dirUserCommandMenu);

    // Connect the signals to their callback functions.
    g_signal_connect(fileView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
    g_signal_connect(fileView.get(), "button-release-event", G_CALLBACK(onFileButtonReleased_gui), (gpointer)this);
    g_signal_connect(fileView.get(), "key-release-event", G_CALLBACK(onFileKeyReleased_gui), (gpointer)this);
    g_signal_connect(dirView.get(), "button-press-event", G_CALLBACK(onButtonPressed_gui), (gpointer)this);
    g_signal_connect(dirView.get(), "button-release-event", G_CALLBACK(onDirButtonReleased_gui), (gpointer)this);
    g_signal_connect(dirView.get(), "key-release-event", G_CALLBACK(onDirKeyReleased_gui), (gpointer)this);
    g_signal_connect(getWidget("matchButton"), "clicked", G_CALLBACK(onMatchButtonClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("findButton"), "clicked", G_CALLBACK(onFindButtonClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("nextButton"), "clicked", G_CALLBACK(onNextButtonClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("dirDownloadItem"), "activate", G_CALLBACK(onDownloadDirClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("fileDownloadItem"), "activate", G_CALLBACK(onDownloadClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("searchForAlternatesItem"), "activate", G_CALLBACK(onSearchAlternatesClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("copyMagnetItem"), "activate", G_CALLBACK(onCopyMagnetClicked_gui), (gpointer)this);
    g_signal_connect(getWidget("copyPictureItem"), "activate", G_CALLBACK(onCopyPictureClicked_gui), (gpointer)this);

    GError *error = NULL;
    g_thread_create(threadLoad_list, (gpointer)this, FALSE, &error);
    if (error) g_error_free(error);
}

ShareBrowser::~ShareBrowser()
{
    // Save the pane position
    int panePosition = gtk_paned_get_position(GTK_PANED(getWidget("pane")));
    WSET("sharebrowser-pane-position", panePosition);

    gtk_widget_destroy(getWidget("findDialog"));
    gtk_widget_destroy(getWidget("dirChooserDialog"));

    g_object_unref(getWidget("dirMenu"));
    g_object_unref(getWidget("fileMenu"));
}
bool ShareBrowser::isFull()
{
    return full;
}

void ShareBrowser::show()
{
    updateStatus_gui();
    WulforManager::get()->getMainWindow()->setMainStatus_gui(_("File list loaded"));
}

gpointer ShareBrowser::threadLoad_list(gpointer data)
{
    ShareBrowser *man = (ShareBrowser *)data;
    man->setStatus_gui("mainStatus", _("Parse and build tree..."));
    if (man->buildList_gui())
        man->setStatus_gui("mainStatus", _("Done"));
    return NULL;
}

bool ShareBrowser::buildList_gui()
{
    // Load the xml file containing the share list.
    try
    {
        // Set name of root entry to user nick.
        listing.getRoot()->setName(nick);

        if (full) {
            listing.loadFile(file);

            // Search ADL
            ADLSearchManager::getInstance()->matchListing(listing);
        }
        // Add entries to dir tree view starting with the root entry.
        buildDirs_gui(listing.getRoot(), NULL);

        if (full) {
            openDir_gui(initialDirectory);
        }
    }
    catch (const Exception &e)
    {
        setStatus_gui("mainStatus", _("Unable to load file list: ") + e.getError());
        return FALSE;
    }
    return TRUE;
}

/*
 * Selects the directory in the tree view, and shows that directory's contents.
 */
void ShareBrowser::openDir_gui(const string &dir)
{
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dirStore), &iter))
    {
        GtkTreePath *path;
        DirectoryListing::Directory *directory;

        if (findDir_gui(dir, &iter))
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(dirStore), &iter);
        else
            path = gtk_tree_path_new_first();

        directory = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");

        gtk_tree_view_expand_to_path(dirView.get(), path);
        gtk_tree_view_scroll_to_cell(dirView.get(), path, gtk_tree_view_get_column(dirView.get(), 0), FALSE, 0.0, 0.0);
        gtk_tree_view_set_cursor(dirView.get(), path, NULL, FALSE);
        gtk_tree_path_free(path);

        updateFiles_gui(directory);
    }
}

bool ShareBrowser::findDir_gui(const string &dir, GtkTreeIter *parent)
{
    if (dir.empty())
        return TRUE;

    string::size_type i = dir.find_first_of(PATH_SEPARATOR);
    const string &current = dir.substr(0, i);
    GtkTreeIter iter;
    bool valid = gtk_tree_model_iter_children(GTK_TREE_MODEL(dirStore), &iter, parent);

    while (valid)
    {
        if (dirView.getString(&iter, _("Dir")) == current)
        {
            *parent = iter;
            return findDir_gui(dir.substr(i + 1), parent);
        }

        valid = gtk_tree_model_iter_next(GTK_TREE_MODEL(dirStore), &iter);
    }

    return FALSE;
}

void ShareBrowser::buildDirs_gui(DirectoryListing::Directory *dir, GtkTreeIter *iter)
{
    DirectoryListing::Directory::Iter it;
    DirectoryListing::File::Iter file;
    GtkTreeIter newIter;

    gtk_tree_store_append(dirStore, &newIter, iter);
    gtk_tree_store_set(dirStore, &newIter, dirView.col(_("Dir")), dir->getName().c_str(), -1);

    gtk_tree_store_set(dirStore, &newIter,
        dirView.col("DL Dir"), (gpointer)dir,
        dirView.col("Icon"), "icon-directory",
        -1);

    for (file = dir->files.begin(); file != dir->files.end(); ++file)
    {
        shareItems++;
        shareSize += (*file)->getSize();
    }

    // Recursive call for all subdirs of current dir.
    std::sort(dir->directories.begin(), dir->directories.end(), DirectoryListing::Directory::DirSort());
    for (it = dir->directories.begin(); it != dir->directories.end(); ++it)
        buildDirs_gui(*it, &newIter);
}

void ShareBrowser::updateFiles_gui(DirectoryListing::Directory *dir)
{
    DirectoryListing::Directory::List *dirs = &(dir->directories);
    DirectoryListing::Directory::Iter it_dir;
    DirectoryListing::File::List *files = &(dir->files);
    DirectoryListing::File::Iter it_file;
    GtkTreeIter iter;
    int64_t size;
    gint sortColumn;
    GtkSortType sortType;

    currentSize = 0;
    currentItems = 0;

    gtk_list_store_clear(fileStore);

    gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(fileStore), &sortColumn, &sortType);
    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), GTK_TREE_SORTABLE_UNSORTED_SORT_COLUMN_ID, sortType);

    // Add directories to the store.
    for (it_dir = dirs->begin(); it_dir != dirs->end(); ++it_dir)
    {
        gtk_list_store_append(fileStore, &iter);
        gtk_list_store_set(fileStore, &iter,
            fileView.col(_("Filename")), Util::getFileName((*it_dir)->getName()).c_str(),
            fileView.col("File Order"), Util::getFileName("d"+(*it_dir)->getName()).c_str(),
            -1);

        size = (*it_dir)->getTotalSize(false);
        gtk_list_store_set(fileStore, &iter,
            fileView.col("Icon"), "icon-directory",
            fileView.col(_("Size")), Util::formatBytes(size).c_str(),
            fileView.col(_("Exact Size")), Util::formatExactSize(size).c_str(),
            fileView.col("Size Order"), size,
            fileView.col(_("Type")), _("Directory"),
            fileView.col("DL File"), (gpointer)(*it_dir),
            fileView.col(_("TTH")), "",
            -1);

        currentSize += size;
        currentItems++;
    }

    // Add files to the store.
    for (it_file = files->begin(); it_file != files->end(); ++it_file)
    {
        gtk_list_store_append(fileStore, &iter);

        // If ext is empty we cannot do substr on it.
        string ext = Util::getFileExt((*it_file)->getName());
        if (ext.length() > 0)
            ext = ext.substr(1);

        gtk_list_store_set(fileStore, &iter,
            fileView.col(_("Filename")), Util::getFileName((*it_file)->getName()).c_str(),
            fileView.col(_("Type")), ext.c_str(),
            fileView.col("File Order"), Util::getFileName("f"+(*it_file)->getName()).c_str(),
            -1);

        size = (*it_file)->getSize();
        gtk_list_store_set(fileStore, &iter,
            fileView.col("Icon"), "icon-file",
            fileView.col(_("Size")), Util::formatBytes(size).c_str(),
            fileView.col(_("Exact Size")), Util::formatExactSize(size).c_str(),
            fileView.col("Size Order"), size,
            fileView.col("DL File"), (gpointer)(*it_file),
            fileView.col(_("TTH")), (*it_file)->getTTH().toBase32().c_str(),
            fileView.col(_("Bitrate")), ((*it_file)->mediaInfo.bitrate) ? (Util::toString((*it_file)->mediaInfo.bitrate)).c_str() : Util::emptyString.c_str(),
            fileView.col(_("Resolution")), !(*it_file)->mediaInfo.video_info.empty() ? (*it_file)->mediaInfo.resolution.c_str() : Util::emptyString.c_str(),
            fileView.col(_("Video")), (*it_file)->mediaInfo.video_info.c_str(),
            fileView.col(_("Audio")), (*it_file)->mediaInfo.audio_info.c_str(),
            -1);

        currentSize += size;
        currentItems++;
    }

    gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), sortColumn, sortType);
    //gtk_tree_view_scroll_to_point(fileView.get(), 0, 0);
    updateStatus_gui();
    updateFileView = TRUE;
}

void ShareBrowser::updateStatus_gui()
{
    string items, files, size, total;
    files = _("Files: ") + Util::toString(shareItems);
    total = _("Total: ") + Util::formatBytes(shareSize);

    if (gtk_tree_selection_get_selected(dirSelection, NULL, NULL))
    {
        items = _("Items: ") + Util::toString(currentItems);
        size = _("Size: ") + Util::formatBytes(currentSize);
    }
    else
    {
        items = _("Items: 0");
        size = _("Size: 0 B");
    }

    setStatus_gui("itemsStatus", items);
    setStatus_gui("sizeStatus", size);
    setStatus_gui("filesStatus", files);
    setStatus_gui("totalStatus", total);
}

void ShareBrowser::setStatus_gui(string statusBar, string msg)
{
    gtk_statusbar_pop(GTK_STATUSBAR(getWidget(statusBar)), 0);
    gtk_statusbar_push(GTK_STATUSBAR(getWidget(statusBar)), 0, msg.c_str());
}

void ShareBrowser::fileViewSelected_gui()
{
    gpointer ptr;
    string fileOrder;
    GtkTreeIter iter, parentIter;
    GtkTreeModel *m = GTK_TREE_MODEL(fileStore);
    GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
    GtkTreePath *path = (GtkTreePath *)g_list_nth_data(list, 0);

    if (gtk_tree_model_get_iter(m, &iter, path))
    {
        ptr = fileView.getValue<gpointer>(&iter, "DL File");
        fileOrder = fileView.getString(&iter, "File Order");
        int64_t filesize = fileView.getValue<int64_t>(&iter, "Size Order");

        if (fileOrder[0] == 'd' && gtk_tree_selection_get_selected(dirSelection, NULL, &parentIter))
        {
            m = GTK_TREE_MODEL(dirStore);
            gboolean valid = gtk_tree_model_iter_children(m, &iter, &parentIter);

            while (valid && ptr != dirView.getValue<gpointer>(&iter, "DL Dir"))
                valid = gtk_tree_model_iter_next(m, &iter);

            if (!full && filesize == 0) {
                typedef Func1<ShareBrowser, DirectoryListing::Directory*> F1;
                F1 *func = new F1(this,&ShareBrowser::downloadChangedDir,
                (DirectoryListing::Directory *)dirView.getValue<gpointer>(&iter, "DL Dir"));
                WulforManager::get()->dispatchClientFunc(func);
                path = gtk_tree_model_get_path(m, &iter);
                gtk_tree_view_expand_to_path(dirView.get(), path);
                gtk_tree_view_set_cursor(dirView.get(), path, NULL, FALSE);
            } else {
                updateFiles_gui((DirectoryListing::Directory *)ptr);
            }
        }
        else
            downloadSelectedFiles_gui(Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)));
    }

    gtk_tree_path_free(path);
    g_list_free(list);
}

void ShareBrowser::downloadSelectedFiles_gui(const string &target)
{
    gpointer ptr;
    string fileOrder;
    string filename;
    GtkTreeIter iter;
    GtkTreePath *path;
    DirectoryListing::File *file;
    DirectoryListing::Directory *dir;
    GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fileStore), &iter, path))
        {
            ptr = fileView.getValue<gpointer>(&iter, "DL File");
            fileOrder = fileView.getString(&iter, "File Order");

            if (fileOrder[0] == 'd')
            {
                dir = (DirectoryListing::Directory *)ptr;

                typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
                F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
                WulforManager::get()->dispatchClientFunc(func);
            }
            else
            {
                file = (DirectoryListing::File *)ptr;

                string filename = Util::getFileName(file->getName());

                typedef Func2<ShareBrowser, DirectoryListing::File *, string> F2;
                F2 * func = new F2(this, &ShareBrowser::downloadFile_client, file, target + filename);
                WulforManager::get()->dispatchClientFunc(func);
            }
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void ShareBrowser::downloadSelectedDirs_gui(const string &target)
{
    DirectoryListing::Directory *dir;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
    {
        dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");

        typedef Func2<ShareBrowser, DirectoryListing::Directory *, string> F2;
        F2 * func = new F2(this, &ShareBrowser::downloadDir_client, dir, target);
        WulforManager::get()->dispatchClientFunc(func);
    }
}

void ShareBrowser::popupFileMenu_gui()
{
    GtkWidget *menuItem;

    // Clean menus
    gtk_container_foreach(GTK_CONTAINER(getWidget("fileDownloadMenu")), (GtkCallback)gtk_widget_destroy, NULL);
    fileUserCommandMenu->cleanMenu_gui();

    // Build file download menu
    StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
    if (!spl.empty())
    {
        for (StringPairIter i = spl.begin(); i != spl.end(); ++i)
        {
            menuItem = gtk_menu_item_new_with_label(i->second.c_str());
            g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
            g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteClicked_gui), (gpointer)this);
            gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("fileDownloadMenu")), menuItem);
        }
        menuItem = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("fileDownloadMenu")), menuItem);
    }

    menuItem = gtk_menu_item_new_with_label(_("Browse..."));
    g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadToClicked_gui), (gpointer)this);
    gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("fileDownloadMenu")), menuItem);

    // Build user command menu
    StringList hubs = WulforUtil::getHubAddress(listing.getUser().user->getCID(), "");
    fileUserCommandMenu->addHub(hubs);
    GtkTreeIter iter;
    GList *list = gtk_tree_selection_get_selected_rows(fileSelection, NULL);
    string cid = listing.getUser().user->getCID().toBase32();

    for (GList *i = list; i; i = i->next)
    {
        GtkTreePath *path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(fileStore), &iter, path))
        {
            string filepath;
            string fileOrder = fileView.getString(&iter, "File Order");
            gpointer ptr = fileView.getValue<gpointer>(&iter, "DL File");

            if (fileOrder[0] == 'd')
            {
                DirectoryListing::Directory *dir = (DirectoryListing::Directory *)ptr;
                filepath = listing.getPath(dir->getParent());
            }
            else
            {
                DirectoryListing::File *file = (DirectoryListing::File *)ptr;
                filepath = listing.getPath(file);
            }

            fileUserCommandMenu->addFile(cid,
                fileView.getString(&iter, _("Filename")),
                filepath,
                fileView.getValue<int64_t>(&iter, "Size Order"),
                fileView.getString(&iter, _("TTH")));
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
    fileUserCommandMenu->buildMenu_gui();

    gtk_menu_popup(GTK_MENU(getWidget("fileMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
    gtk_widget_show_all(getWidget("fileMenu"));
}

void ShareBrowser::popupDirMenu_gui()
{
    GtkWidget *menuItem;

    // Clean menus
    gtk_container_foreach(GTK_CONTAINER(getWidget("dirDownloadMenu")), (GtkCallback)gtk_widget_destroy, NULL);
    dirUserCommandMenu->cleanMenu_gui();

    StringPairList spl = FavoriteManager::getInstance()->getFavoriteDirs();
    if (!spl.empty())
    {
        for (StringPairIter i = spl.begin(); i != spl.end(); ++i)
        {
            menuItem = gtk_menu_item_new_with_label(i->second.c_str());
            g_object_set_data_full(G_OBJECT(menuItem), "fav", g_strdup(i->first.c_str()), g_free);
            g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadFavoriteDirClicked_gui), (gpointer)this);
            gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("dirDownloadMenu")), menuItem);
        }
        menuItem = gtk_separator_menu_item_new();
        gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("dirDownloadMenu")), menuItem);
    }

    menuItem = gtk_menu_item_new_with_label(_("Browse..."));
    g_signal_connect(menuItem, "activate", G_CALLBACK(onDownloadDirToClicked_gui), (gpointer)this);
    gtk_menu_shell_append(GTK_MENU_SHELL(getWidget("dirDownloadMenu")), menuItem);

    // Add user commands.
    GtkTreeIter iter;
    if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
    {
        string filename;
        string filepath;
        string cid = listing.getUser().user->getCID().toBase32();
        StringList hubs = WulforUtil::getHubAddress(listing.getUser().user->getCID(), "");
        DirectoryListing::Directory *dir = dirView.getValue<DirectoryListing::Directory *>(&iter, "DL Dir");

        if (dir != listing.getRoot())
        {
            filename = dirView.getString(&iter, _("Dir"));
            filepath = listing.getPath(dir->getParent());
        }
        dirUserCommandMenu->addFile(cid, filename, filepath);
        dirUserCommandMenu->addHub(hubs);
        dirUserCommandMenu->buildMenu_gui();
    }

    gtk_menu_popup(GTK_MENU(getWidget("dirMenu")), NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time());
    gtk_widget_show_all(getWidget("dirMenu"));
}

/*
 * Searches the directories iteratively for the requested pattern. Uses a pre-order
 * traversal method, with the exception that it searches the parent's dir name first.
 * Instead of keeping track of the last directory its search ended at, it counts
 * the number of matches and re-searches the listing, skipping matches until it
 * reaches the newest one. Slightly slower, but simpler.
 */
void ShareBrowser::find_gui()
{
    string name;
    bool findLeafNode = TRUE;
    int cursorPos, hits = 0;
    DirectoryListing::Directory *dir;
    DirectoryListing::File::Iter file;
    GtkTreeIter iter;
    GtkTreeModel *m = GTK_TREE_MODEL(dirStore);
    GtkTreePath *dirPath = gtk_tree_path_new_first();

    if (gtk_tree_path_get_depth(dirPath) == 0 || !gtk_tree_model_get_iter(m, &iter, dirPath))
    {
        gtk_tree_path_free(dirPath);
        return;
    }

    gint sortColumn;
    GtkSortType sortType;
    gtk_tree_sortable_get_sort_column_id(GTK_TREE_SORTABLE(fileStore), &sortColumn, &sortType);
    if (sortColumn != fileView.col("File Order") || sortType != GTK_SORT_ASCENDING)
    {
        gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(fileStore), fileView.col("File Order"), GTK_SORT_ASCENDING);
        gtk_tree_view_column_set_sort_indicator(gtk_tree_view_get_column(fileView.get(), fileView.col(_("Filename"))), TRUE);
    }

    while (TRUE)
    {
        // Drill down until we reach a leaf node (e.g. a dir with no child dirs).
        if (findLeafNode)
        {
            do
            {
                name = Text::toLower(dirView.getString(&iter, _("Dir")));
                // We found a matching directory name.
                if (name.find(search, 0) != string::npos && hits++ == skipHits)
                {
                    skipHits = hits;
                    gtk_tree_view_expand_to_path(dirView.get(), dirPath);
                    gtk_tree_view_set_cursor(dirView.get(), dirPath, NULL, FALSE);
                    dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");
                    updateFiles_gui(dir);
                    gtk_widget_grab_focus(GTK_WIDGET(dirView.get()));
                    updateFileView = FALSE;
                    gtk_tree_path_free(dirPath);
                    setStatus_gui("mainStatus", _("Found a match"));
                    return;
                }
                gtk_tree_path_down(dirPath);
            }
            while (gtk_tree_model_get_iter(m, &iter, dirPath));
        }

        // Come back up one directory. If we can't, then we've returned to the root and are done.
        if (!gtk_tree_path_up(dirPath) || gtk_tree_path_get_depth(dirPath) == 0 ||
            !gtk_tree_model_get_iter(m, &iter, dirPath))
        {
            setStatus_gui("mainStatus", _("No matches"));
            gtk_tree_path_free(dirPath);
            return;
        }

        // Search the files that are contained in this directory.
        dir = dirView.getValue<gpointer, DirectoryListing::Directory *>(&iter, "DL Dir");
        std::sort(dir->files.begin(), dir->files.end(), DirectoryListing::File::FileSort());

        for (file = dir->files.begin(), cursorPos = dir->directories.size(); file != dir->files.end(); ++file, ++cursorPos)
        {
            name = Text::toLower((*file)->getName());

            // We found a matching file. Update the cursors and the fileView if necessary.
            if (name.find(search, 0) != string::npos && hits++ == skipHits)
            {
                if (updateFileView)
                {
                    gtk_tree_view_expand_to_path(dirView.get(), dirPath);
                    gtk_tree_view_set_cursor(dirView.get(), dirPath, NULL, FALSE);
                    updateFiles_gui(dir);
                    updateFileView = FALSE;
                }

                skipHits = hits;
                // Keeping track of the current index allows us to quickly get the path to the file.
                GtkTreePath *path = gtk_tree_path_new_from_string(Util::toString(cursorPos).c_str());
                gtk_tree_view_set_cursor(fileView.get(), path, NULL, FALSE);
                gtk_widget_grab_focus(GTK_WIDGET(fileView.get()));
                gtk_tree_path_free(path);
                gtk_tree_path_free(dirPath);
                setStatus_gui("mainStatus", _("Found a match"));
                return;
            }
        }
        updateFileView = TRUE;

        // Determine if we are to go to the next sibling or back to the parent dir.
        gtk_tree_path_next(dirPath);
        if (!gtk_tree_model_get_iter(m, &iter, dirPath))
            findLeafNode = FALSE;
        else
            findLeafNode = TRUE;
    }
}

gboolean ShareBrowser::onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    sb->oldType = event->type;

    if (event->button == 3)
    {
        GtkTreePath *path;

        if (gtk_tree_view_get_path_at_pos(sb->fileView.get(), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL))
        {
            bool selected = gtk_tree_selection_path_is_selected(sb->fileSelection, path);
            gtk_tree_path_free(path);

            if (selected)
                return TRUE;
        }
    }

    return FALSE;
}

gboolean ShareBrowser::onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

    if (count > 0 && event->type == GDK_BUTTON_RELEASE && event->button == 3)
        sb->popupFileMenu_gui();
    else if (count == 1 && sb->oldType == GDK_2BUTTON_PRESS && event->button == 1)
        sb->fileViewSelected_gui();

    return FALSE;
}

gboolean ShareBrowser::onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    gint count = gtk_tree_selection_count_selected_rows(sb->fileSelection);

    if (count > 0 && (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK)))
        sb->popupFileMenu_gui();
    else if (count == 1 && (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter))
        sb->fileViewSelected_gui();

    return FALSE;
}

gboolean ShareBrowser::onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    GtkTreeIter iter;
    gpointer ptr;

    if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
        return FALSE;

    if (!sb->isFull()) sb->viewPartial_gui();

    if (event->button == 1 && sb->oldType == GDK_2BUTTON_PRESS)
    {
        GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
        if (gtk_tree_view_row_expanded(sb->dirView.get(), path))
            gtk_tree_view_collapse_row(sb->dirView.get(), path);
        else
            gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
        gtk_tree_path_free(path);
    }
    else if (event->button == 1 && event->type == GDK_BUTTON_RELEASE)
    {
        ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
        sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
    }
    else if (event->button == 3 && event->type == GDK_BUTTON_RELEASE)
    {
        ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
        sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
        sb->popupDirMenu_gui();
    }

    return FALSE;
}

gboolean ShareBrowser::onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    GtkTreeIter iter;
    gpointer ptr;

    if (!gtk_tree_selection_get_selected(sb->dirSelection, NULL, &iter))
        return FALSE;

    if (!sb->isFull()) sb->viewPartial_gui();

    if (event->keyval == GDK_Return || event->keyval == GDK_KP_Enter || event->keyval == GDK_Right || event->keyval == GDK_Left)
    {
        GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(sb->dirStore), &iter);
        if (gtk_tree_view_row_expanded(sb->dirView.get(), path))
            gtk_tree_view_collapse_row(sb->dirView.get(), path);
        else
            gtk_tree_view_expand_row(sb->dirView.get(), path, FALSE);
        gtk_tree_path_free(path);
    }
    else if (event->keyval == GDK_Up || event->keyval == GDK_KP_Up ||
        event->keyval == GDK_Down || event->keyval == GDK_KP_Down)
    {
        ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
        sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
    }
    else if (event->keyval == GDK_Menu || (event->keyval == GDK_F10 && event->state & GDK_SHIFT_MASK))
    {
        ptr = sb->dirView.getValue<gpointer>(&iter, "DL Dir");
        sb->updateFiles_gui((DirectoryListing::Directory *)ptr);
        sb->popupDirMenu_gui();
    }

    return FALSE;
}

void ShareBrowser::onMatchButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    typedef Func0<ShareBrowser> F0;
    F0 *f0 = new F0((ShareBrowser*)data, &ShareBrowser::matchQueue_client);
    WulforManager::get()->dispatchClientFunc(f0);
}

void ShareBrowser::onFindButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;

    gtk_widget_grab_focus(GTK_WIDGET(sb->getWidget("findEntry")));
    gint response = gtk_dialog_run(GTK_DIALOG(sb->getWidget("findDialog")));

    // Fix crash, if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(sb->getWidget("findDialog"));

    if (response == GTK_RESPONSE_OK)
    {
        string text = gtk_entry_get_text(GTK_ENTRY(sb->getWidget("findEntry")));
        if (!text.empty())
        {
            sb->search = text;
            sb->skipHits = 0;
            sb->find_gui();
        }
        else
        {
            sb->setStatus_gui("mainStatus", _("No matches"));
        }
    }
}

void ShareBrowser::onNextButtonClicked_gui(GtkWidget *widget, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    if (!sb->search.empty())
        sb->find_gui();
    else
        sb->setStatus_gui("mainStatus", _("No search text entered"));
}

void ShareBrowser::onDownloadClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    sb->downloadSelectedFiles_gui(Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)));
}

void ShareBrowser::onDownloadToClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;

    gint response = gtk_dialog_run(GTK_DIALOG(sb->getWidget("dirChooserDialog")));

    // Fix crash, if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(sb->getWidget("dirChooserDialog"));

    if (response == GTK_RESPONSE_OK)
    {
        gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(sb->getWidget("dirChooserDialog")));
        if (temp)
        {
            string path = Text::toUtf8(temp);
            g_free(temp);
            if (path[path.length() - 1] != PATH_SEPARATOR)
                path += PATH_SEPARATOR;

            sb->downloadSelectedFiles_gui(path);
        }
    }
}

void ShareBrowser::onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    string target = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
    sb->downloadSelectedFiles_gui(target);
}

void ShareBrowser::onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    sb->downloadSelectedDirs_gui(Text::fromUtf8(SETTING(DOWNLOAD_DIRECTORY)));
}

void ShareBrowser::onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;

    gint response = gtk_dialog_run(GTK_DIALOG(sb->getWidget("dirChooserDialog")));

    // Fix crash, if the dialog gets programmatically destroyed.
    if (response == GTK_RESPONSE_NONE)
        return;

    gtk_widget_hide(sb->getWidget("dirChooserDialog"));

    if (response == GTK_RESPONSE_OK)
    {
        gchar *temp = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(sb->getWidget("dirChooserDialog")));
        if (temp)
        {
            string path = Text::toUtf8(temp);
            g_free(temp);
            if (path[path.length() - 1] != PATH_SEPARATOR)
                path += PATH_SEPARATOR;

            sb->downloadSelectedDirs_gui(path);
        }
    }
}

void ShareBrowser::onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    string target = string((gchar *)g_object_get_data(G_OBJECT(item), "fav"));
    sb->downloadSelectedDirs_gui(target);
}

void ShareBrowser::onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    GtkTreeIter iter;
    GtkTreePath *path;
    string fileOrder;
    Search *s;
    DirectoryListing::File *file;
    GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
        {
            fileOrder = sb->fileView.getString(&iter, "File Order");

            if (fileOrder[0] == 'f')
            {
                file = sb->fileView.getValue<gpointer, DirectoryListing::File *>(&iter, "DL File");
                s = WulforManager::get()->getMainWindow()->addSearch_gui();
                s->putValue_gui(file->getTTH().toBase32(), 0, SearchManager::SIZE_DONTCARE, SearchManager::TYPE_TTH);
            }
        }
        gtk_tree_path_free(path);
    }
    g_list_free(list);
}

void ShareBrowser::onCopyMagnetClicked_gui(GtkMenuItem* item, gpointer data)
{
    ShareBrowser *sb = (ShareBrowser *)data;
    GtkTreeIter iter;
    GtkTreePath *path;
    int64_t size;
    string magnets, magnet, filename, tth;
    GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);

    for (GList *i = list; i; i = i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
        {
            filename = sb->fileView.getString(&iter, _("Filename"));
            size = sb->fileView.getValue<int64_t>(&iter, "Size Order");
            tth = sb->fileView.getString(&iter, _("TTH"));
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

void ShareBrowser::onCopyPictureClicked_gui(GtkMenuItem* item, gpointer data)
{
   ShareBrowser *sb = (ShareBrowser *)data;
   GtkTreeIter iter;
   GtkTreePath *path;
   int64_t size;
   string magnets, magnet, filename, tth;
   GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);

   for (GList *i = list; i; i = i->next)
   {
       path = (GtkTreePath *)i->data;
       if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
       {
           filename = sb->fileView.getString(&iter, _("Filename"));
           size = sb->fileView.getValue<int64_t>(&iter, "Size Order");
           tth = sb->fileView.getString(&iter, _("TTH"));
           magnet = WulforUtil::makeMagnet(filename, size, tth);

           if (!magnet.empty())
           {
               magnet = "[img]" + magnet + "[/img]";
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

void ShareBrowser::downloadFile_client(DirectoryListing::File *file, string target)
{
    try
    {
        listing.download(file, target, FALSE, FALSE);
    }
    catch (const Exception& e)
    {
        typedef Func2<ShareBrowser, string, string> F2;
        F2 *func = new F2(this, &ShareBrowser::setStatus_gui, "mainStatus", e.getError());
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void ShareBrowser::downloadDir_client(DirectoryListing::Directory *dir, string target)
{
    try
    {
        listing.download(dir, target, FALSE);
    }
    catch (const Exception& e)
    {
        typedef Func2<ShareBrowser, string, string> F2;
        F2 *func = new F2(this, &ShareBrowser::setStatus_gui, "mainStatus", e.getError());
        WulforManager::get()->dispatchGuiFunc(func);
    }
}

void ShareBrowser::matchQueue_client()
{
    int matched = QueueManager::getInstance()->matchListing(listing);
    string message = _("Matched ") + Util::toString(matched) + _(" files");

    typedef Func2<ShareBrowser, string, string> F2;
    F2 *f = new F2(this, &ShareBrowser::setStatus_gui, "mainStatus", message);
    WulforManager::get()->dispatchGuiFunc(f);
}
void ShareBrowser::onDirGet(GtkMenuItem* item, gpointer data)
{
    ShareBrowser *sb=(ShareBrowser*)data;
    GList *list = gtk_tree_selection_get_selected_rows(sb->fileSelection, NULL);
    GtkTreePath *path;
    GtkTreeIter iter;
    string name,fullpath;
    DirectoryListing::File *filed;

    for (GList *i = list;i;i=i->next)
    {
        path = (GtkTreePath *)i->data;
        if (gtk_tree_model_get_iter(GTK_TREE_MODEL(sb->fileStore), &iter, path))
        {
            filed = sb->fileView.getValue<gpointer, DirectoryListing::File *>(&iter, "DL File");

            ItemInfo* ii= new ItemInfo(filed);
            if(ii->type == ItemInfo::FILE)
            {
                if(!ii->file->getAdls())return;
                DirectoryListing::Directory *dir=ii->file->getParent();
                while( (dir!=NULL) && (dir!=sb->listing.getRoot()))
                {
                    fullpath=dir->getName()+PATH_SEPARATOR+fullpath;
                    dir=dir->getParent();
                }
            }
            else if(ii->type == ItemInfo::DIRECTORY)
            {
                if(!(ii->dir->getAdls()) && (ii->dir->getParent() != sb->listing.getRoot()))
                    return;
                fullpath = Text::toT(((DirectoryListing::AdlDirectory*)ii->dir)->getFullPath());
            }
        }
    }
    sb->openDir_gui(fullpath);
}

int ShareBrowser::ItemInfo::compareItems(ItemInfo* a, ItemInfo* b, int col) {
    if(a->type == DIRECTORY) {
        if(b->type == DIRECTORY) {
            switch(col) {
                case COLUMN_EXACTSIZE: return compare(a->dir->getTotalSize(), b->dir->getTotalSize());
                case COLUMN_SIZE: return compare(a->dir->getTotalSize(), b->dir->getTotalSize());
                default: break; //strcmp(a->columns[col].c_str(), b->columns[col].c_str());
            }
        } else {
            return -1;
        }
    } else if(b->type == DIRECTORY) {
        return 1;
    } else {
        switch(col) {
            case COLUMN_EXACTSIZE: return compare(a->file->getSize(), b->file->getSize());
            case COLUMN_SIZE: return compare(a->file->getSize(), b->file->getSize());
            default: break;// strcmp(a->columns[col].c_str(), b->columns[col].c_str());
        }
    }

    return -1;
}

void ShareBrowser::loadXML(string txt)
{
    typedef Func1<ShareBrowser,string> F1;
    F1 *func = new F1(this,&ShareBrowser::load,txt);
    WulforManager::get()->dispatchGuiFunc(func);
}

void ShareBrowser::load(string xml)
{
    // Set name of root entry to user nick.
    listing.getRoot()->setName(nick);

    GtkTreeIter iter;
    DirectoryListing::Directory *dirList;
    string path,path2;
    GtkTreePath *treepath;
    if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
    {
        dirList = (DirectoryListing::Directory *)dirView.getValue<gpointer>(&iter,"DL Dir");
        path2 = dirList->getName();
        treepath = gtk_tree_path_copy(gtk_tree_model_get_path (GTK_TREE_MODEL(dirStore), gtk_tree_iter_copy(&iter)));

        //path = QueueManager::getInstance()->getListPath(listing.getUser()) + ".xml";
        path = Util::getListPath() + nick + user->getCID().toBase32() + ".xml.bz2";
        if(File::getSize(path) != -1) {
            // load the cached list.
            listing.updateXML(File(path, File::READ, File::OPEN).read());
        }
        auto base = listing.updateXML(xml);
        //listing.save(path);
        ADLSearchManager::getInstance()->matchListing(listing);
        gtk_tree_store_clear(dirStore);
        gtk_list_store_clear(fileStore);
        buildDirs_gui(listing.getRoot(),NULL);
        gtk_tree_view_expand_to_path(dirView.get(), treepath);
        gtk_tree_view_scroll_to_cell(dirView.get(),treepath,NULL,FALSE,0,0);
        gtk_tree_selection_select_path(dirSelection,treepath);
        updateFiles_gui(dirList);
    }
}

void ShareBrowser::viewPartial_gui()
{
    GtkTreeIter iter;
    DirectoryListing::Directory *dirList;
    if (gtk_tree_selection_get_selected(dirSelection, NULL, &iter))
    {
        dirList = (DirectoryListing::Directory *)dirView.getValue<gpointer>(&iter,"DL Dir");
    }
    typedef Func1<ShareBrowser, DirectoryListing::Directory*> F1;
    F1 *func = new F1(this,&ShareBrowser::downloadChangedDir,dirList);
    WulforManager::get()->dispatchClientFunc(func);
}


void ShareBrowser::downloadChangedDir(DirectoryListing::Directory* d) {
    if(!d->getComplete()) {
        dcdebug("Directory %s incomplete, downloading...\n", d->getName().c_str());
        if(listing.getUser().user->isOnline()) {
            try {
                QueueManager::getInstance()->addList(listing.getUser(), QueueItem::FLAG_PARTIAL_LIST, listing.getPath(d));
            } catch(const QueueException& e) { }
        } else {
            setStatus_gui("mainStatus","User went offline");
        }
    }
}
/*Many code from orginal DC++*/
