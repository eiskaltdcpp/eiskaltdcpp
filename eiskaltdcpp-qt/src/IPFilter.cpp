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

//#define _DEBUG_IPFILTER_

#ifdef _DEBUG_IPFILTER_
#include <QtDebug>
#endif

#include "dcpp/stdinc.h"
#include "dcpp/Util.h"

using namespace dcpp;

const QString signature = "$EISKALTDC IPFILTERLIST$";

IPFilter::IPFilter() {
}

IPFilter::~IPFilter() {
    clearRules(false);
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

QString IPFilter::Uint32ToString(quint32 ip){
    return QString("%1.%2.%3.%4").arg((ip & 0xFF000000) >> 24)
                                 .arg((ip & 0x00FF0000) >> 16)
                                 .arg((ip & 0x0000FF00) >> 8)
                                 .arg((ip & 0x000000FF));
}

quint32 IPFilter::MaskToCIDR(quint32 mask){
#ifdef _DEBUG_IPFILTER_
    printf("IPFilter::MaskToCIDR(%x)\n", mask);
#endif
    if (mask == 0)
        return 0;
    else if (mask == 0xFFFFFFFF)
        return 32;

    quint32 shift_mask = 0x00000001;

    int j = 0;

    for (; ((mask & shift_mask) == 0) && (j <= 32); j++, shift_mask <<= 1);

    j = 32 - j;

    return j;
}

quint32 IPFilter::MaskForBits(quint32 bits){
    bits = 32 - (bits > 32?32:bits);

    quint32 mask = 0xFFFFFFFF;
    quint32 bit  = 0xFFFFFFFE;

    for (quint32 i = 0; i < bits; i++, bit <<= 1)
        mask &= bit;

    return mask;
}

bool IPFilter::ParseString(QString exp, quint32 &ip, quint32 &mask, eTableAction &act){
    if (exp.isEmpty())
        return false;

    if (exp.indexOf("/0") >= 0){
        if (exp.indexOf("!") == 0)
            act = etaDROP;
        else
            act = etaACPT;

        ip = mask = 0x00000000;

        return true;
    }

    QString str_ip = "", str_mask = "";

    if (exp.indexOf("/") > 0){
        int c_pos = exp.indexOf("/");

        str_ip = exp.left(c_pos);
        str_mask = exp.mid(c_pos+1,exp.length()-c_pos-1);
    }
    else
        str_ip = exp;

    if (str_ip.indexOf("!") == 0){
        act = etaDROP;
        str_ip.replace("!", "");
    }
    else
        act = etaACPT;

    if (!isIP(str_ip))
        return false;

    if (str_mask != "")
        mask = MaskForBits(str_mask.toULong() > 32?32:str_mask.toULong());
    else
        mask = MaskForBits(32);

    ip = StringToUint32(str_ip);

    return true;
}

void IPFilter::addToRules(QString exp, eDIRECTION direction) {
    quint32 exp_ip, exp_mask;
    eTableAction act;

    if (!ParseString(exp, exp_ip, exp_mask, act))
        return;

    IPFilterElem *el = NULL;

    if (list_ip.contains(exp_ip)) {
#ifdef _DEBUG_IPFILTER_
    qDebug() << "\tIP already in list";
#endif
        QIPHash::const_iterator it = list_ip.find(exp_ip);

        while (it != list_ip.end() && it.key() == exp_ip){
            el = it.value();

            if ((el->direction != direction) && (el->action == act)){
#ifdef _DEBUG_IPFILTER_
                qDebug() << "\tChange direction of IP";
#endif
                emit ruleChanged(exp, el->direction, eDIRECTION_BOTH, act);

                el->direction = eDIRECTION_BOTH;

                return;
            }
            else if (el->direction == direction && el->action == act)
                return;

            ++it;
        }
    }

    el = new IPFilterElem;

    el->mask      = exp_mask;
    el->ip        = exp_ip;
    el->direction = direction;
    el->action    = act;

#ifdef _DEBUG_IPFILTER_
    qDebug() << "\tCreated new element:";
    printf("\t\tMASK: 0x%x\n"
           "\t\tIP  : %s\n"
           "\t\tD   : %i\n"
           "\t\tA   : %i\n",
           el->mask, IPFilter::Uint32ToString(el->ip).toAscii().constData(),
           (int)el->direction, (int)el->action
          );
#endif

    list_ip.insert(el->ip, el);
    rules.push_back(el);

    emit ruleAdded(exp, direction);
}

void IPFilter::remFromRules(QString exp, eTableAction act) {

    QString str_ip;
    quint32 exp_ip;

    if (exp.indexOf("/") > 0)
        str_ip = exp.left(exp.indexOf("/"));

    if (!isIP(str_ip))
        return;

    exp_ip = IPFilter::StringToUint32(str_ip);

    if (!list_ip.contains(exp_ip))
        return;

    QIPHash::const_iterator it = list_ip.find(exp_ip);
    IPFilterElem *el;

    while (it != list_ip.end() && it.key() == exp_ip){
        el = it.value();

        if (!el)//i know that it's impossible..
            return;

        if (el->action == act){
            list_ip.remove(exp_ip, el);

            if (rules.contains(el))
                rules.removeAt(rules.indexOf(el));

            emit ruleRemoved(exp, el->direction, el->action);

            delete el;

            return;
        }

        ++ it;
    }
}

void IPFilter::changeRuleDirection(QString exp, eDIRECTION direction, eTableAction act) {
    if (exp.indexOf("/") > 0)
        exp = exp.left(exp.indexOf("/"));

    if (!isIP(exp))
        return;

    quint32 exp_ip = IPFilter::StringToUint32(exp);
    QIPHash::const_iterator it = list_ip.find(exp_ip);

    while (it != list_ip.end() && it.key() == exp_ip){
        IPFilterElem *el = it.value();

        if (!el)
            return;

        if (el->action == act){
            emit ruleChanged(exp, el->direction, direction, act);

            el->direction = direction;

            return;
        }

        ++it;
    }
}

bool IPFilter::OK(const QString &exp, eDIRECTION direction){
#ifdef _DEBUG_IPFILTER_
    qDebug() << "IPFilter::OK(" << exp.toAscii().constData() << ", " << (int)direction << ")";
#endif
    QString str_src(exp);

    if (str_src.indexOf(":") > 0)
        str_src = str_src.left(exp.indexOf(":")); //XXX.XXX.XXX.XXX:PORT -> XXX.XXX.XXX.XXX

    if (!isIP(str_src))
        return false;

    quint32 src = IPFilter::StringToUint32(str_src);
    IPFilterElem *el;

    for (int i = 0; i < rules.size(); i++){
        el = rules.at(i);

#ifdef _DEBUG_IPFILTER_
    printf("\tel->ip & el->mask == %x\n"
           "\tsrc    & el->mask == %x\n",
           (el->ip & el->mask),
           (src    & el->mask)
          );
#endif

        if ((el->ip & el->mask) == (src & el->mask)){//Exact match
            bool exact_direction = ((el->direction == direction) || (el->direction == eDIRECTION_BOTH));
#ifdef _DEBUG_IPFILTER_
            printf("\tFound match... ");
#endif
            if      ((el->action == etaDROP) && exact_direction){
#ifdef _DEBUG_IPFILTER_
                printf("DROP.\n");
#endif
                return false;
            }
            else if ((el->action == etaACPT) && exact_direction){
#ifdef _DEBUG_IPFILTER_
                printf("ACCEPT.\n");
#endif
                return true;
            }
#ifdef _DEBUG_IPFILTER_
            else
                printf("IGNORE.\n");
#endif
        }
    }

    return true;
}

void IPFilter::step(quint32 ip, eTableAction act, bool down){
    IPFilterElem *el = NULL;

    QIPHash::const_iterator it = list_ip.find(ip);

    while (it != list_ip.end() && it.key() == ip){
        if (it.value()->action == act){
            el = it.value();

            break;
        }

        ++it;
    }

    if (!el)
        return;

    int index = rules.indexOf(el);
    int control = (down?(rules.size()-1):0);
    int inc = (down?1:-1);

    if ((index == control) || (index < 0))
        return;

    int new_index = index+inc;
    IPFilterElem *old_el = rules.at(new_index);

    rules[index]    = old_el;
    rules[new_index]= el;
}

void IPFilter::moveRuleUp(quint32 ip, eTableAction act){
    step(ip, act, false);
}

void IPFilter::moveRuleDown(quint32 ip, eTableAction act){
    step(ip, act, true);
}

bool IPFilter::isIP(QString &exp) {
    return (QRegExp("^(\\d{1,3}.){3,3}\\d{1,3}$").exactMatch(exp));
}

void IPFilter::loadList() {
    QFile file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "ipfilter");

    if (!file.exists())
        return;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    if (!list_ip.empty())
        clearRules();

    QTextStream in(&file);

    while (!in.atEnd()) {
        QString pattern = in.readLine();
        QString str_ip, str_mask = "";
        eDIRECTION direction = eDIRECTION_IN;

        pattern.replace("\n", "");
        pattern.replace(" ", "");

#ifdef _DEBUG_IPFILTER_
        qDebug() << pattern;
#endif

        if (pattern.indexOf("|D_IN|:") == 0){
            pattern = pattern.right(pattern.length() - 7);//delete "|D_IN|:"
        }
        else if (pattern.toUpper().indexOf("|D_OUT|:") == 0) {//delete "|D_OUT|:"
            pattern = pattern.right(pattern.length() - 8);
            direction = eDIRECTION_OUT;
        }
        else if (pattern.toUpper().indexOf("|D_BOTH|:") == 0) {//delete "|D_BOTH|:"
            pattern = pattern.right(pattern.length() - 9);
            direction = eDIRECTION_BOTH;
        } else
            continue;

#ifdef _DEBUG_IPFILTER_
        qDebug() << pattern;
#endif

        addToRules(pattern, direction);
    }

#ifdef _DEBUG_IPFILTER_
    qDebug() << rules;
#endif

    file.close();
}

void IPFilter::saveList(){
    QFile file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "ipfilter");
    if (!file.isOpen() && !file.open(QIODevice::WriteOnly|QIODevice::Text))
        return;

    QTextStream out(&file);

    out << signature << "\n";

    out.flush();

    for (int i = 0; i < rules.size(); i++) {
        QString prefix;
        IPFilterElem *el = rules.at(i);

        eDIRECTION direction = el->direction;
        eTableAction act = el->action;

        prefix = (direction == eDIRECTION_IN?"|D_IN|:":(direction == eDIRECTION_OUT?"|D_OUT|:":"|D_BOTH|:"));
        prefix += QString(act == etaACPT?"":"!");

        out << prefix + IPFilter::Uint32ToString(el->ip) + "/" + QString().setNum(IPFilter::MaskToCIDR(el->mask)) + "\n";

        out.flush();
    }

    file.close();
}

void IPFilter::exportTo(QString path) {
    QFile file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "ipfilter");
    QMessageBox msgBox;

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);

    saveList();

    if (!file.exists()) {
        msgBox.setText(tr("Nothing to export."));
        msgBox.exec();

        return;
    }

    QFile exp_file(path);

    if (exp_file.exists())
        exp_file.remove();

    if (!file.copy(path)) {
        msgBox.setText(tr("Unable to export settings."));
        msgBox.exec();

        return;
    }
}

void IPFilter::importFrom(QString path) {
    QFile file(path);
    QMessageBox msgBox;

    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.setIcon(QMessageBox::Warning);

    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        msgBox.setText(tr("Nothing to import."));
        msgBox.exec();
        file.close();

        return;
    }

    QTextStream in(&file);
    QString sign = in.readLine();

    sign.replace("\n", "");

    file.close();

    if (sign == signature) {
        QFile old_file(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "ipfilter");
        old_file.remove();

        file.copy(QString::fromStdString(Util::getPath(Util::PATH_USER_CONFIG)) + "ipfilter");

        clearRules();

        loadList();
    } else {
        msgBox.setText(tr("Invalid signature."));
        msgBox.exec();
    }
}

const QIPList &IPFilter::getRules() {
    return rules;
}

const QIPHash &IPFilter::getHash() {
    return list_ip;
}

void IPFilter::clearRules(bool emit_signal) {
    QIPHash::const_iterator it = list_ip.constBegin();

    while (!list_ip.empty() && it != list_ip.constEnd()){
        if (it.value())
            delete it.value();

        ++it;
    }

    list_ip.clear();
    rules.clear();

    if (emit_signal)
        emit ruleRemoved("*", eDIRECTION_BOTH, etaACPT);
}
