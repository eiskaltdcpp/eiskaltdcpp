#include "SpellCheck.h"
#include "WulforSettings.h"

#include <QLocale>
#include <QDir>

SpellCheck::SpellCheck(QObject *parent) :
    QObject(parent),
    config(NULL),
    spell_checker(NULL)
{
    config = new_aspell_config();

    aspell_config_replace(config, "encoding", "utf-8");
    aspell_config_replace(config, "personal", (QDir::homePath()+QDir::separator()+".eiskaltdc++"+QDir::separator()+"dict").toAscii().constData());

    AspellCanHaveError *error = new_aspell_speller(config);

    if (aspell_error(error) != 0){
        delete_aspell_config(config);

        config = NULL;
    }
    else
        spell_checker = to_aspell_speller(error);

    const AspellDictInfoList *dicts = get_aspell_dict_info_list(config);
    AspellDictInfoEnumeration *enumer = aspell_dict_info_list_elements(dicts);
    const AspellDictInfo *info = NULL;

    QStringList all;

    while ((info = aspell_dict_info_enumeration_next(enumer)) != NULL)
        all.append(QString::fromUtf8(info->code, strlen(info->code)));

    if (WSGET(WS_APP_ASPELL_LANG).isEmpty()){
        QString lc_prefix = QLocale::system().name();

        if (all.contains(lc_prefix))//Loading dictionary from system locale
            aspell_config_replace(config, "lang", lc_prefix.toAscii().constData());
        else if (all.contains(lc_prefix.left(lc_prefix.indexOf("_")))) {
            aspell_config_replace(config, "lang", lc_prefix.left(lc_prefix.indexOf("_")).toAscii().constData());
        }
    }
    else
        aspell_config_replace(config, "lang", WSGET(WS_APP_ASPELL_LANG).toAscii().constData());
}

SpellCheck::~SpellCheck(){
    if (spell_checker)
        aspell_speller_save_all_word_lists(spell_checker);

    delete_aspell_config(config);
    delete_aspell_speller(spell_checker);
}

bool SpellCheck::ok(const QString &word){
    if (!spell_checker || word.isEmpty())
        return true;

    int correct = aspell_speller_check(spell_checker, word.toAscii().constData(), -1);

    return (correct != 0);
}

void SpellCheck::suggestions(const QString &word, QStringList &list){
    if (!spell_checker || word.isEmpty())
        return;

    const AspellWordList *suggestions = aspell_speller_suggest(spell_checker, word.toUtf8().constData(), word.length());
    AspellStringEnumeration *elements = aspell_word_list_elements(suggestions);

    const char * sugg;
    while ((sugg = aspell_string_enumeration_next(elements)) != NULL ){
        list.append(QString::fromUtf8(sugg, strlen(sugg)));
    }

    delete_aspell_string_enumeration(elements);
}

void SpellCheck::addToDict(const QString &word){
    if (!spell_checker || word.isEmpty())
        return;

    aspell_speller_add_to_personal(spell_checker, word.toUtf8().constData(), -1);
    aspell_speller_add_to_session(spell_checker, word.toUtf8().constData(), -1);
}
