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

#ifndef JSON_RPC_METHODS_H
#define JSON_RPC_METHODS_H

#include <jsoncpp/json.h>
class JsonRpcMethods
{
  public:
    bool Print(const Json::Value& root, Json::Value& response);

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
    

};

#endif /* JSON_RPC_METHODS_H */
