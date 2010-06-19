/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef HASHPROGRESS_H
#define HASHPROGRESS_H

#include <QDialog>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>

#include "ui_UIHashProgressDialog.h"

class HashProgress :
        public QDialog,
        private Ui::UIHashDialog
{
    Q_OBJECT
public:
    HashProgress(QWidget* = NULL);
    virtual ~HashProgress();
    enum { IDLE, RUNNING, PAUSED };
    static unsigned getHashStatus();

public slots:
    void slotAutoClose(bool);

private slots:
    void timerTick();
    void slotStart();
    void stateButton();

private:
    QTimer *timer;

    bool autoClose;
    qint64 startBytes;
    size_t startFiles;
    qint32 startTime;
};

#endif // HASHPROGRESS_H
