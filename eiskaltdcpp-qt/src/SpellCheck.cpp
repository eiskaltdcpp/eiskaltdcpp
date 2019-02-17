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
#include "WulforUtil.h"
#include "dcpp/stdinc.h"
#include "dcpp/Util.h"

#include <QLocale>
#include <QDir>
#include <QtDebug>

SpellCheck::SpellCheck(QObject *parent) :
    QObject(parent),
    spell_checker(nullptr)
{
    setLanguage(QString());
}

SpellCheck::~SpellCheck()
{
    deleteSpeller();
}

void SpellCheck::deleteSpeller()
{
    if (spell_checker) {
        aspell_speller_save_all_word_lists(spell_checker);
        delete_aspell_speller(spell_checker);
        spell_checker = nullptr;
    }
}

void SpellCheck::loadAspellConfig(AspellConfig * const config)
{
    AspellCanHaveError *ret = new_aspell_speller(config);

    if (aspell_error(ret)) {
        printf("Error: %s\n", aspell_error_message(ret));
        delete_aspell_can_have_error(ret);
    } else {
        deleteSpeller();
        spell_checker = to_aspell_speller(ret);
    }
}

AspellConfig *SpellCheck::defaultAspellConfig()
{
    AspellConfig *config = new_aspell_config();

    aspell_config_replace(config, "encoding", "utf-8");
    aspell_config_replace(config, "personal", (dcpp::Util::getPath(dcpp::Util::PATH_USER_CONFIG)+"dict").c_str());

    if (WulforSettings::getInstance()) {
        aspell_config_replace(config, "lang", WSGET(WS_APP_ASPELL_LANG, "en").toUtf8().constData());
    }

#if defined(Q_OS_WIN) || defined(Q_OS_MAC) || defined(LOCAL_ASPELL_DATA)
    const QString aspellDataPath = WulforUtil::getInstance()->getAspellDataPath();
    aspell_config_replace(config, "data-dir",
                          QByteArray(aspellDataPath.toUtf8() + "/data").constData());
    aspell_config_replace(config, "dict-dir",
                          QByteArray(aspellDataPath.toUtf8() + "/dict").constData());
#endif

    return config;
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

    const char *sugg = aspell_string_enumeration_next(elements);
    for (; sugg; sugg = aspell_string_enumeration_next(elements)) {
        list.append(QString::fromUtf8(sugg, strlen(sugg)));
    }

    delete_aspell_string_enumeration(elements);
}

void SpellCheck::setLanguage(const QString &lang)
{
    AspellConfig *config = defaultAspellConfig();

    if (!lang.isEmpty()) {
        aspell_config_replace(config, "lang", lang.toUtf8().constData());
    }

    loadAspellConfig(config);
    delete_aspell_config(config);
}

void SpellCheck::addToDict(const QString &word) {
    if (!spell_checker || word.isEmpty())
        return;

    aspell_speller_add_to_personal(spell_checker, word.toUtf8().constData(), -1);
    aspell_speller_add_to_session(spell_checker, word.toUtf8().constData(), -1);
}
