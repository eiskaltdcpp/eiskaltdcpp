#ifndef SPELLCHECK_H
#define SPELLCHECK_H

#include <QObject>
#include <QList>
#include <QStringList>

#include <aspell.h>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"

class SpellCheck :
        public QObject,
        public dcpp::Singleton<SpellCheck>
{
Q_OBJECT
friend class dcpp::Singleton<SpellCheck>;

public:
    bool ok(const QString &word);
    void suggestions(const QString &word, QStringList &list);
    void addToDict(const QString &word);

private:
    SpellCheck(QObject *parent = 0);
    ~SpellCheck();

    AspellConfig *config;
    AspellSpeller *spell_checker;
};

#endif // SPELLCHECK_H
