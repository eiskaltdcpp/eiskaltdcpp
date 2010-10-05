#ifndef CUSTOMSETTING_H
#define CUSTOMSETTING_H

#include <QVariant>

enum CustomSettingType{
    IntSetting=0,
    StrSetting,
    BoolSetting
};

void eRegisterCustomSetting(CustomSettingType type, const QString &key, const QVariant &value);

#endif // CUSTOMSETTING_H
