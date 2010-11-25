/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef DCANTISPAM_H
#define DCANTISPAM_H

//#include <QObject>
//#include <std::vector>
//#include <std::map>
//#include <QMetaType>
//#include <QTextStream>
//#include <QFile>
//#include <QDateTime>
#include <string>
#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"
#include "dcpp/Singleton.h"

enum AntiSpamObjectState {
    eIN_BLACK = 0,
    eIN_GRAY,
    eIN_WHITE
};

class AntiSpam :
        //public QObject,
        public dcpp::Singleton<AntiSpam>
{
    //Q_OBJECT

    friend class dcpp::Singleton<AntiSpam>;
public:
    void move(std::string, AntiSpamObjectState);

    std::vector<std::string> getBlack();
    std::vector<std::string> getGray();
    std::vector<std::string> getWhite();

    void loadSettings();
    void saveSettings();
    void loadLists();
    void saveLists();

    std::string getPhrase() const;
    void setPhrase(std::string &phrase);
    std::vector<std::string> getKeys();
    void setKeys(const std::vector<std::string> &keys);

    void setAttempts(int);
    int  getAttempts() const;

    void checkUser(const std::string &, const std::string &, const std::string &);

    friend AntiSpam& operator<<(AntiSpam&, AntiSpamObjectState);
    friend AntiSpam& operator<<(AntiSpam&, const std::vector<std::string>&);
    friend AntiSpam& operator<<(AntiSpam&, const std::string&);

public slots:
    bool isInBlack(const std::string&) const;
    bool isInWhite(const std::string&) const;
    bool isInGray (const std::string&) const;
    bool isInAny  (const std::string&) const;
    bool isInSandBox(const std::string&) const;
    void addToBlack(const std::vector<std::string> &list);
    void addToWhite(const std::vector<std::string> &list);
    void addToGray(const std::vector<std::string> &list);
    void remFromBlack(const std::vector<std::string> &list);
    void remFromWhite(const std::vector<std::string> &list);
    void remFromGray(const std::vector<std::string> &list);
    void clearBlack();
    void clearGray();
    void clearWhite();
    void clearAll();

private:

    AntiSpam();
    virtual ~AntiSpam();

    inline void addToList(std::vector<std::string>&, const std::vector<std::string>&);
    inline void remFromList(std::vector<std::string>&, const std::vector<std::string>&);
    inline void log(const std::string &log_msg){ log_stream << std::string("[%1] ").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss")) << log_msg << "\n"; }

    void loadBlack();
    void loadWhite();
    void loadGray();
    void saveBlack();
    void saveWhite();
    void saveGray();

    void readFile(std::string, std::vector<std::string>&);
    void saveFile(std::string, std::vector<std::string>&);

    std::vector<std::string> white_list, black_list, gray_list;

    std::string phrase;
    std::vector<std::string> keys;
    std::map< std::string, int > sandbox;

    QTextStream log_stream;
    QFile log_file;

    int try_count;

    AntiSpamObjectState state;//used only by operator<<

public slots:

    /** */
    void slotObjectChangeState(std::string obj, AntiSpamObjectState from, AntiSpamObjectState to);

};

//Q_DECLARE_METATYPE(AntiSpam*)

#endif // DCANTISPAM_H
