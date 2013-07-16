/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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
 */

#pragma once

/** @file
 * This file contains forward declarations for the various DC++ classes
 */

#include <boost/intrusive_ptr.hpp>

namespace dcpp {

class AdcCommand;

class ADLSearch;

class BufferedSocket;

class Bundle;
typedef boost::intrusive_ptr<Bundle> BundlePtr;

struct ChatMessage;

class CID;

class Client;

class ClientManager;

class ConnectionQueueItem;

class CRC32Filter;

class Download;
typedef Download* DownloadPtr;

class FavoriteHubEntry;
typedef FavoriteHubEntry* FavoriteHubEntryPtr;

class FavoriteUser;

class File;

class FinishedFileItem;
typedef boost::intrusive_ptr<FinishedFileItem> FinishedFileItemPtr;

class FinishedUserItem;
typedef boost::intrusive_ptr<FinishedUserItem> FinishedUserItemPtr;

class FinishedManager;

template<class Hasher>
struct HashValue;

struct HintedUser;

class HttpConnection;

struct HttpDownload;

class HubEntry;

class Identity;

class InputStream;

class LogManager;

class OnlineUser;
typedef OnlineUser* OnlineUserPtr;

class OutputStream;

class QueueItem;
typedef QueueItem* QueueItemPtr;

class SearchResult;
typedef boost::intrusive_ptr<SearchResult> SearchResultPtr;

class Socket;
class SocketException;

class StringSearch;

class TigerHash;

class Transfer;

typedef HashValue<TigerHash> TTHValue;

class UnZFilter;

class Upload;
typedef Upload* UploadPtr;

class User;
typedef boost::intrusive_ptr<User> UserPtr;

class UserCommand;

class UserConnection;
typedef UserConnection* UserConnectionPtr;

struct UserMatch;

class WindowInfo;

} // namespace dcpp
