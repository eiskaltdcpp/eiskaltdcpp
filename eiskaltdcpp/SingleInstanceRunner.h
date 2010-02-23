#ifndef SINGLEINSTANCERUNNER_H
#define SINGLEINSTANCERUNNER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

const int EISKALTPORT = 33561;

class SingleInstanceRunner: public QObject
{
Q_OBJECT
public:
    explicit SingleInstanceRunner();
    ~SingleInstanceRunner() {}

    bool isServerRunning(const QStringList&);
    void servStop() { serv.close(); }

private slots:
    void slotNewConnection();
    void slotReadyRead();

private:
    QTcpServer serv;
};

#endif // SINGLEINSTANCERUNNER_H
