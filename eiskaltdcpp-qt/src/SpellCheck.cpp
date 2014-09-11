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
    QObject(parent)
{
    AspellConfig * config;
    AspellDictInfoList * dlist;
    AspellDictInfoEnumeration * dels;
    const AspellDictInfo * entry;

    config = new_aspell_config();

    /* the returned pointer should _not_ need to be deleted */
    dlist = get_aspell_dict_info_list(config);

    dels = aspell_dict_info_list_elements(dlist);

    while ( (entry = aspell_dict_info_enumeration_next(dels)) != 0)
    {
        if ((::strcmp(entry->name, entry->code) == 0) && !spellChekers.contains(QString(entry->name).mid(0,2))) {
            aspell_config_replace(config, "lang", entry->name);
            aspell_config_replace(config, "encoding", "utf-8");
            aspell_config_replace(config, "personal", (dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG)+"dict_"+ entry->name).c_str());
            #if defined(Q_OS_WIN)
                aspell_config_replace(config, "data-dir", "./aspell/data");
                aspell_config_replace(config, "dict-dir", "./aspell/dict");
            #endif
             AspellCanHaveError *ret = new_aspell_speller(config);
            if (aspell_error(ret)) {
                printf("Error: %s\n", aspell_error_message(ret));fflush(stdout);
                delete_aspell_can_have_error(ret);
            } else {
                AspellSpeller *spell_checker = to_aspell_speller(ret);
                spellChekers.insert(QString(entry->name), spell_checker);
            }
        }
    }
    delete_aspell_config(config);
}

SpellCheck::~SpellCheck() {

    auto it = spellChekers.constBegin();
    while (it != spellChekers.constEnd()) {
        AspellSpeller *speller = it.value();
        if (speller) aspell_speller_save_all_word_lists(speller);
        delete_aspell_speller(speller);
        ++it;
    }
}

bool SpellCheck::ok(const QString &word) {
    if (word.isEmpty() || spellChekers.isEmpty()) {
        return true;
    }

    int correct = 0;
    auto it = spellChekers.constBegin();
    while (it != spellChekers.constEnd()) {
        AspellSpeller *speller = it.value();
        if (speller) correct = aspell_speller_check(speller, word.toUtf8().constData(), -1);
        if (correct > 0) {
            break;
        }
        ++it;
    }
    return (correct != 0);
}

void SpellCheck::suggestions(const QString &word, QMap<QString, QStringList> &listMap/*, QStringList &list_en*/) {
    if (word.isEmpty() || spellChekers.isEmpty()) {
        return;
    }

    auto it = spellChekers.constBegin();
    while (it != spellChekers.constEnd()) {
        AspellSpeller *speller = it.value();
        const AspellWordList *suggestions = aspell_speller_suggest(speller, word.toUtf8().constData(), -1);
        if (!suggestions) {
            printf("Error: %s\n", aspell_speller_error_message(speller));fflush(stdout);
        } else {
            QStringList list;
            AspellStringEnumeration *elements = aspell_word_list_elements(suggestions);

            const char * sugg;
            while ((sugg = aspell_string_enumeration_next(elements)) != NULL) {
                list.append(QString::fromUtf8(sugg, strlen(sugg)));
            }

            delete_aspell_string_enumeration(elements);
            listMap.insert(it.key(),list);
        }
        ++it;
    }

}

void SpellCheck::addToDict(const QString& word, const QString& lang) {
    if (word.isEmpty() || spellChekers.isEmpty())
        return;

    if (spellChekers.contains(lang)) {
        aspell_speller_add_to_personal(spellChekers.value(lang), word.toUtf8().constData(), -1);
        aspell_speller_add_to_session(spellChekers.value(lang), word.toUtf8().constData(), -1);
    }

}
