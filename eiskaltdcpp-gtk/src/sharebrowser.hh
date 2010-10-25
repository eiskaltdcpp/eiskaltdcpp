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

#ifndef WULFOR_SHARE_BROWSER_HH
#define WULFOR_SHARE_BROWSER_HH

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/DirectoryListing.h>

#include "bookentry.hh"
#include "treeview.hh"

class UserCommandMenu;

class ShareBrowser:
    public BookEntry
{
    public:
        ShareBrowser(dcpp::UserPtr user, const std::string &file, const std::string &initialDirectory);
        virtual ~ShareBrowser();
        virtual void show();

    private:
        // GUI functions
        void buildList_gui();
        void openDir_gui(const std::string &dir);
        bool findDir_gui(const std::string &dir, GtkTreeIter *parent);
        void buildDirs_gui(dcpp::DirectoryListing::Directory *dir, GtkTreeIter *iter);
        void updateFiles_gui(dcpp::DirectoryListing::Directory *dir);
        void updateStatus_gui();
        void setStatus_gui(std::string statusBar, std::string msg);
        void fileViewSelected_gui();
        void downloadSelectedFiles_gui(const std::string &target);
        void downloadSelectedDirs_gui(const std::string &target);
        void popupFileMenu_gui();
        void popupDirMenu_gui();
        void find_gui();

        // GUI callbacks
        static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static gboolean onFileButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static gboolean onFileKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
        static gboolean onDirButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static gboolean onDirKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
        static void onMatchButtonClicked_gui(GtkWidget *widget, gpointer data);
        static void onFindButtonClicked_gui(GtkWidget *widget, gpointer);
        static void onNextButtonClicked_gui(GtkWidget *widget, gpointer);
        static void onDownloadClicked_gui(GtkMenuItem *item, gpointer data);
        static void onDownloadToClicked_gui(GtkMenuItem *item, gpointer data);
        static void onDownloadFavoriteClicked_gui(GtkMenuItem *item, gpointer data);
        static void onDownloadDirClicked_gui(GtkMenuItem *item, gpointer data);
        static void onDownloadDirToClicked_gui(GtkMenuItem *item, gpointer data);
        static void onDownloadFavoriteDirClicked_gui(GtkMenuItem *item, gpointer data);
        static void onSearchAlternatesClicked_gui(GtkMenuItem *item, gpointer data);
        static void onCopyMagnetClicked_gui(GtkMenuItem* item, gpointer data);
        static void onCopyPictureClicked_gui(GtkMenuItem* item, gpointer data);
        static void onDirGet(GtkMenuItem* item, gpointer data);
        // Client functions
        void downloadFile_client(dcpp::DirectoryListing::File *file, std::string target);
        void downloadDir_client(dcpp::DirectoryListing::Directory *dir, std::string target);
        void matchQueue_client();

        GdkEventType oldType;
        dcpp::UserPtr user;
        std::string file;
        std::string initialDirectory;
        std::string nick;
        dcpp::DirectoryListing listing;
        int64_t shareSize;
        int64_t currentSize;
        int shareItems;
        int currentItems;
        std::string search;
        bool updateFileView;
        int skipHits;
        TreeView dirView, fileView;
        GtkListStore *fileStore;
        GtkTreeStore *dirStore;
        GtkTreeSelection *fileSelection, *dirSelection;
        UserCommandMenu *fileUserCommandMenu;
        UserCommandMenu *dirUserCommandMenu;

                enum {
                COLUMN_FILENAME,
                COLUMN_TYPE,
                COLUMN_SIZE,
                COLUMN_EXACTSIZE,
                COLUMN_TTH,
                COLUMN_LAST
                };

        class ItemInfo : public dcpp::FastAlloc<ItemInfo> {
        public:
                enum ItemType {
                        FILE,
                        DIRECTORY,
                        USER
                } type;

                union {
                        dcpp::DirectoryListing::File* file;
                        dcpp::DirectoryListing::Directory* dir;
                };

                ItemInfo(const std::string& nick, dcpp::DirectoryListing::Directory* d) : type(USER), dir(d) {
                        columns.insert(ParamMap::value_type("FILENAME",nick));
                }

                ItemInfo(dcpp::DirectoryListing::File* f) : type(FILE), file(f) {
                        columns.insert(ParamMap::value_type("FILENAME",f->getName()));
                        //columns["COLUMN_FILENAME"] = dcpp::Text::toT(f->getName());
                        columns.insert(ParamMap::value_type("TYPE",dcpp::Util::getFileExt(columns["FILENAME"])));
                        //columns["COLUMN_TYPE"] = dcpp::Util::getFileExt(columns["COLUMN_FILENAME"]);
                        //if(columns["COLUMN_TYPE"].size() > 0 && columns["COLUMN_TYPE"][0] == '.')
                        //      columns["COLUMN_TYPE"].erase(0, 1);

                        columns["EXACTSIZE"] = dcpp::Text::toT(dcpp::Util::formatExactSize(f->getSize()));
                        columns["SIZE"] = dcpp::Text::toT(dcpp::Util::formatBytes(f->getSize()));
                        columns["TTH"] = dcpp::Text::toT(f->getTTH().toBase32());
                }
                ItemInfo(dcpp::DirectoryListing::Directory* d) : type(DIRECTORY), dir(d) {
                        columns.insert(ParamMap::value_type("FILENAME",d->getName()));
                        columns.insert(ParamMap::value_type("EXACTSIZE",d->getComplete() ? dcpp::Util::formatExactSize(d->getTotalSize()) : _("?")));
                        //columns["COLUMN_FILENAME"] = dcpp::Text::toT(d->getName());
                        //columns["COLUMN_EXACTSIZE"] = d->getComplete() ? dcpp::Text::toT(dcpp::Util::formatExactSize(d->getTotalSize())) : _("?");
                        columns.insert(ParamMap::value_type("SIZE",d->getComplete() ? dcpp::Util::formatBytes(d->getTotalSize()) : _("?")));
                        //columns["COLUMN_SIZE"] = d->getComplete() ? dcpp::Text::toT(dcpp::Util::formatBytes(d->getTotalSize())) : _("?");
                }

                //const char *getText() const {
                //      return columns["COLUMN_FILENAME"].c_str();
                //}

                int getImage()  const {return 0;};

                int getSelectedImage() const {
                        return getImage();
                }

                /*const std::string& getText(int col) const {
                        return columns[col];
                }*/

                struct TotalSize {
                        TotalSize() : total(0) { }
                        void operator()(ItemInfo* a) { total += a->type == DIRECTORY ? a->dir->getTotalSize() : a->file->getSize(); }
                        int64_t total;
                };

                static int compareItems(ItemInfo* a, ItemInfo* b, int col);
                private:
                //std::string columns[COLUMN_LAST];
                typedef std::map<std::string, std::string> ParamMap;
                ParamMap columns;
                };
};

#else
class ShareBrowser;
#endif
