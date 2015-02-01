/***************************************************************************
*                                                                         *
*   Copyright 2011 Eugene Petrov <dhamp@ya.ru>                            *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <json/json.h>
class JsonRpcMethods
{
  public:

    bool StopDaemon(const Json::Value&, Json::Value&);
    bool MagnetAdd(const Json::Value&, Json::Value&);
    bool HubAdd(const Json::Value&, Json::Value&);
    bool HubDel(const Json::Value&, Json::Value&);
    bool HubSay(const Json::Value&, Json::Value&);
    bool HubSayPM(const Json::Value&, Json::Value&);
    bool ListHubs(const Json::Value&, Json::Value&);
    bool AddDirInShare(const Json::Value&, Json::Value&);
    bool RenameDirInShare(const Json::Value&, Json::Value&);
    bool DelDirFromShare(const Json::Value&, Json::Value&);
    bool ListShare(const Json::Value&, Json::Value&);
    bool RefreshShare(const Json::Value&, Json::Value&);
    bool GetFileList(const Json::Value&, Json::Value&);
    bool GetChatPub(const Json::Value&, Json::Value&);
    bool SendSearch(const Json::Value&, Json::Value&);
    bool ReturnSearchResults(const Json::Value&, Json::Value&);
    bool ShowVersion(const Json::Value&, Json::Value&);
    bool ShowRatio(const Json::Value&, Json::Value&);
    bool SetPriorityQueueItem(const Json::Value&, Json::Value&);
    bool MoveQueueItem(const Json::Value&, Json::Value&);
    bool RemoveQueueItem(const Json::Value&, Json::Value&);
    bool ListQueueTargets(const Json::Value&, Json::Value&);
    bool ListQueue(const Json::Value&, Json::Value&);
    bool ClearSearchResults(const Json::Value&, Json::Value&);
    bool AddQueueItem(const Json::Value&, Json::Value&);
    bool GetSourcesItem(const Json::Value&, Json::Value&);
    bool GetHashStatus(const Json::Value&, Json::Value&);
    bool GetMethodList(const Json::Value&, Json::Value&);
    bool PauseHash(const Json::Value&, Json::Value&);
    bool MatchAllLists(const Json::Value&, Json::Value&);
    bool ListHubsFullDesc(const Json::Value&, Json::Value&);
    bool GetHubUserList(const Json::Value&, Json::Value&);
    bool GetUserInfo(const Json::Value&, Json::Value&);
    bool OpenFileList(const Json::Value&, Json::Value&);
    bool ShowLocalLists(const Json::Value&, Json::Value&);
    bool GetClientFileList(const Json::Value&, Json::Value&);
    bool CloseFileList(const Json::Value&, Json::Value&);
    bool CloseAllFileLists(const Json::Value&, Json::Value&);
    bool ShowOpenedLists(const Json::Value&, Json::Value&);
    bool LsDirInList(const Json::Value&, Json::Value&);
    bool DownloadDirFromList(const Json::Value&, Json::Value&);
    bool DownloadFileFromList(const Json::Value&, Json::Value&);
    bool GetItemDescbyTarget(const Json::Value&, Json::Value&);
    bool QueueClear(const Json::Value&, Json::Value&);
    bool SettingsGetSet(const Json::Value&, Json::Value&);
    bool IpFilterOnOff(const Json::Value&, Json::Value&);
    bool IpFilterList(const Json::Value&, Json::Value&);
    bool IpFilterAddRules(const Json::Value&, Json::Value&);
    bool IpFilterPurgeRules(const Json::Value&, Json::Value&);
    bool IpFilterUpDownRule(const Json::Value&, Json::Value&);
private:
    void FailedValidateRequest(Json::Value &error);
};
