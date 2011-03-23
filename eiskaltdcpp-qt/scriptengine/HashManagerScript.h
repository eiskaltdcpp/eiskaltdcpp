/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef HASHMANAGERSCRIPT_H
#define HASHMANAGERSCRIPT_H

#include <QObject>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/HashManager.h"
#include "dcpp/HashManagerListener.h"
#include "dcpp/HashValue.h"

class HashManagerScript :
        public QObject,
        public dcpp::Singleton<HashManagerScript>,
        public dcpp::HashManagerListener
{
Q_OBJECT
friend class dcpp::Singleton<HashManagerScript>;

public Q_SLOTS:
    void stopHashing(const QString &baseDir);
    QString getTTH(const QString &aFileName, quint64 size) const;
    QString getTTH(const QString &aFileName) const;
    void rebuild();
    void startup();
    void shutdown();
    bool pauseHashing() const;
    void resumeHashing();
    bool isHashingPaused() const;

Q_SIGNALS:
    void done(const QString &file, const QString &tth);

protected:
    virtual void on(TTHDone, const dcpp::string& , const dcpp::TTHValue&) throw();

private:
    HashManagerScript(QObject *parent = 0);
    HashManagerScript(const HashManagerScript&){}
    ~HashManagerScript();
    HashManagerScript &operator=(const HashManagerScript&){ return *this; }

    dcpp::HashManager *HM;
};

#endif // HASHMANAGERSCRIPT_H
