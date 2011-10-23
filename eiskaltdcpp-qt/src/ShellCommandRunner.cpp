/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "ShellCommandRunner.h"

#include <QProcess>
#include <cstdio>

/** */
ShellCommandRunner::ShellCommandRunner(QString a, QObject * parent) : QThread(parent), _exitCode(-1) {
    args = a;
    stop = false;
    useArgList = false;
}

ShellCommandRunner::ShellCommandRunner(QString cmd, QStringList argList, QObject * parent) : QThread(parent), _exitCode(-1) {
    this->argList = argList;
    this->cmd = cmd;
    useArgList = true;
    stop = false;
}

/** */
ShellCommandRunner::~ShellCommandRunner() {
    cancel();
    /* the longest total sleep in run() is 1.1 seconds */
    if (!wait(1150)) {
        printf("~ShellCommandRunner: warning not finished\n");
    }
}

/** */
void ShellCommandRunner::run() {
    _exitCode = 1;
    QString output;
    bool succeeded = false;
    QProcess process;

    if (useArgList)
        process.start(cmd, argList);
    else
        process.start(args);

    process.closeWriteChannel();
    process.waitForFinished(100);

    int i = 0;
    while (((process.state() == QProcess::Starting) || (process.state() == QProcess::Running)) && (i < 2400) && (stop == false)) {
        process.waitForFinished(50);
        i++;
    }

    if (stop) {
        /* just kill the damn process */
        process.terminate();
        QThread::msleep(100);
        process.close();
        if (process.state() != QProcess::NotRunning) {
            process.kill();
            QThread::msleep(100);
        }
        return;
    }

    if ((process.state() == QProcess::NotRunning)) {
        if (process.exitStatus() == QProcess::NormalExit) {
            _exitCode = process.exitCode();
            if (_exitCode == 0) {
                output = QString(process.readAllStandardOutput()).trimmed();
                
                if (output.isEmpty())
                    output = tr("Command produced no visible output.");
                
                succeeded = true;
            } else {
                output = QString(process.readAllStandardError()).trimmed();
            }
        } else {
            output = QString(process.readAllStandardError()).trimmed();
        }
    } else {
        _exitCode = 1;
        output = QString(process.readAllStandardError()).trimmed();
        
        process.terminate();
        QThread::msleep(1000);
        process.close();
        if (process.state() != QProcess::NotRunning) {
            process.kill();
            QThread::msleep(100);
        }
    }

    if (!stop) // stop is only set to true when things are being deleted
        emit finished(succeeded, output);
}

/** */
void ShellCommandRunner::cancel() {
    stop = true;
}
