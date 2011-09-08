#include "jsonrpcmethods.h"

bool JsonRpcMethods::Print(const Json::Value& root, Json::Value& response)
{
  std::cout << "Receive query: " << root << std::endl;
  response["jsonrpc"] = "2.0";
  response["id"] = root["id"];
  response["result"] = "success";
  return true;
}

bool JsonRpcMethods::Notify(const Json::Value& root, Json::Value& response)
{
  std::cout << "Notification: " << root << std::endl;
  response = Json::Value::null;
  return true;
}
bool JsonRpcMethods::StopDaemon(const Json::Value& root, Json::Value& response)
{
    std::cout << "StopDaemon: " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];

}


Json::Value JsonRpcMethods::GetDescription()
{
  Json::FastWriter writer;
  Json::Value root;
  Json::Value parameters;
  Json::Value param1;

  root["description"] = "Print";

  /* type of parameter named arg1 */
  param1["type"] = "integer";
  param1["description"] = "argument 1";

  /* push it into the parameters list */
  parameters["arg1"] = param1;
  root["parameters"] = parameters;

  /* no value returned */
  root["returns"] = Json::Value::null;

  return root;
}
