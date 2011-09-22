/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#include <QApplication>
#include <QSharedMemory>

class QtSingleCoreApplication : public QApplication
{
    Q_OBJECT

public:
    QtSingleCoreApplication(int &argc, char **argv, const QString &key);
    QtSingleCoreApplication(const QString &id, int &argc, char **argv);

    bool isRunning();

public Q_SLOTS:
    bool sendMessage(QString message);
    void checkForMessage();

Q_SIGNALS:
    void messageReceived(const QString &message);

private:
    bool _isRunning;
    QSharedMemory sharedMemory;
};
