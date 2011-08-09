
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
     */
    bool Print(const Json::Value& root, Json::Value& response);

    /**
     * \brief Notification.
     * \param root JSON-RPC request
     * \param response JSON-RPC response
     * \return true if correctly processed, false otherwise
     */
    bool Notify(const Json::Value& root, Json::Value& response);

    /**
     * \brief Get the description in JSON format.
     * \return JSON description
     */
    Json::Value GetDescription();
};

#endif /* JSON_RPC_METHODS_H */
