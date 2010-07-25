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

#ifndef WULFOR_FINISHED_TRANSFERS
#define WULFOR_FINISHED_TRANSFERS

#include <dcpp/stdinc.h>
#include <dcpp/DCPlusPlus.h>
#include <dcpp/FinishedManager.h>
#include <dcpp/FinishedItem.h>
#include "bookentry.hh"
#include "treeview.hh"

class PreviewMenu;

class FinishedTransfers:
	public BookEntry,
	public dcpp::FinishedManagerListener
{
	public:
		static FinishedTransfers* createFinishedUploads();
		static FinishedTransfers* createFinishedDownloads();
		virtual ~FinishedTransfers();
		virtual void show();

	private:
		FinishedTransfers(const EntryType type, const std::string &title, bool isUpload);

		// GUI functions
		void addFile_gui(dcpp::StringMap params, bool update);
		void addUser_gui(dcpp::StringMap params, bool update);
		void removeFile_gui(std::string target);
		void removeUser_gui(std::string cid);
		void updateStatus_gui();
		bool findFile_gui(GtkTreeIter* iter, const std::string& item);
		bool findUser_gui(GtkTreeIter* iter, const std::string& cid);

		// GUI callbacks
		static gboolean onButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onKeyReleased_gui(GtkWidget *widget, GdkEventKey *event, gpointer data);
		static void onOpen_gui(GtkMenuItem *item, gpointer data);
		static void onOpenFolder_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveItems_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveAll_gui(GtkMenuItem *item, gpointer data);
		static void onPageSwitched_gui(GtkNotebook *notebook, GtkNotebookPage *page, guint num, gpointer data);

		// Client functions
		void initializeList_client();
		void getFinishedParams_client(const dcpp::FinishedFileItemPtr &item, const std::string &file,  dcpp::StringMap &params);
		void getFinishedParams_client(const dcpp::FinishedUserItemPtr &item, const dcpp::UserPtr &user,  dcpp::StringMap &params);
		void removeFile_client(std::string target);
		void removeUser_client(std::string cid);
		void removeAll_client();

		// Client callbacks
		virtual void on(dcpp::FinishedManagerListener::AddedFile, bool upload, const std::string &file, const dcpp::FinishedFileItemPtr &item) throw();
		virtual void on(dcpp::FinishedManagerListener::AddedUser, bool upload, const dcpp::UserPtr &user, const dcpp::FinishedUserItemPtr &item) throw();
		virtual void on(dcpp::FinishedManagerListener::UpdatedFile, bool upload, const std::string &file, const dcpp::FinishedFileItemPtr &item) throw();
		virtual void on(dcpp::FinishedManagerListener::RemovedFile, bool upload, const std::string &file) throw();
		virtual void on(dcpp::FinishedManagerListener::UpdatedUser, bool upload, const dcpp::UserPtr &user) throw();
		virtual void on(dcpp::FinishedManagerListener::RemovedUser, bool upload, const dcpp::UserPtr &user) throw();
		/* virtual void on(dcpp::FinishedManagerListener::RemoveAll, bool upload) throw();  Implement? */

		GtkListStore *fileStore, *userStore;
		TreeView userView;
		TreeView fileView;
		GtkTreeSelection *fileSelection,*userSelection;
		bool isUpload;
		int totalFiles;
		int totalUsers;
		int64_t totalBytes, totalTime;
		PreviewMenu *appsPreviewMenu;
};

#else
class FinishedTransfers;
#endif
