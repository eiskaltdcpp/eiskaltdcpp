#include "stdafx.h"
#include "jsonrpcmethods.h"
#include "ServerManager.h"
#include "utility.h"
#include "ServerThread.h"
#include "VersionGlobal.h"
#include "dcpp/format.h"

using namespace std;

bool JsonRpcMethods::Print(const Json::Value& root, Json::Value& response)
{
    std::cout << "Receive query: " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    response["result"] = "success";
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
bool JsonRpcMethods::HubDel(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubDel (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ServerThread::getInstance()->disconnectClient(root["huburl"].asString());
    response["result"] = 0;
    std::cout << "HubDel (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSay(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubSay (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->findHubInConnectedClients(root["huburl"].asString())) {
        ServerThread::getInstance()->sendMessage(root["huburl"].asString(),root["message"].asString());
        response["result"] = 0;
    } else
        response["result"] = 1;
    std::cout << "HubSay (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::HubSayPM(const Json::Value& root, Json::Value& response)
{
    std::cout << "HubSayPM (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp = ServerThread::getInstance()->sendPrivateMessage(root["huburl"].asString(), root["nick"].asString(), root["message"].asString());
    response["result"] = tmp;
    std::cout << "HubSayPM (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListHubs(const Json::Value& root, Json::Value& response)
{
    std::cout << "ListHubs (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string listhubs;
    ServerThread::getInstance()->listConnectedClients(listhubs, root["separator"].asString());
    response["result"] = listhubs;
    std::cout << "ListHubs (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::AddDirInShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "AddDirInShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    try {
        if (ServerThread::getInstance()->addDirInShare(root["directory"].asString(), root["virtname"].asString()))
            response["result"] = 0;
        else
            response["result"] = 1;
    } catch (const ShareException& e) {
        response["result"] = e.getError();
    }
    std::cout << "AddDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RenameDirInShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "RenameDirInShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    try {
        if (ServerThread::getInstance()->renameDirInShare(root["directory"].asString(), root["virtname"].asString()))
            response["result"] = 0;
        else
            response["result"] = 1;
    } catch (const ShareException& e) {
        response["result"] = e.getError();
    }
    std::cout << "RenameDirInShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::DelDirFromShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "DelDirFromShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->delDirFromShare(root["directory"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    std::cout << "DelDirFromShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ListShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "ListShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string listshare;
    ServerThread::getInstance()->listShare(listshare, root["separator"].asString());
    response["result"] = listshare;
    std::cout << "ListShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::RefreshShare(const Json::Value& root, Json::Value& response)
{
    std::cout << "RefreshShare (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    ShareManager::getInstance()->setDirty();
    ShareManager::getInstance()->refresh(true);
    response["result"] = 0;
    std::cout << "RefreshShare (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetFileList(const Json::Value& root, Json::Value& response)
{
    std::cout << "GetFileList (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string tmp;
    tmp = ServerThread::getInstance()->getFileList_client(root["huburl"].asString(), root["nick"].asString(), false);
    response["result"] = tmp;
    std::cout << "GetFileList (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::GetChatPub(const Json::Value& root, Json::Value& response)
{
    std::cout << "GetChatPub (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string retchat;
    ServerThread::getInstance()->getChatPubFromClient(retchat, root["huburl"].asString(), root["separator"].asString());
    response["result"] = retchat;
    std::cout << "GetChatPub (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::SendSearch(const Json::Value& root, Json::Value& response)
{
    std::cout << "SendSearch (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    if (ServerThread::getInstance()->sendSearchonHubs(root["searchstring"].asString(), root["searchtype"].asInt(), root["sizemode"].asInt(), root["sizetype"].asInt(), root["sizetype"].asDouble(), root["huburls"].asString()))
        response["result"] = 0;
    else
        response["result"] = 1;
    std::cout << "SendSearch (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ReturnSearchResults(const Json::Value& root, Json::Value& response)
{
    std::cout << "ReturnSearchResults (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    vector<StringMap> tmp;
    Json::Value parameters;
    ServerThread::getInstance()->returnSearchResults(tmp);
    for (vector<StringMap>::iterator i = tmp.begin(); i != tmp.end(); ++i) {
            Json::Value param;
            for (StringMap::iterator kk = (*i).begin(); kk != (*i).end(); ++kk) {
                param[kk->first] = kk->second;
            }
            //parameters[string(i)] = param;
        }
    response["result"] = parameters;
    std::cout << "ReturnSearchResults (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowVersion(const Json::Value& root, Json::Value& response)
{
    std::cout << "ShowVersion (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    string version(EISKALTDCPP_VERSION);
    version.append(" (");
    version.append(EISKALTDCPP_VERSION_SFX);
    version.append(")");
    response["result"] = version;
    std::cout << "ShowVersion (response): " << response << std::endl;
    return true;
}

bool JsonRpcMethods::ShowRatio(const Json::Value& root, Json::Value& response)
{
    std::cout << "ShowRatio (root): " << root << std::endl;
    response["jsonrpc"] = "2.0";
    response["id"] = root["id"];
    double ratio;
    double up   = static_cast<double>(SETTING(TOTAL_UPLOAD));
    double down = static_cast<double>(SETTING(TOTAL_DOWNLOAD));

    if (down > 0)
        ratio = up / down;
    else
        ratio = 0;

    char ratio_c[32];
    sprintf(ratio_c,"%.3f", ratio);

    string uploaded = Util::formatBytes(up);
    string downloaded = Util::formatBytes(down);

    string line = str(dcpp_fmt("ratio: %1% (uploads: %2%, downloads: %3% )")
    % string(ratio_c) % uploaded % downloaded);
    response["result"] = line;
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
