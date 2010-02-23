/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
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

class ShellCommandRunner : public QThread {
    Q_OBJECT

public:
    /** constructor */
    ShellCommandRunner(QString args, QObject * parent = 0);
    /** destructor */
    virtual ~ShellCommandRunner();

    /** the method that runs in the thread */
    virtual void run();

    /** Cancel the shell command e.g. if the chat is closed */
    void cancel();

signals:
    /** emitted when the command has finished */
    virtual void finished(bool ok, QString output);

private:
    /** used to cancel the thread */
    bool stop;
    /** the program to run and its arguments */
    QString args;
};

#endif // ShellCommandRunner_U
