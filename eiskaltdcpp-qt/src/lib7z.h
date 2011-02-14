/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef LIB7Z_H
#define LIB7Z_H

#include <QString>
#include <QStringList>
#include <QByteArray>

class lib7z
{
public:
    enum compress7z{
        without = 0,
        verylow = 1,
        low = 3,
        standart = 5,
        high = 7,
        veryhigh = 9
    };

    static bool check7z();
    static QStringList getIncFiles(QString file);
    static bool test(QString arch);
    static bool create7z(lib7z::compress7z compres, QString arch, QStringList files, QString *error);
    static bool extractFile(QString arc, QString name, QString outFile, QString *error);
    static bool extractFile(QString arc, QString name, QByteArray *data, QString *error);
private:
    lib7z(){};
};

#endif // LIB7Z_H
