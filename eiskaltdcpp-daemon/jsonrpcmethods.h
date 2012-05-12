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

#include <jsoncpp/json.h>
class JsonRpcMethods
{
  public:

    bool StopDaemon(const Json::Value& root, Json::Value& response);
    bool MagnetAdd(const Json::Value& root, Json::Value& response);
    bool HubAdd(const Json::Value& root, Json::Value& response);
    bool HubDel(const Json::Value& root, Json::Value& response);
    bool HubSay(const Json::Value& root, Json::Value& response);
    bool HubSayPM(const Json::Value& root, Json::Value& response);
    bool ListHubs(const Json::Value& root, Json::Value& response);
    bool AddDirInShare(const Json::Value& root, Json::Value& response);
    bool RenameDirInShare(const Json::Value& root, Json::Value& response);
    bool DelDirFromShare(const Json::Value& root, Json::Value& response);
    bool ListShare(const Json::Value& root, Json::Value& response);
    bool RefreshShare(const Json::Value& root, Json::Value& response);
    bool GetFileList(const Json::Value& root, Json::Value& response);
    bool GetChatPub(const Json::Value& root, Json::Value& response);
    bool SendSearch(const Json::Value& root, Json::Value& response);
    bool ReturnSearchResults(const Json::Value& root, Json::Value& response);
    bool ShowVersion(const Json::Value& root, Json::Value& response);
    bool ShowRatio(const Json::Value& root, Json::Value& response);
    bool SetPriorityQueueItem(const Json::Value& root, Json::Value& response);
    bool MoveQueueItem(const Json::Value& root, Json::Value& response);
    bool RemoveQueueItem(const Json::Value& root, Json::Value& response);
    bool ListQueueTargets(const Json::Value& root, Json::Value& response);
    bool ListQueue(const Json::Value& root, Json::Value& response);
    bool ClearSearchResults(const Json::Value& root, Json::Value& response);
    bool AddQueueItem(const Json::Value& root, Json::Value& response);
    bool GetSourcesItem(const Json::Value& root, Json::Value& response);
    bool GetHashStatus(const Json::Value& root, Json::Value& response);
    bool GetMethodList(const Json::Value& root, Json::Value& response);
    bool PauseHash(const Json::Value& root, Json::Value& response);
    Json::Value GetDescriptionStopDaemon();
    Json::Value GetDescriptionMagnetAdd();
    Json::Value GetDescriptionHubAdd();
    Json::Value GetDescriptionHubDel();
    Json::Value GetDescriptionHubSay();
    Json::Value GetDescriptionHubSayPM();
    Json::Value GetDescriptionListHubs();
    Json::Value GetDescriptionAddDirInShare();
    Json::Value GetDescriptionRenameDirInShare();
    Json::Value GetDescriptionDelDirFromShare();
    Json::Value GetDescriptionListShare();
    Json::Value GetDescriptionRefreshShare();
    Json::Value GetDescriptionGetFileList();
    Json::Value GetDescriptionGetChatPub();
    Json::Value GetDescriptionSendSearch();
    Json::Value GetDescriptionReturnSearchResults();
    Json::Value GetDescriptionShowVersion();
    Json::Value GetDescriptionShowRatio();
    Json::Value GetDescriptionSetPriorityQueueItem();
    Json::Value GetDescriptionMoveQueueItem();
    Json::Value GetDescriptionRemoveQueueItem();
    Json::Value GetDescriptionListQueueTargets();
    Json::Value GetDescriptionListQueue();
    Json::Value GetDescriptionClearSearchResults();
    Json::Value GetDescriptionAddQueueItem();
    Json::Value GetDescriptionGetSourcesItem();
    Json::Value GetDescriptionGetHashStatus();
    Json::Value GetDescriptionPauseHash();
};
