/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "SpellCheck.h"
#include "WulforSettings.h"
#include "dcpp/stdinc.h"
#include "dcpp/Util.h"

#include <QLocale>
#include <QDir>
#include <QtDebug>

SpellCheck::SpellCheck(QObject *parent) :
    QObject(parent),
    spell_checker(NULL)
{
    AspellConfig *config = new_aspell_config();

    aspell_config_replace(config, "encoding", "utf-8");
    aspell_config_replace(config, "personal", (dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG)+"dict").c_str());

#if defined(Q_WS_WIN)
    aspell_config_replace(config, "data-dir", "./aspell/data");
    aspell_config_replace(config, "dict-dir", "./aspell/dict");
#endif

    AspellCanHaveError *ret = new_aspell_speller(config);

    /* config is no longer needed */
    delete_aspell_config(config);

    if (aspell_error(ret)) {
        printf("Error: %s\n", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
    } else {
        spell_checker = to_aspell_speller(ret);
    }
}

SpellCheck::~SpellCheck() {
    if (spell_checker) {
        aspell_speller_save_all_word_lists(spell_checker);

        AspellConfig *config = aspell_speller_config(spell_checker);

        if (config)
            WSSET(WS_APP_ASPELL_LANG, aspell_config_retrieve(config, "lang"));
    }

    delete_aspell_speller(spell_checker);
}

bool SpellCheck::ok(const QString &word) {
    if (!spell_checker || word.isEmpty())
        return true;

    int correct = aspell_speller_check(spell_checker, word.toUtf8().constData(), -1);

    return (correct != 0);
}

void SpellCheck::suggestions(const QString &word, QStringList &list) {
    if (!spell_checker || word.isEmpty())
        return;

    const AspellWordList *suggestions = aspell_speller_suggest(spell_checker, word.toUtf8().constData(), -1);

    if (!suggestions) {
        printf("Error: %s\n", aspell_speller_error_message(spell_checker));
        return;
    }

    AspellStringEnumeration *elements = aspell_word_list_elements(suggestions);

    const char * sugg;
    while (sugg = aspell_string_enumeration_next(elements)) {
        list.append(QString::fromUtf8(sugg, strlen(sugg)));
    }

    delete_aspell_string_enumeration(elements);
}

void SpellCheck::addToDict(const QString &word) {
    if (!spell_checker || word.isEmpty())
        return;

    aspell_speller_add_to_personal(spell_checker, word.toUtf8().constData(), -1);
    aspell_speller_add_to_session(spell_checker, word.toUtf8().constData(), -1);
}
