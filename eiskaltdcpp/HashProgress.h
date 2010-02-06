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

private slots:
    void timerTick();

private:
    QTimer *timer;

    bool autoClose;
    qint64 startBytes;
    size_t startFiles;
    qint32 startTime;
};

#endif // HASHPROGRESS_H
