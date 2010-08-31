#include <stdlib.h>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/forward.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/ClientManager.h"
#include "dcpp/ConnectionManager.h"
#include "dcpp/HashManager.h"

#ifdef USE_MINIUPNP
#include "miniupnp/upnpc.h"
#include <dcpp/UPnPManager.h>//NOTE: core 0.762
#endif

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WFitLayout>

#include <Wt/Ext/Menu>
#include <Wt/Ext/ToolBar>
#include <Wt/Ext/Button>
#include <Wt/Ext/Dialog>
#include <Wt/Ext/Panel>

#include <Wt/WString>

#include "SearchFrame.h"
#include "DownloadQueue.h"
#include "Utils.h"
#include "Version.h"

using namespace Wt;
using namespace dcpp;


void printHelp() {
    printf("Using:\n"
           "  eiskaltdcpp-wt --http-port=<port> --http-address=<address> --docroot <path>\n"
           "  eiskaltdcpp-wt <Key>\n"
           "EiskaltDC++ is a program for UNIX-like systems that uses the Direct Connect and ADC protocol.\n"
           "\n"
           "Keys:\n"
           "  -h, --help\t Show this message\n"
           "  -v, --version\t Show version string\n"
           "Examples:\n"
           "  eiskaltdcpp-wt --http-port=\"8080\" --http-address=\"0.0.0.0\" --docroot \"/usr/share/eiskaltdcpp/wt/\"\n"
           );
}

void printVersion() {
#ifndef DCPP_REVISION
    printf("%s (%s)\n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX);
#else
    printf("%s - %s %s \n", EISKALTDCPP_VERSION, EISKALTDCPP_VERSION_SFX, DCPP_REVISION);
#endif
}

void callBack(void* x, const std::string& a)
{
    std::cout << "Loading: " << a << std::endl;
}

void autoconnect();
void startSocket();

class WApp: public Wt::WApplication{
public:
    WApp(const Wt::WEnvironment &env): Wt::WApplication(env), dq(NULL), sfr(NULL){
        setTitle("EiskaltDC++ Web Control");

        useStyleSheet("eiskaltdcpp.css");
        messageResourceBundle().use("eiskaltdcpp");

        toolbar = new Ext::ToolBar(root());

        search_btn = toolbar->addButton("Search");
        search_btn->setIcon("resources/edit-find.png");

        download_btn = toolbar->addButton("Download Queue");
        download_btn->setIcon("resources/download.png");

        dq   = NULL;
        sfr  = NULL;
        currentWidget = NULL;

        search_btn->clicked().connect(this, &WApp::showSearchFrame);
        download_btn->clicked().connect(this, &WApp::showDQFrame);
    }

    void showSearchFrame(){
        sfr = sfr? sfr : new SearchFrame(root());

        if (currentWidget == sfr)
            return;

        if (currentWidget)
            root()->removeWidget(currentWidget);

        root()->addWidget(sfr);

        currentWidget = sfr;
    }

    void showDQFrame(){
        dq = dq? dq : new DownloadQueue(root());

        if (currentWidget == dq)
            return;

        if (currentWidget)
            root()->removeWidget(currentWidget);

        root()->addWidget(dq);

        currentWidget = dq;
    }

    virtual ~WApp() {
        delete toolbar;
        delete sfr;
    }

private:
    Ext::ToolBar *toolbar;
    Ext::Button *search_btn;
    Ext::Button *download_btn;

    SearchFrame *sfr;
    DownloadQueue *dq;

    WContainerWidget *currentWidget;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */

  return new WApp(env);
}

int main(int argc, char** argv) {

    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--help") || !strcmp(argv[i],"-h")){
            printHelp();
            exit(0);
        }
        else if (!strcmp(argv[i],"--version") || !strcmp(argv[i],"-v")){
            printVersion();
            exit(0);
        }
    }

    Utils::init();

    dcpp::startup(callBack, NULL);
    dcpp::TimerManager::getInstance()->start();
    dcpp::HashManager::getInstance()->setPriority(Thread::IDLE);

#ifdef USE_MINIUPNP
    dcpp::UPnPManager::getInstance()->addImplementation(new UPnPc());//NOTE: core 0.762
#endif

    startSocket();

    autoconnect();

    int ret = Wt::WRun(argc, argv, &createApplication);

    dcpp::shutdown();

    return ret;
}

void autoconnect(){
    const FavoriteHubEntryList& fl = FavoriteManager::getInstance()->getFavoriteHubs();

    for(FavoriteHubEntryList::const_iterator i = fl.begin(); i != fl.end(); ++i) {
        FavoriteHubEntry* entry = *i;

        if (entry->getConnect()) {
            if (entry->getNick().empty() && SETTING(NICK).empty())
                continue;

            std::string enc = entry->getEncoding();
            std::string serv = entry->getServer();

            Client *client = ClientManager::getInstance()->getClient(serv);
            client->setEncoding(enc);
            client->setPassword(entry->getPassword());
            client->password(entry->getPassword());

            client->connect();
        }
    }
}

void startSocket(){
    SearchManager::getInstance()->disconnect();
    ConnectionManager::getInstance()->disconnect();

    if (ClientManager::getInstance()->isActive()) {
        try {
            ConnectionManager::getInstance()->listen();
        } catch(const Exception &e) {
            printf("%s %s %s\n", "Cannot listen socket because: \n", e.getError().c_str(), "\n\nPlease check your connection settings");
        }
        try {
            SearchManager::getInstance()->listen();
        } catch(const Exception &e) {
            printf("%s %s %s\n", "Cannot listen socket because: \n", e.getError().c_str(), "\n\nPlease check your connection settings");
        }
    }

#ifdef USE_MINIUPNP
    if( SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_UPNP )
        UPnPManager::getInstance()->open();
    else if (SETTING(INCOMING_CONNECTIONS) != SettingsManager::INCOMING_FIREWALL_UPNP && UPnPManager::getInstance()->getOpened())
        UPnPManager::getInstance()->close();
#endif
}

