
#ifndef JSON_RPC_METHODS_H
#define JSON_RPC_METHODS_H

#include <json/json.h>
class JsonRpcMethods
{
  public:
    /**
     * \brief Reply with success.
     * \param root JSON-RPC request
     * \param response JSON-RPC response
     * \return true if correctly processed, false otherwise
     //*/
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

    /**
     * \brief Get the description in JSON format.
     * \return JSON description
     */
    Json::Value GetDescription();
};

#endif /* JSON_RPC_METHODS_H */
