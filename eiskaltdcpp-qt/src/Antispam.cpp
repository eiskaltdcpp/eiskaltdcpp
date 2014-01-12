/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QDir>
#include <QFile>
#include <QTextStream>

#include "Antispam.h"
#include "WulforUtil.h"
#include "dcpp/stdinc.h"
#include "dcpp/Util.h"
#include "dcpp/ClientManager.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"

using namespace dcpp;

AntiSpam& operator<<(AntiSpam &sp, AntiSpamObjectState st){
    sp.state = st;

    return sp;
}

AntiSpam& operator<<(AntiSpam &sp, const QList<QString> &list){
    switch (sp.state){
        case eIN_BLACK:
            sp.addToBlack(list);
            break;
        case eIN_GRAY:
            sp.addToGray(list);
            break;
        default:
            sp.addToWhite(list);
            break;
    }

    return sp;
}

AntiSpam& operator <<(AntiSpam &sp, const QString &list){
    QString users = list;
    users += ",";

    sp << users.split(",", QString::SkipEmptyParts);

    return sp;
}

AntiSpam::AntiSpam():
        state(eIN_BLACK)
{
    try_count = 0;

    log_file.setFileName(_q(Util::getPath(Util::PATH_USER_CONFIG)) + "antispam.log");

    if (log_file.open(QIODevice::WriteOnly))
        log_stream.setDevice(&log_file);
}

AntiSpam::~AntiSpam() {
    gray_list.clear();
    white_list.clear();
    black_list.clear();

    log_file.close();
}

void AntiSpam::slotObjectChangeState(QString obj, AntiSpamObjectState from, AntiSpamObjectState to) {
    if (from == to)
        return;

    if ((from == eIN_BLACK && !black_list.contains(obj)) ||
            (from == eIN_WHITE && !white_list.contains(obj)) ||
            (from == eIN_GRAY && !gray_list.contains(obj))) {
        return;
    }

    QList<QString> *from_list = &white_list;
    QList<QString> *to_list = &black_list;

    if (from != eIN_WHITE) {
        if (from == eIN_BLACK)
            from_list = &black_list;
        else
            from_list = &gray_list;
    }\

    if (to != eIN_BLACK) {
        if (to == eIN_WHITE)
            to_list = &white_list;
        else
            to_list = &gray_list;
    }

    from_list->removeAll(obj);

    if (to_list->contains(obj)) {//just remove from source list
        return;
    }

    to_list->append(obj);

}

bool AntiSpam::isInAny(const QString &obj) const {
    return ( isInBlack(obj) || isInGray(obj) || isInWhite(obj));
}

bool AntiSpam::isInBlack(const QString &obj) const {
    return (black_list.contains(obj));
}

bool AntiSpam::isInGray(const QString &obj) const {
    return ( gray_list.contains(obj) || WBGET(WB_ANTISPAM_AS_FILTER));
}

bool AntiSpam::isInWhite(const QString &obj) const {
    return ( white_list.contains(obj) || WBGET(WB_ANTISPAM_AS_FILTER));
}

bool AntiSpam::isInSandBox(const QString &obj_cid) const {
    return sandbox.contains(obj_cid);
}

void AntiSpam::checkUser(const QString &cid, const QString &msg, const QString &hubUrl){
    UserPtr user = ClientManager::getInstance()->findUser(CID(_tq(cid)));

    if (!user->isOnline()){
        if (sandbox.contains(cid))
            sandbox.remove(cid);

        return;
    }

    log(tr("Checking user %1 (message: %2, cid: %3)...").arg(WulforUtil::getInstance()->getNicks(cid)).arg(msg).arg(cid));

    if (sandbox.contains(cid)){
        int counter = sandbox[cid];
        ++counter;

        QList<QString> keys = getKeys();

        for (auto key : keys){
            if (key.toUpper() == msg.toUpper()){
                (*this) << eIN_GRAY << WulforUtil::getInstance()->getNicks(cid);
                log(tr("%1: Moving user to GRAY.").arg(cid));

                sandbox.remove(cid);

                return;
            }
        }

        if (counter > try_count){
            (*this) << eIN_BLACK << WulforUtil::getInstance()->getNicks(cid);
            log(tr("%1: Moving user to BLACK.").arg(cid));

            sandbox.remove(cid);

            return;
        }

        ClientManager::getInstance()->privateMessage(HintedUser(user, _tq(hubUrl)), _tq("Try again."), false);
        log(tr("%1: Sending \"Try again\" message.").arg(cid));

        sandbox[cid] = counter;
    }
    else {
        sandbox[cid] = 0;

        QString question = tr("Hi, this is AntiSpam bot. So question is \"%1\"").arg(phrase);

        ClientManager::getInstance()->privateMessage(HintedUser(user, _tq(hubUrl)), _tq(question), false);
    }
}

void AntiSpam::move(QString obj, AntiSpamObjectState state) {
    if (isInAny(obj)) {
        AntiSpamObjectState from;

        if (isInBlack(obj))
            from = eIN_BLACK;
        else if (isInGray(obj))
            from = eIN_GRAY;
        else
            from = eIN_WHITE;

        slotObjectChangeState(obj, from, state);
    } else {
        if (state == eIN_BLACK)
            black_list << obj;
        else if (state == eIN_GRAY)
            gray_list << obj;
        else
            white_list << obj;
    }
}

inline void AntiSpam::addToList(QList<QString> &source, const QList<QString> &dest) {
    for (int i = 0; i < dest.size(); i++) {
        if (!source.contains(dest.at(i)))
            source << dest.at(i);
    }
}

inline void AntiSpam::remFromList(QList<QString> &from, const QList<QString> &what) {
    for (int i = 0; i < what.size(); i++) {
        int index = from.indexOf(what.at(i));

        if (index != -1)
            from.removeAt(index);
    }
}

void AntiSpam::addToBlack(const QList<QString> &list) {
    addToList(black_list, list);
}

void AntiSpam::addToGray(const QList<QString> &list) {
    addToList(gray_list, list);
}

void AntiSpam::addToWhite(const QList<QString> &list) {
    addToList(white_list, list);
}

void AntiSpam::remFromBlack(const QList<QString> &list) {
    remFromList(black_list, list);
}

void AntiSpam::remFromGray(const QList<QString> &list) {
    remFromList(gray_list, list);
}

void AntiSpam::remFromWhite(const QList<QString> &list) {
    remFromList(white_list, list);
}

QList<QString> AntiSpam::getBlack() {
    return black_list;
}

QList<QString> AntiSpam::getGray() {
    return gray_list;
}

QList<QString> AntiSpam::getWhite() {
    return white_list;
}

void AntiSpam::loadLists() {
    if (!gray_list.empty() ||
            !black_list.empty() ||
            !white_list.empty()) {
        return;
    }
    loadBlack();
    loadWhite();
    loadGray();
}

void AntiSpam::saveLists() {
    saveBlack();
    saveGray();
    saveWhite();
}

void AntiSpam::loadBlack() {
    readFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "blacklist", black_list);
}

void AntiSpam::loadGray() {
    readFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "graylist", gray_list);
}

void AntiSpam::loadWhite() {
    readFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "whitelist", white_list);
}

void AntiSpam::saveBlack() {
    saveFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "blacklist", black_list);
}

void AntiSpam::saveGray() {
    saveFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "graylist", gray_list);
}

void AntiSpam::saveWhite() {
    saveFile(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "whitelist", white_list);
}

void AntiSpam::readFile(QString path, QList<QString> &list) {
    if (!QFile::exists(path))
        return;

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();

        line.replace("\n", "");

        if (line == "")
            continue;

        list << line;
    }

    file.close();
}

void AntiSpam::saveFile(QString path, QList<QString> &list) {
    QFile file(path);

    if (!file.open(QIODevice::WriteOnly))
        return;

    QTextStream out(&file);

    for (int i = 0; i < list.size(); i++)
        out << list.at(i) << "\n";

    file.close();
}

void AntiSpam::loadSettings() {
    QFile file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "antispam");

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    keys.append("10");
        phrase = "5+5=?";

        return;
    }

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine();

        if (line.indexOf("|ANTISPAM_PHRASE|") != -1) {
            line = line.right(line.length() - 17);
            line.replace("\n", "");

            if (line == "")
                phrase = "5+5=?";
            else
                phrase = line;
    } else if (line.indexOf("|ANTISPAM_KEY|") != -1) {
            line = line.right(line.length() - 14);
            line.replace("\n", "");

            if (line == "")
        keys.append("10");
        else {
            QList<QString> words = line.split("|", QString::SkipEmptyParts);

            if (!keys.empty())
                keys.clear();

            keys.append(words);
        }
        } else if (line.indexOf("|ATTEMPTS|") != -1){
            line = line.right(line.length() - 10);
            line.replace("\n", "");

            bool ok = false;

            if (line == "")
                try_count = 0;
            else
                try_count = line.toInt(&ok, 10);

            if (!ok)
                try_count = 0;
        }
    }

    file.close();
}

void AntiSpam::saveSettings() {
    QFile file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "antispam");

    if (!file.open(QIODevice::WriteOnly))
        return;

    QTextStream out(&file);

    out << "|ANTISPAM_PHRASE|" << phrase << "\n";

    QString words = "";

    for (int i = 0; i < keys.size(); i++)
        words += keys.at(i).toUpper() + "|";

    out << "|ANTISPAM_KEY|" << words << "\n";
    out << "|ATTEMPTS|" << QString().setNum(try_count, 10) << "\n";

    file.close();
}

void AntiSpam::setPhrase(QString &phrase) {
    if (phrase == "") {
        this->phrase = (phrase = "5+5=?");
    } else
        this->phrase = phrase;
}

void AntiSpam::setKeys(const QList<QString> &keys) {
    if (keys.empty())
        return;

    this->keys.clear();
    this->keys.append(keys);
}

QList<QString> AntiSpam::getKeys() {
    return keys;
}

QString AntiSpam::getPhrase() const {
    return phrase;
}

void AntiSpam::clearAll() {
    clearBlack();
    clearGray();
    clearWhite();
}

void AntiSpam::clearBlack() {
    black_list.clear();
}

void AntiSpam::clearGray() {
    gray_list.clear();
}

void AntiSpam::clearWhite() {
    white_list.clear();
}

void AntiSpam::setAttempts(int i){
    try_count = i < 0?try_count:i;
}

int AntiSpam::getAttempts() const{
    return try_count;
}
