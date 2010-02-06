#include "Magnet.h"

#include <QUrl>
#include <QMessageBox>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Util.h"
#include "dcpp/ClientManager.h"
#include "dcpp/SettingsManager.h"
#include "dcpp/QueueManager.h"

#include "WulforUtil.h"
#include "WulforManager.h"
#include "SearchFrame.h"
#include "Func.h"

using namespace dcpp;

Magnet::Magnet(QWidget *parent) :
    QDialog(parent)
{
    setupUi(this);

    connect(pushButton_CANCEL,  SIGNAL(clicked()), this, SLOT(accept()));
    connect(pushButton_SEARCH,  SIGNAL(clicked()), this, SLOT(search()));
    connect(pushButton_DOWNLOAD,SIGNAL(clicked()), this, SLOT(download()));
}

Magnet::~Magnet(){
    SearchManager::getInstance()->removeListener(this);
}

void Magnet::customEvent(QEvent *e){
    if (e->type() == MagnetCustomEvent::Event){
        MagnetCustomEvent *c_e = reinterpret_cast<MagnetCustomEvent*>(e);

        c_e->func()->call();
    }

    e->accept();
}

void Magnet::setLink(const QString &link){
    if (link.isEmpty() || !link.startsWith("magnet:?xt=urn:tree:tiger:"))
        return;

    QUrl url;

    if (!link.contains("+"))
        url.setEncodedUrl(link.toAscii());
    else {
        QString _l = link;

        _l.replace("+", "%20");
        url.setEncodedUrl(_l.toAscii());
    }

    if (url.hasQueryItem("dn"))
        lineEdit_FNAME->setText(url.queryItemValue("dn"));

    if (url.hasQueryItem("xl"))
        lineEdit_SIZE->setText(_q(Util::formatBytes(url.queryItemValue("xl").toLongLong())));

    QString tth = link;

    tth.replace("magnet:?xt=urn:tree:tiger:", "");//remove magnet signature

    if (!lineEdit_SIZE->text().isEmpty())
        tth = tth.left(tth.indexOf("&xl="));
    else if (!lineEdit_FNAME->text().isEmpty())
        tth = tth.left(tth.indexOf("&dn="));

    lineEdit_TTH->setText(tth);
    lineEdit_LINK->setText(link);

    setWindowTitle(lineEdit_FNAME->text());
}

void Magnet::search(){
    QString tth = lineEdit_TTH->text();

    if (tth.isEmpty())
        return;

    SearchFrame *fr = new SearchFrame();
    fr->setAttribute(Qt::WA_DeleteOnClose);

    fr->searchAlternates(tth);

    typedef Func0<Magnet> FUNC;
    FUNC *f = new FUNC(this, &Magnet::accept);

    QApplication::postEvent(this, new MagnetCustomEvent(f));
}

void Magnet::download(){
    QString tth = lineEdit_TTH->text();

    if (tth.isEmpty())
        return;

    StringList client_list;

    ClientManager* clientMgr = ClientManager::getInstance();

    clientMgr->lock();
    Client::List& clients = clientMgr->getClients();

    for(Client::List::iterator it = clients.begin(); it != clients.end(); ++it) {
        Client* client = *it;

        if(!client->isConnected())
            continue;

        client_list.push_back(client->getHubUrl());
    }

    clientMgr->unlock();

    if (client_list.empty()){
        QMessageBox::critical(this, tr("Error"), tr("You have not active connections to hubs"));

        accept();;
    }

    string token = Util::toString(Util::rand());
    SearchManager::SizeModes sizeMode = SearchManager::SIZE_DONTCARE;
    SearchManager::TypeModes ftype = SearchManager::TYPE_TTH;

    if(SearchManager::getInstance()->okToSearch()){
        pushButton_SEARCH->setEnabled(false);
        pushButton_DOWNLOAD->setEnabled(false);
        pushButton_CANCEL->setEnabled(false);

        setWindowTitle(tr("Please, wait..."));

        SearchManager::getInstance()->addListener(this);

        SearchManager::getInstance()->search(client_list, _tq(tth), 0, ftype, sizeMode, token);
    }
    else {
        QMessageBox::information(this, tr(""), tr("Search Manager not ready. Please, try again later."));

        accept();
    }
}

void Magnet::on(SearchManagerListener::SR, const dcpp::SearchResultPtr &result) throw(){
    if (!result)
        return;

    string tth = _tq(lineEdit_TTH->text());

    if (result->getType() != SearchResult::TYPE_FILE || TTHValue(tth) != result->getTTH())
        return;

    try {
        SearchManager::getInstance()->removeListener(this);

        UserPtr user = result->getUser();
        string target = SETTING(DOWNLOAD_DIRECTORY) + _tq(_q(result->getFileName()).split("\\", QString::SkipEmptyParts).last());

        QueueManager::getInstance()->add(target, result->getSize(), result->getTTH(), user, result->getHubURL());

        typedef Func0<Magnet> FUNC;
        FUNC *f = new FUNC(this, &Magnet::accept);

        QApplication::postEvent(this, new MagnetCustomEvent(f));
    }
    catch (const Exception &){}
}
