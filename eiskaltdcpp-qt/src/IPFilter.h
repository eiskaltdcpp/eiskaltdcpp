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
#include <QRegExp>
#include <QHash>
#include <QFile>

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"

#ifdef _DEBUG_
#include <QtDebug>
#endif

enum eDIRECTION {
    eDIRECTION_IN = 0,
    eDIRECTION_OUT,
    eDIRECTION_BOTH
};

enum eTableAction{
    etaDROP=0,
    etaACPT
};

typedef struct _IPFilterElem{
    quint32 ip;
    quint32 mask;

    eDIRECTION direction;
    eTableAction action;
} IPFilterElem;

typedef QMultiHash<quint32, IPFilterElem*> QIPHash;
typedef QList<IPFilterElem*> QIPList;

class IPFilter :
        public QObject,
        public dcpp::Singleton<IPFilter>
{
    Q_OBJECT

    friend class dcpp::Singleton<IPFilter>;

public:
    /** */
    static quint32 StringToUint32(const QString&);
    /** */
    static QString Uint32ToString(quint32);
    /** */
    static quint32 MaskToCIDR(quint32);
    /** */
    static quint32 MaskForBits(quint32);
    /** */
    static bool ParseString(QString, quint32&, quint32&, eTableAction&);
    /** */
    static bool isIP(QString &exp);

    /** */
    void loadList();
    /** */
    void saveList();

    /** */
    const QIPList &getRules();
    /** */
    const QIPHash &getHash ();

    /** */
    void addToRules(QString exp, eDIRECTION direction);
    /** */
    void remFromRules(QString exp, eTableAction);
    /** */
    void changeRuleDirection(QString exp, eDIRECTION, eTableAction);
    /** */
    void clearRules(bool emit_signal = true);

    /** */
    void moveRuleUp(quint32, eTableAction);
    /** */
    void moveRuleDown(quint32, eTableAction);

    /** */
    bool OK(const QString &exp, eDIRECTION direction);

    /** */
    void exportTo(QString path);
    /** */
    void importFrom(QString path);

#ifdef _DEBUG_
    void printHash();
#endif

private:
    /** */
    IPFilter();
    /** */
    virtual ~IPFilter();

    /** */
    void step(quint32, eTableAction, bool down = true);
    /** */
    QIPHash list_ip;
    /** */
    QIPList rules;

signals:
    void ruleAdded(QString, eDIRECTION);
    void ruleRemoved(QString, eDIRECTION, eTableAction);
    void ruleChanged(QString, eDIRECTION, eDIRECTION, eTableAction);
};
