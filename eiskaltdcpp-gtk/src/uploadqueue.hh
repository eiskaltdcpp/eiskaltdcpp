//      uploadqueue.hh
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

#pragma once

#include <dcpp/stdinc.h>
#include <dcpp/UploadManager.h>
#include "bookentry.hh"
#include "treeview.hh"

class UploadQueue:
        public BookEntry,
        private dcpp::UploadManagerListener
{
    public:
        UploadQueue();
        virtual ~UploadQueue();
        virtual void show();

    private:
        typedef std::map<std::string,GtkTreeIter> MapUsers;

        void getParams(const std::string file, dcpp::UserPtr user, dcpp::StringMap &params);
        void addFile(dcpp::StringMap &params, GtkTreeIter *iter);
        void AddFile_gui(dcpp::StringMap params);
        void removeUser(std::string cid);

        static void onGrantSlotItemClicked_gui(GtkMenuItem *item, gpointer data);
        static void onRemoveItem_gui(GtkMenuItem *item, gpointer data);
        static void onSendPMItemClicked_gui(GtkMenuItem *item, gpointer data);
        static void onBrowseItemClicked_gui(GtkMenuItem *item, gpointer data);
        static void onFavoriteUserAddItemClicked_gui(GtkMenuItem *item, gpointer data);
        static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
        static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
        static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);

        //client funcs
        void grantSlot_client(const std::string cid);
        void getFileList_client(const std::string cid);
        void removeUploadFromQueue(const std::string cid);
        void addFavoriteUser_client(const std::string cid);
        void init();

        virtual void on(dcpp::UploadManagerListener::WaitingAddFile, const dcpp::HintedUser& hUser, std::string file) noexcept;
        virtual void on(dcpp::UploadManagerListener::WaitingRemoveUser, const dcpp::HintedUser& user) noexcept;

        TreeView users;
        GtkListStore *store;
        MapUsers mapUsers;
        GtkTreeSelection *selection;
        GdkEventType previous;


};
