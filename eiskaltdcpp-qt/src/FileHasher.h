/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QDialog>
#include <QEvent>
#include <QThread>

#include "dcpp/stdinc.h"

#include "ui_UIFileHasher.h"

class HashThread: public QThread{
    Q_OBJECT

    public:
        HashThread();
        virtual ~HashThread();

        void run();

        void setFile(const QString &);
        QString getHash();

    protected:
        void calculate_tth();
    private:
        QString file_name, hash;
};

class FileHasher :
        public QDialog,
        private Ui::UIFileHasher
{
Q_OBJECT
public:
    explicit FileHasher(QWidget *parent = nullptr);
    virtual ~FileHasher();

    private slots:
        void saveWindowSize();
        void slotStart();
        void slotBrowse();
        void slotCopyMagnet();
        void slotCopySearchString();
        void slotDone();
        void search();
        void search(const QString &, const qulonglong &, const QString &);
        void searchTTH(const QString &);
        void searchFile(const QString &);
    private:
        HashThread *hasher;
};
