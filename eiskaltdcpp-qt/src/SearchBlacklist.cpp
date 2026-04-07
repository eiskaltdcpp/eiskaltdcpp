/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#include "SearchBlacklist.h"
#include "WulforUtil.h"
#include "QtContext.h"

#include <QFile>
#include <QTextStream>

#include "dcpp/Util.h"

SearchBlacklist::SearchBlacklist(dcpp::DCContext& ctx) : QtContextAware(ctx) {
    list[NAME] = {};
    list[TTH]  = {};

    loadLists();
}

SearchBlacklist::~SearchBlacklist(){
    saveLists();
}

void SearchBlacklist::loadLists(){
    const QString &config = _q(Util::getPath(Util::PATH_USER_CONFIG)) + "searchblacklist";
    QFile f(config);

    if (!(f.exists() && f.open(QIODevice::ReadOnly)))
        return;

    QTextStream stream(&f);
    QString in = "";
    SearchBlacklist::Argument arg = SearchBlacklist::TTH;

    while (!stream.atEnd()){
        in = stream.readLine();

        in.replace("\n", "");

        if (in.startsWith("TTH:") && in.length() > 4){
            in.remove(0, 4);

            arg = SearchBlacklist::TTH;
        }
        else if (in.startsWith("NAME:") && in.length() > 5){
            in.remove(0, 5);

            arg = SearchBlacklist::NAME;
        }
        else
            continue;

        list[arg].push_back(in);
    }

    f.close();
}

void SearchBlacklist::saveLists(){
    const QString &config = _q(Util::getPath(Util::PATH_USER_CONFIG)) + "searchblacklist";
    QFile f(config);

    if (!f.open(QIODevice::WriteOnly))
        return;

    QTextStream stream(&f);

    for (const QString &line : list[NAME])
        stream << "NAME:" << line << "\n";

    for (const QString &line : list[TTH])
        stream << "TTH:" << line << "\n";

    f.close();
}

bool SearchBlacklist::ok(const QString &exp, Argument type){
    const QList<QString> &l = (type == NAME)? list[NAME] : list[TTH];

    for (const QString &str : l){
        QRegularExpression reg_exp(
            QRegularExpression::wildcardToRegularExpression(str),
            QRegularExpression::CaseInsensitiveOption);

        if (reg_exp.match(exp).hasMatch())
            return false;
    }

    return true;
}
