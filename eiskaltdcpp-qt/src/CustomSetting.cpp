#include "CustomSetting.h"
#include "WulforSettings.h"

void eRegisterCustomSetting(CustomSettingType type, const QString &key, const QVariant &value){
    if (!WulforSettings::getInstance())
        return;

    switch (type){
    case IntSetting:
    case BoolSetting:
    {
        if ((value.type() != QVariant::Int) || (value.type() != QVariant::Bool))
            break;

        QMap<QString, int> &map = WulforSettings::getInstance()->intmap;

        if (!WulforSettings::getInstance()->hasKey(key))
            map.insert(key, qvariant_cast<int>(value));

        break;
    }
    case StrSetting:
    {
        if (value.type() != QVariant::String)
            break;

        QMap<QString, QString> &map = WulforSettings::getInstance()->strmap;

        if (!WulforSettings::getInstance()->hasKey(key))
            map.insert(key, qvariant_cast<QString>(value));

        break;
    }
    }
}
