#ifndef SINGLEINSTANCERUNNER_H
#define SINGLEINSTANCERUNNER_H

#include <QThread>
#include <QTcpServer>
#include <QTcpSocket>

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
    unsigned EISKALTPORT;
};

#endif // SINGLEINSTANCERUNNER_H
