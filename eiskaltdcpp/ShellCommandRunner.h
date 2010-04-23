/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SHELLCOMMANDRUNNER_H
#define SHELLCOMMANDRUNNER_H

/**
 * @author Edward Sheldrake
 *
 * ShellCommandRunner is a QThread which starts a QProcess, waits for it
 * to finish, gets the output and then emits a sigal.
 */

#include <QThread>
#include <QString>
#include <QList>
#include <QStringList>

class ShellCommandRunner : public QThread {
    Q_OBJECT

public:
    /** constructor */
    ShellCommandRunner(QString args, QObject * parent = 0);
    ShellCommandRunner(QString cmd, QStringList args, QObject * parent = 0);
    /** destructor */
    virtual ~ShellCommandRunner();

    /** the method that runs in the thread */
    virtual void run();

    /** Cancel the shell command e.g. if the chat is closed */
    void cancel();

signals:
    /** emitted when the command has finished */
    void finished(bool ok, QString output);

private:
    /** used to cancel the thread */
    bool stop;
    /** */
    bool useArgList;
    /** the program to run and its arguments */
    QString args;
    QStringList argList;
    QString cmd;
};

#endif // ShellCommandRunner_U
