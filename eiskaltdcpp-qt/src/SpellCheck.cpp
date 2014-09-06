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
    spell_checker_main(NULL),
    spell_checker_en(NULL)
{
    AspellConfig *config = new_aspell_config();

    aspell_config_replace(config, "encoding", "utf-8");
    aspell_config_replace(config, "personal", (dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG)+"dict").c_str());

#if defined(Q_WS_WIN)
    aspell_config_replace(config, "data-dir", "./aspell/data");
    aspell_config_replace(config, "dict-dir", "./aspell/dict");
#endif

    AspellCanHaveError *ret = new_aspell_speller(config);

    QString lang_main = aspell_config_retrieve(config, "lang");
    bool mainSpellIsEn = lang_main == "en";
    AspellConfig * config_sub = NULL;
    AspellCanHaveError * ret_sub = NULL;
    if (!mainSpellIsEn) {
        config_sub = aspell_config_clone(config);
        aspell_config_replace(config_sub, "lang","en");
        aspell_config_replace(config_sub, "personal", (dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG)+"dict_en").c_str());
        ret_sub = new_aspell_speller(config_sub);
        delete_aspell_config(config_sub);
    }

    // config is no longer needed
    delete_aspell_config(config);

    if (aspell_error(ret)) {
        printf("Main: Error: %s\n", aspell_error_message(ret));fflush(stdout);
        delete_aspell_can_have_error(ret);
    } else {
        spell_checker_main = to_aspell_speller(ret);
    }
    if (!mainSpellIsEn) {
        if (aspell_error(ret_sub)) {
            printf("Sub: Error: %s\n", aspell_error_message(ret_sub));fflush(stdout);
            delete_aspell_can_have_error(ret_sub);
        } else {
            spell_checker_en = to_aspell_speller(ret_sub);
        }
    }

}

SpellCheck::~SpellCheck() {

    if (spell_checker_main) {
        aspell_speller_save_all_word_lists(spell_checker_main);

        AspellConfig *config = aspell_speller_config(spell_checker_main);

        if (config)
            WSSET(WS_APP_ASPELL_LANG, aspell_config_retrieve(config, "lang"));
    }
    if (spell_checker_en) {
        aspell_speller_save_all_word_lists(spell_checker_en);
    }

    delete_aspell_speller(spell_checker_main);
    delete_aspell_speller(spell_checker_en);
}

bool SpellCheck::ok(const QString &word) {
    if (!spell_checker_main || word.isEmpty())
        return true;

    int correct = aspell_speller_check(spell_checker_main, word.toUtf8().constData(), -1);
    if (correct == 0 && spell_checker_en) {
        correct = aspell_speller_check(spell_checker_en, word.toUtf8().constData(), -1);
    }

    return (correct != 0);
}

void SpellCheck::suggestions(const QString &word, QStringList &list, QStringList &list_en) {
    if (!spell_checker_main || word.isEmpty())
        return;

    const AspellWordList *suggestions = aspell_speller_suggest(spell_checker_main, word.toUtf8().constData(), -1);
    const AspellWordList *suggestions_en = NULL;
    if (spell_checker_en) {
        bool isAscii = true;
        for (int i=0; i< word.size(); ++i) {
             isAscii = isAscii && (word.at(i).unicode() <= 127);
        }
        if (isAscii) suggestions_en = aspell_speller_suggest(spell_checker_en, word.toUtf8().constData(), -1);
    }
    if (!suggestions) {
        printf("Main: Error: %s\n", aspell_speller_error_message(spell_checker_main));fflush(stdout);
    } else {
        AspellStringEnumeration *elements = aspell_word_list_elements(suggestions);

        const char * sugg;
        while ((sugg = aspell_string_enumeration_next(elements)) != NULL) {
            list.append(QString::fromUtf8(sugg, strlen(sugg)));
        }

        delete_aspell_string_enumeration(elements);
    }
    if (spell_checker_en) {
        if (!suggestions_en){
            printf("Sub: Error: %s\n", aspell_speller_error_message(spell_checker_en));fflush(stdout);
        } else {
            AspellStringEnumeration *elements_en = aspell_word_list_elements(suggestions_en);

            const char * sugg;
            while ((sugg = aspell_string_enumeration_next(elements_en)) != NULL ) {
                list_en.append(QString::fromUtf8(sugg, strlen(sugg)));
            }

            delete_aspell_string_enumeration(elements_en);
        }
    }
    if (!suggestions_en && !suggestions)  return;
}

void SpellCheck::addToDict(const QString& word) {
    if (!spell_checker_main || word.isEmpty())
        return;

    bool isAscii = false;
    if (spell_checker_en) {
        isAscii = true;
        for (int i=0; i< word.size(); ++i) {
             isAscii = isAscii && (word.at(i).unicode() <= 127);
        }
    }

    if (spell_checker_en && isAscii) {
        aspell_speller_add_to_personal(spell_checker_en, word.toUtf8().constData(), -1);
        aspell_speller_add_to_session(spell_checker_en, word.toUtf8().constData(), -1);
    } else {
        aspell_speller_add_to_personal(spell_checker_main, word.toUtf8().constData(), -1);
        aspell_speller_add_to_session(spell_checker_main, word.toUtf8().constData(), -1);
    }
}
