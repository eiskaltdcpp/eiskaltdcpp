/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QObject>
#include <QList>
#include <QMap>
#include <QMetaType>
#include <QTextStream>
#include <QFile>
#include <QDateTime>

#include "dcpp/stdinc.h"
#include "dcpp/User.h"
#include "dcpp/Singleton.h"

enum AntiSpamObjectState {
    eIN_BLACK = 0,
    eIN_GRAY,
    eIN_WHITE
};

class AntiSpam :
        public QObject,
        public dcpp::Singleton<AntiSpam>
{
    Q_OBJECT

    friend class dcpp::Singleton<AntiSpam>;
public:
    void move(QString, AntiSpamObjectState);

    QList<QString> getBlack();
    QList<QString> getGray();
    QList<QString> getWhite();

    void loadSettings();
    void saveSettings();
    void loadLists();
    void saveLists();

    QString getPhrase() const;
    void setPhrase(QString &phrase);
    QList<QString> getKeys();
    void setKeys(const QList<QString> &keys);

    void setAttempts(int);
    int  getAttempts() const;

    void checkUser(const QString &, const QString &, const QString &);

    friend AntiSpam& operator<<(AntiSpam&, AntiSpamObjectState);
    friend AntiSpam& operator<<(AntiSpam&, const QList<QString>&);
    friend AntiSpam& operator<<(AntiSpam&, const QString&);

public slots:
    bool isInBlack(const QString&) const;
    bool isInWhite(const QString&) const;
    bool isInGray (const QString&) const;
    bool isInAny  (const QString&) const;
    bool isInSandBox(const QString&) const;
    void addToBlack(const QList<QString> &list);
    void addToWhite(const QList<QString> &list);
    void addToGray(const QList<QString> &list);
    void remFromBlack(const QList<QString> &list);
    void remFromWhite(const QList<QString> &list);
    void remFromGray(const QList<QString> &list);
    void clearBlack();
    void clearGray();
    void clearWhite();
    void clearAll();

private:

    AntiSpam();
    virtual ~AntiSpam();

    inline void addToList(QList<QString>&, const QList<QString>&);
    inline void remFromList(QList<QString>&, const QList<QString>&);
    inline void log(const QString &log_msg){ log_stream << QString("[%1] ").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss")) << log_msg << "\n"; }

    void loadBlack();
    void loadWhite();
    void loadGray();
    void saveBlack();
    void saveWhite();
    void saveGray();

    void readFile(QString, QList<QString>&);
    void saveFile(QString, QList<QString>&);

    QList<QString> white_list, black_list, gray_list;

    QString phrase;
    QList<QString> keys;
    QMap< QString, int > sandbox;

    QTextStream log_stream;
    QFile log_file;

    int try_count;

    AntiSpamObjectState state;//used only by operator<<

public slots:

    /** */
    void slotObjectChangeState(QString obj, AntiSpamObjectState from, AntiSpamObjectState to);

};

Q_DECLARE_METATYPE(AntiSpam*)
