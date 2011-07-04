/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "HashProgress.h"
#include "WulforUtil.h"

#include <QDir>

#include "dcpp/stdinc.h"
#include "dcpp/HashManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/TimerManager.h"

using namespace dcpp;

unsigned HashProgress::getHashStatus() {
    ShareManager *SM = ShareManager::getInstance();
    HashManager  *HM = HashManager::getInstance();
    if( SM->isRefreshing() )
        return LISTUPDATE;

    if( HM->isHashingPaused() ) {
        return (Util::getUpTime() < SETTING(HASHING_START_DELAY)) ? DELAYED : PAUSED;
    }

    string path;
    int64_t bytes = 0;
    size_t files = 0;
    HM->getStats(path, bytes, files);

    if( bytes != 0 || files != 0 )
        return RUNNING;

    return IDLE;
}

HashProgress::HashProgress(QWidget *parent):
        QDialog(parent),
        autoClose(false),
        startBytes(0),
        startFiles(0),
        startTime(0)
{
    setupUi(this);

    setWindowModality(Qt::ApplicationModal);
    setAttribute( Qt::WA_QuitOnClose, false ); // Very important, wo this line app exits then hide

    HashManager::getInstance()->setPriority(Thread::NORMAL);

    timer = new QTimer();
    timer->setInterval(250);

    connect(timer, SIGNAL(timeout()), this, SLOT(timerTick()));
    connect(pushButton_START, SIGNAL(clicked()), this, SLOT(slotStart()));
    connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(slotAutoClose(bool)));

    timer->start();
}

void HashProgress::resetProgress() {
        startBytes = 0;
        startFiles = 0;
        startTime  = 0;
}

HashProgress::~HashProgress(){
    timer->stop();//really need?

    delete timer;

    HashManager::getInstance()->setPriority(Thread::LOW);
}

float HashProgress::getProgress() {
    return static_cast<float>( progress->value() )/progress->maximum();
}

void HashProgress::timerTick(){
    string path;
    int64_t bytes = 0;
    size_t files = 0;
    uint64_t tick = GET_TICK();

    stateButton();

    HashManager::getInstance()->getStats(path, bytes, files);
    if(ShareManager::getInstance()->isRefreshing()) {
        file->setText(tr("Refreshing file list"));
        return;
    }

    if( startTime == 0 )
        startTime = tick;

    if(bytes > startBytes)
        startBytes = bytes;

    if(files > startFiles)
        startFiles = files;

    if(autoClose && files == 0) {
        accept();

        return;;
    }

    double diff = tick - startTime;
    bool paused = HashManager::getInstance()->isHashingPaused();

    QString eta;

    if(startFiles == 0 || startBytes == 0)
        progress->setValue(0);
    else
        progress->setValue( (10000*(startBytes - bytes))/startBytes);

    if( diff == 0. || files == 0 || bytes == 0 || paused) {
        stat->setText(QString(tr("-.-- files/h, %1 files left")).arg((uint32_t)files));
        speed->setText(tr("-.-- B/s, %1 left").arg(WulforUtil::formatBytes(bytes)));
        eta = tr("-:--:--");
    } else {
        double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
        double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

        stat->setText(tr("%1 files/h, %2 files left").arg(filestat).arg((uint32_t)files));
        speed->setText(tr("%1/s, %2 left, %3 shared").arg(WulforUtil::formatBytes((int64_t)speedStat))
                                                     .arg(WulforUtil::formatBytes(bytes))
                                                     .arg(WulforUtil::formatBytes(ShareManager::getInstance()->getShareSize())));

        if(/*filestat == 0 ||*/ speedStat == 0) {
            eta = tr("-:--:--");
        }
        else {
            double ss = bytes / speedStat;

            eta = _q(Text::toT(Util::formatSeconds((int64_t)(ss))));
        }
    }
    progress->setFormat( tr("%p% %1 left").arg(eta) );

    if(files == 0 ) {
        //progress->setValue(10000); // generates anoying blinking 0 -> 100%
        file->setText(tr("Done"));
    } else {
        QString fname = QString::fromStdString(path);
        QFontMetrics metrics(font());

        file->setToolTip(fname);

        if (metrics.width(fname) > file->width()*3/4){
            QStringList parts = fname.split(QDir::separator(), QString::SkipEmptyParts);

            if (parts.size() > 1){
                QString out = "";

                for (int i = (parts.size()-1); i >= 0; i--){
                    if (metrics.width(out+parts.at(i)+QDir::separator()) < file->width()*3/4){
                        out = parts.at(i) + (out.isEmpty()? out : (QDir::separator() + out));
                    }
                    else{
                        out = QString("..") + QDir::separator() + out;

                        break;
                    }
                }

                if (out.isEmpty())
                    out = parts.last();

                fname = out;
            }
        }

        file->setText(fname);
    }
}

void HashProgress::slotStart(){
    ShareManager *SM = ShareManager::getInstance();
    HashManager  *HM = HashManager::getInstance();
    switch( getHashStatus() ) {
    case IDLE:
            SM->setDirty();
            SM->refresh(true);
            break;
    case LISTUPDATE:
    case RUNNING:
            HM->pauseHashing();
            //HM->setPriority(Thread::IDLE);
            break;
    case DELAYED:
    case PAUSED:
            HM->resumeHashing();
            //HM->setPriority(Thread::NORMAL);
            break;
    }
    stateButton();
}

void HashProgress::slotAutoClose(bool b){
    autoClose = b;

    blockSignals(true);
    checkBox->setChecked(b);
    blockSignals(false);
}

void HashProgress::stateButton(){
    switch( getHashStatus() ) {
    case IDLE:
        pushButton_START->setText(tr("Start"));
        break;
    case LISTUPDATE:
    case RUNNING:
        pushButton_START->setText(tr("Pause"));
        break;
    case DELAYED:
    case PAUSED:
        pushButton_START->setText(tr("Resume"));
        break;
    }
}
