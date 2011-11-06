#include "stdafx.h"
#include "jsonrpcmethods.h"
#include "ServerManager.h"
#include "utility.h"
#include "ServerThread.h"

using namespace std;

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
    std::cout << "StopDaemon (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = 0;
    bServerTerminated = true;
    std::cout << "StopDaemon (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::MagnetAdd(const Json::Value& root, Json::Value& response)
{
    std::cout << "MagnetAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::string name,tth;int64_t size;

    bool ok = splitMagnet(root["magnet"].asString(), name, size, tth);
#ifdef _DEBUG
    fprintf(stderr,"tth: %s\n",tth.c_str());
    fprintf(stderr,"size: %d\n",size);
    fprintf(stderr,"name: %s\n",name.c_str());
    fflush(stderr);
#endif
    if (ok && ServerThread::getInstance()->addInQueue(root["directory"].asString(), name, size, tth))
        response["result"] = 0;
    else
        response["result"] = 1;
    std::cout << "MagnetAdd (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubAdd(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubAdd (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->connectClient(root["huburl"].asString(), root["enc"].asString());
    response["result"] = "Connecting to " + root["huburl"].asString();
    std::cout << "HubAdd (response): " << response << std::endl;
    return true;
}
bool HubDel(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubDel (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "HubDel (response): " << response << std::endl;
    return true;
}

bool HubSay(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubSay (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "HubSay (response): " << response << std::endl;
    return true;
}

bool HubSayPM(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubSayPM (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "HubSayPM (response): " << response << std::endl;
    return true;
}

bool ListHubs(const Json::Value& root, Json::Value& response)
{
    std::cout << "ListHubs (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "ListHubs (response): " << response << std::endl;
    return true;
}

bool AddDirInShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "AddDirInShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "AddDirInShare (response): " << response << std::endl;
    return true;
}

bool RenameDirInShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "RenameDirInShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "RenameDirInShare (response): " << response << std::endl;
    return true;
}

bool DelDirFromShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "DelDirFromShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "DelDirFromShare (response): " << response << std::endl;
    return true;
}

bool ListShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "ListShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "ListShare (response): " << response << std::endl;
    return true;
}

bool RefreshShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "RefreshShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "RefreshShare (response): " << response << std::endl;
    return true;
}

bool GetFileList(const Json::Value& root, Json::Value& response)
{
    std::cout << "GetFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "GetFileList (response): " << response << std::endl;
    return true;
}

bool GetChatPub(const Json::Value& root, Json::Value& response)
{
    std::cout << "GetChatPub (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "GetChatPub (response): " << response << std::endl;
    return true;
}

bool SendSearch(const Json::Value& root, Json::Value& response)
{
    std::cout << "SendSearch (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "SendSearch (response): " << response << std::endl;
    return true;
}

bool ReturnSearchResults(const Json::Value& root, Json::Value& response)
{
    std::cout << "ReturnSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "ReturnSearchResults (response): " << response << std::endl;
    return true;
}

bool ShowVersion(const Json::Value& root, Json::Value& response)
{
    std::cout << "ShowVersion (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "ShowVersion (response): " << response << std::endl;
    return true;
}

bool ShowRatio(const Json::Value& root, Json::Value& response)
{
    std::cout << "ShowRatio (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    std::cout << "ShowRatio (response): " << response << std::endl;
    return true;
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
