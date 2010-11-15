/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef FILEHASHER_H
#define FILEHASHER_H


#include <QDialog>
#include <QEvent>
#include <QThread>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"

#include "ui_UIFileHasher.h"

class HashThread: public QThread{
    Q_OBJECT

    public:
        HashThread();
        virtual ~HashThread();

        void run();

        void setFile(QString);
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
    explicit FileHasher(QWidget *parent = 0);
    virtual ~FileHasher();

    private slots:
        void slotStart();
        void slotBrowse();
        void slotMagnet();
        void slotDone();
    private:
        HashThread *hasher;
};

#endif // FILEHASHER_H

