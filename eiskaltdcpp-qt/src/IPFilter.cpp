/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "IPFilter.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>

#ifdef _DEBUG_IPFILTER
#include <QtDebug>
#endif

#include "dcpp/stdinc.h"
#include "dcpp/Util.h"

using namespace dcpp;

const QString signature = "$EISKALTDC IPFILTERLIST$";

IPFilter::IPFilter() {
    if (!ipfilter::getInstance()){
        ipfilter::newInstance();
    }
}

IPFilter::~IPFilter() {
}

quint32 IPFilter::StringToUint32(const QString& ip){
    quint32 ret = ip.section('.',0,0).toULong();
    ret <<= 8;
    ret |= ip.section('.',1,1).toULong();
    ret <<= 8;
    ret |= ip.section('.',2,2).toULong();
    ret <<= 8;
    ret |= ip.section('.',3,3).toULong();

    return ret;
}

QString IPFilter::Uint32ToString(const quint32 ip){
    return QString("%1.%2.%3.%4").arg((ip & 0xFF000000) >> 24)
                                 .arg((ip & 0x00FF0000) >> 16)
                                 .arg((ip & 0x0000FF00) >> 8)
                                 .arg((ip & 0x000000FF));
}

quint32 IPFilter::MaskToCIDR(const quint32 mask){
#ifdef _DEBUG_IPFILTER
    printf("IPFilter::MaskToCIDR(%x)\n", mask);
#endif

    return ipfilter::getInstance()->MaskToCIDR(mask);
}

quint32 IPFilter::MaskForBits(quint32 bits) {

    return ipfilter::getInstance()->MaskForBits(bits);
}

bool IPFilter::ParseString(const QString &exp, quint32 &ip, quint32 &mask, eTableAction &act){

    return ipfilter::getInstance()->ParseString(exp.toStdString(), ip, mask, act);
}

void IPFilter::addToRules(const QString &exp, const eDIRECTION direction) {

    ipfilter::getInstance()->addToRules(exp.toStdString(), direction);

    emit ruleAdded(exp, direction);
}

void IPFilter::remFromRules(const QString &exp, const eTableAction act) {

    return ipfilter::getInstance()->remFromRules(exp.toStdString(), act);
}

void IPFilter::changeRuleDirection(QString exp, const eDIRECTION direction, const eTableAction act) {

    ipfilter::getInstance()->changeRuleDirection(exp.toStdString(), direction, act);
}

bool IPFilter::OK(const QString &exp, eDIRECTION direction){

    return ipfilter::getInstance()->OK(exp.toStdString(), direction);
}

void IPFilter::step(quint32 ip, eTableAction act, bool down){

    ipfilter::getInstance()->step(ip, act, down);
}

void IPFilter::moveRuleUp(quint32 ip, eTableAction act){
    step(ip, act, false);
}

void IPFilter::moveRuleDown(quint32 ip, eTableAction act){
    step(ip, act, true);
}

bool IPFilter::isIP(const QString &exp) {
    return (QRegExp("^(\\d{1,3}.){3,3}\\d{1,3}$").exactMatch(exp));
}

void IPFilter::loadList() {

    ipfilter::getInstance()->loadList();
}

void IPFilter::saveList(){

    ipfilter::getInstance()->saveList();
}

void IPFilter::exportTo(QString path, std::string &error) {
    QFile file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "ipfilter");
    QMessageBox msgBox;

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);

    error = "";
    ipfilter::getInstance()->exportTo(path.toStdString(), error);

    if (error.length() > 0){
        msgBox.setText(tr(error.c_str()));
        msgBox.exec();
    }
}

void IPFilter::importFrom(QString path, std::string &error) {
    QFile file(path);
    QMessageBox msgBox;

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);

    error = "";
    ipfilter::getInstance()->importFrom(path.toStdString(), error);

    if (error.length() > 0){
        msgBox.setText(tr(error.c_str()));
        msgBox.exec();
    }
}

const QIPList IPFilter::getRules() {
    IPList list  = ipfilter::getInstance()->getRules();
    QIPList res;
    for(auto & item : list){
        res.append(item);
    }
    return res;

}

const QIPHash IPFilter::getHash() {

    IPHash hash = ipfilter::getInstance()->getHash();
    QIPHash res;

    for(auto & item : hash){
        res.insert(item.first, item.second);
    }
    return res;
}

void IPFilter::clearRules(const bool emit_signal) {
    ipfilter::getInstance()->clearRules();

    if (emit_signal)
        emit ruleRemoved("*", eDIRECTION_BOTH, etaACPT);
}
