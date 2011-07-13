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

#ifndef WULFOR_TRANSFERS_HH
#define WULFOR_TRANSFERS_HH

#include <dcpp/stdinc.h>
#include <dcpp/ConnectionManager.h>
#include <dcpp/DownloadManager.h>
#include <dcpp/LogManager.h>
#include <dcpp/QueueManager.h>
#include <dcpp/UploadManager.h>

#include "treeview.hh"
#include "entry.hh"
#include "sound.hh"

class PreviewMenu;
class UserCommandMenu;

class Transfers:
	public dcpp::ConnectionManagerListener,
	public dcpp::DownloadManagerListener,
	public dcpp::QueueManagerListener,
	public dcpp::UploadManagerListener,
	public Entry
{
	public:
		Transfers();
		~Transfers();

		GtkWidget *getContainer() { return getWidget("mainBox"); }
		virtual void show();

	private:
		// GUI functions
		void addConnection_gui(dcpp::StringMap params, bool download);
		void removeConnection_gui(const std::string cid, bool download);

		void initTransfer_gui(dcpp::StringMap params);
		void updateTransfer_gui(dcpp::StringMap params, bool download, Sound::TypeSound sound);
		void updateFilePosition_gui(const std::string cid, int64_t filePosition);
		void updateParent_gui(GtkTreeIter* iter);
		void finishParent_gui(const std::string target, const std::string status, Sound::TypeSound sound);

		bool findParent_gui(const std::string& target, GtkTreeIter* iter);
		bool findTransfer_gui(const std::string& cid, bool download, GtkTreeIter* iter);

		void popupTransferMenu_gui();
		void playSound_gui(Sound::TypeSound sound);

		// GUI callbacks
		static gboolean onTransferButtonPressed_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static gboolean onTransferButtonReleased_gui(GtkWidget *widget, GdkEventButton *event, gpointer data);
		static void onGetFileListClicked_gui(GtkMenuItem *item, gpointer data);
		static void onMatchQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onPrivateMessageClicked_gui(GtkMenuItem *item, gpointer data);
		static void onAddFavoriteUserClicked_gui(GtkMenuItem *item, gpointer data);
		static void onGrantExtraSlotClicked_gui(GtkMenuItem *item, gpointer data);
		static void onRemoveUserFromQueueClicked_gui(GtkMenuItem *item, gpointer data);
		static void onForceAttemptClicked_gui(GtkMenuItem *item, gpointer data);
		static void onCloseConnectionClicked_gui(GtkMenuItem *item, gpointer data);

		// Client functions
		void getParams_client(dcpp::StringMap& params, dcpp::ConnectionQueueItem* cqi);
		void getParams_client(dcpp::StringMap& params, dcpp::Transfer* transfer);
		void getFileList_client(std::string cid, std::string hubUrl);
		void matchQueue_client(std::string cid, std::string hubUrl);
		void addFavoriteUser_client(std::string cid);
		void grantExtraSlot_client(std::string cid, std::string hubUrl);
		void removeUserFromQueue_client(std::string cid);
		void forceAttempt_client(std::string cid);
		void closeConnection_client(std::string cid, bool download);
		void onFailed(dcpp::Download* dl, const std::string& reason);

		// DownloadManager
		virtual void on(dcpp::DownloadManagerListener::Requesting, dcpp::Download* dl) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Starting, dcpp::Download* dl) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Tick, const dcpp::DownloadList& dls) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Complete, dcpp::Download* dl) noexcept;
		virtual void on(dcpp::DownloadManagerListener::Failed, dcpp::Download* dl, const std::string& reason) noexcept;
		// ConnectionManager
		virtual void on(dcpp::ConnectionManagerListener::Added, dcpp::ConnectionQueueItem* cqi) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::Connected, dcpp::ConnectionQueueItem* cqi) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::Removed, dcpp::ConnectionQueueItem* cqi) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::Failed, dcpp::ConnectionQueueItem* cqi, const std::string&) noexcept;
		virtual void on(dcpp::ConnectionManagerListener::StatusChanged, dcpp::ConnectionQueueItem* cqi) noexcept;
		// QueueManager
		virtual void on(dcpp::QueueManagerListener::Finished, dcpp::QueueItem* qi, const std::string&, int64_t size) noexcept;
		virtual void on(dcpp::QueueManagerListener::Removed, dcpp::QueueItem* qi) noexcept;
		virtual void on(dcpp::QueueManagerListener::CRCFailed, dcpp::Download* aDownload, const std::string& reason) noexcept;
		// UploadManager
		virtual void on(dcpp::UploadManagerListener::Starting, dcpp::Upload* ul) noexcept;
		virtual void on(dcpp::UploadManagerListener::Tick, const dcpp::UploadList& uls) noexcept;
		virtual void on(dcpp::UploadManagerListener::Complete, dcpp::Upload* ul) noexcept;
		virtual void on(dcpp::UploadManagerListener::Failed, dcpp::Upload* ul, const std::string& reason) noexcept;

		TreeView transferView;
		GtkTreeStore *transferStore;
		GtkTreeSelection *transferSelection;
		UserCommandMenu* userCommandMenu;
		PreviewMenu *appsPreviewMenu;
};

#endif // WULFOR_TRANSFERS_HH
