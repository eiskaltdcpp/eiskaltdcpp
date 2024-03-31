/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <cstdint>

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
    HashProgress(QWidget* = nullptr);
    virtual ~HashProgress();
    enum { IDLE, RUNNING, LISTUPDATE, PAUSED, DELAYED };
    static unsigned getHashStatus();
    float getProgress();

public slots:
    void slotAutoClose(bool);
    void resetProgress();

private slots:
    void timerTick();
    void slotStart();
    void stateButton();

private:
    QTimer *timer;

    bool autoClose;
    uint64_t startBytes;
    size_t startFiles;
    uint64_t startTime;
};
