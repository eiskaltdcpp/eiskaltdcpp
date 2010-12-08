/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QObject>
#include <QList>
#include <QRegExp>
#include <QString>

#include <boost/noncopyable.hpp>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class SearchBlacklist:
        public QObject,
        public dcpp::Singleton<SearchBlacklist>,
        public boost::noncopyable
{
    Q_OBJECT

    friend class dcpp::Singleton<SearchBlacklist>;

public:
    enum Argument {
        NAME,
        TTH
    };

    bool ok(const QString &exp, Argument type);
    QList<QString> getList(Argument arg) const { return (list[arg]); }
    void setList(Argument arg, QList<QString> l) { list[arg] = l; }

private:
    SearchBlacklist();
    virtual ~SearchBlacklist();

    void loadLists();
    void saveLists();

    QList<QString> list[2];
};
