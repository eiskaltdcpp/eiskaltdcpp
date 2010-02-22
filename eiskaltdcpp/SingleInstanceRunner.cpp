#include "SingleInstanceRunner.h"
#include "MainWindow.h"
#include "WulforManager.h"
#include "Func.h"

SingleInstanceRunner::SingleInstanceRunner(QObject *parent) :
    QThread(parent)
{
}

void SingleInstanceRunner::run(){
    serv.moveToThread(this);
    serv.setParent(this);

    connect(&serv, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
    serv.listen(QHostAddress("127.0.0.1"), EISKALTPORT);

    QThread::exec();
}

bool SingleInstanceRunner::isServerRunning(const QStringList &list){
    QTcpSocket sock(NULL);

    sock.connectToHost(QHostAddress("127.0.0.1"), EISKALTPORT, QIODevice::WriteOnly);

    if (!sock.waitForConnected(1000)){
        start();

        return false;
    }

    if (sock.isWritable() && sock.isOpen()){
        QString data = "";

        foreach(QString s, list)
            data += s + "\n";

        sock.write(data.toAscii());

        sock.flush();

        sock.close();

        return true;
    }
    else {
        start();

        return false;
    }
}

void SingleInstanceRunner::slotNewConnection(){
    QTcpSocket *sock = NULL;

    while (serv.hasPendingConnections()){
        sock = serv.nextPendingConnection();

        QString data = sock->readAll();

        if (!data.isEmpty()){
            Func1<MainWindow, QString> *f = new Func1<MainWindow, QString>(MainWindow::getInstance(), &MainWindow::parseInstanceLine, data);

            MainWindowCustomEvent *e = new MainWindowCustomEvent(f);

            QApplication::postEvent(MainWindow::getInstance(), e);
        }

        sock->close();
    }
}
