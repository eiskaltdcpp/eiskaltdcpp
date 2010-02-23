#ifndef SINGLEINSTANCERUNNER_H
#define SINGLEINSTANCERUNNER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

const int EISKALTPORT = 33561;

class SingleInstanceRunner : public QThread
{
Q_OBJECT
public:
    explicit SingleInstanceRunner(QObject *parent = 0);
    ~SingleInstanceRunner() { serv->close(); delete serv;}

    bool isServerRunning(const QStringList&);
    virtual void run();
    void servStop() { serv->close(); }

private slots:
    void slotNewConnection();

private:
    QTcpServer *serv;
};

#endif // SINGLEINSTANCERUNNER_H
