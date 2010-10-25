/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "HashManagerScript.h"
#include "WulforUtil.h"

#include "dcpp/MerkleTree.h"

HashManagerScript::HashManagerScript(QObject *parent) :
    QObject(parent)
{
    HM = dcpp::HashManager::getInstance();

    HM->addListener(this);
}

HashManagerScript::~HashManagerScript(){
    HM->removeListener(this);
}

void HashManagerScript::stopHashing(const QString &baseDir) {
    HM->stopHashing(_tq(baseDir));
}

QString HashManagerScript::getTTH(const QString &aFileName, quint64 size) const{
    dcpp::TTHValue val = HM->getTTH(_tq(aFileName), size);

    return _q(val.toBase32());
}

QString HashManagerScript::getTTH(const QString &aFileName) const{
    const dcpp::TTHValue *v = HM->getFileTTHif(_tq(aFileName));

    if (v)
        return _q(v->toBase32());
    else
        return "";
}

void HashManagerScript::rebuild(){
    HM->rebuild();
}

void HashManagerScript::startup(){
    HM->startup();
}

void HashManagerScript::shutdown() {
    HM->shutdown();
}

bool HashManagerScript::pauseHashing() const {
    return HM->pauseHashing();
}

void HashManagerScript::resumeHashing() {
    return HM->resumeHashing();
}

bool HashManagerScript::isHashingPaused() const {
    return HM->isHashingPaused();
}

void HashManagerScript::on(TTHDone, const dcpp::string &file, const dcpp::TTHValue &val) throw() {
    emit done(_q(file), _q(val.toBase32()));
}
