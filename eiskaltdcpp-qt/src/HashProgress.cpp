/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "HashProgress.h"
#include "QtContext.h"
#include "WulforUtil.h"

#include <QDir>

#include "dcpp/stdinc.h"
#include "dcpp/HashManager.h"
#include "dcpp/ShareManager.h"
#include "dcpp/TimerManager.h"
#include "dcpp/DCPlusPlus.h"

using namespace dcpp;

unsigned HashProgress::getHashStatus() {
    auto& ctx = qtCtx()->dcCtx();
    if (!ctx.isRunning()) return IDLE;
    ShareManager *SM = ctx.getShareManager();
    HashManager  *HM = ctx.getHashManager();
    if (!SM || !HM) return IDLE;
    if( SM->isRefreshing() )
        return LISTUPDATE;

    if( HM->isHashingPaused() ) {
        return (Util::getUpTime() < qtCtx()->dcCtx().getSettingsManager()->get(SettingsManager::HASHING_START_DELAY, true)) ? DELAYED : PAUSED;
    }

    string path;
    uint64_t bytes = 0;
    size_t files = 0;
    HM->getStats(path, bytes, files);

    if( bytes || files )
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

#if defined(USE_PROGRESS_BARS)
    progressIndicator->hide();
    this->adjustSize();
#else
    progress->setFormat(QString());
#endif

    qtCtx()->dcCtx().getHashManager()->setPriority(Thread::NORMAL);

    timer = new QTimer();
    timer->setInterval(250);

    connect(timer, &QTimer::timeout, this, &HashProgress::timerTick);
    connect(pushButton_START, &QPushButton::clicked, this, &HashProgress::slotStart);
    connect(checkBox, &QCheckBox::toggled, this, &HashProgress::slotAutoClose);

    timer->start();
}

void HashProgress::resetProgress() {
        startBytes = 0;
        startFiles = 0;
        startTime  = 0;
}

HashProgress::~HashProgress(){
    timer->stop();

    delete timer;

    auto& ctx = qtCtx()->dcCtx();
    if (ctx.isRunning() && ctx.getHashManager())
        ctx.getHashManager()->setPriority(Thread::LOW);
}

float HashProgress::getProgress() {
    return static_cast<float>(progress->value())/progress->maximum();
}

void HashProgress::timerTick(){
    auto& ctx = qtCtx()->dcCtx();
    if (!ctx.isRunning()) return;
    auto* HM = ctx.getHashManager();
    auto* SM = ctx.getShareManager();
    if (!HM || !SM) return;

    string path;
    uint64_t bytes = 0;
    size_t files = 0;
    uint64_t tick = GET_TICK();

    stateButton();

    HM->getStats(path, bytes, files);
    if(SM->isRefreshing()) {
        file->setText(tr("Refreshing file list"));
        return;
    }

    if( startTime == 0 )
        startTime = tick;

    if(bytes > startBytes)
        startBytes = bytes;

    if(files > startFiles)
        startFiles = files;

    if(autoClose && !files) {
        accept();

        return;
    }

    const double diff = tick - startTime;
    const bool paused = HM->isHashingPaused();

    QString eta;

    if(startFiles == 0 || startBytes == 0) {
        progress->setValue(0);
    }
    else {
        progress->setValue((10000*(startBytes - bytes))/startBytes);
    }

    if(static_cast<uint64_t>(diff) == 0 || files == 0 || bytes == 0 || paused) {
        status->setText(QString(tr("-.-- files/h, %1 files left")).arg((uint32_t)files));
        speed->setText(tr("-.-- B/s, %1 left").arg(WulforUtil::formatBytes(bytes)));
        eta = tr("-:--:--");

#if !defined(USE_PROGRESS_BARS)
        progressIndicator->setText(tr("%1% %2 left")
                                   .arg(QString::number(0))
                                   .arg(eta));
#endif
    }
    else {
        double filestat = (((double)(startFiles - files)) * 60 * 60 * 1000) / diff;
        double speedStat = (((double)(startBytes - bytes)) * 1000) / diff;

        status->setText(tr("%1 files/h, %2 files left").arg(filestat).arg((uint32_t)files));
        speed->setText(tr("%1/s, %2 left, %3 shared").arg(WulforUtil::formatBytes((int64_t)speedStat))
                                                     .arg(WulforUtil::formatBytes(bytes))
                                                     .arg(WulforUtil::formatBytes(SM->getShareSize())));

        if(/*filestat == 0 ||*/ speedStat == 0) {
            eta = tr("-:--:--");
        }
        else {
            double ss = bytes / speedStat;

            eta = _q(Util::formatSeconds((int64_t)(ss)));
        }

#if defined(USE_PROGRESS_BARS)
        progress->setFormat(tr("%p% %1 left").arg(eta));
#else
        progressIndicator->setText(tr("%1% %2 left")
                                   .arg(QString::number((100*(startBytes - bytes))/startBytes))
                                   .arg(eta));
#endif
    }

    if(!files) {
        //progress->setValue(10000); // generates anoying blinking 0 -> 100%
        file->setText(tr("Done"));
    } else {
        QString fname = QString::fromStdString(path);
        QFontMetrics metrics(font());

        file->setToolTip(fname);

        if (metrics.horizontalAdvance(fname) > file->width()*3/4){
            QStringList parts = fname.split(QDir::separator(), Qt::SkipEmptyParts);

            if (parts.size() > 1){
                QString out = "";

                for (int i = (parts.size()-1); i >= 0; i--){
                    if (metrics.horizontalAdvance(out+parts.at(i)+QDir::separator()) < file->width()*3/4){
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
    auto& ctx = qtCtx()->dcCtx();
    if (!ctx.isRunning()) return;
    ShareManager *SM = ctx.getShareManager();
    HashManager  *HM = ctx.getHashManager();
    if (!SM || !HM) return;
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
