#include <QProcess>
#include <QDebug>
#include <QTextCodec>
#include <QFile>

#include "lib7z.h"

#define tr QObject::tr

bool lib7z::check7z()
{
    QProcess pro;
    pro.start(tr("7z"));
    pro.waitForFinished();
    if (pro.readAllStandardOutput().size() == 0){
        return false;
    } else {
        return true;
    }
}

QStringList lib7z::getIncFiles(QString file)
{
    if (!lib7z::check7z()){
        return QStringList();
    }
    if (!QFile::exists(file)){
        return QStringList();
    }
    QProcess pro;
    pro.start(tr("7z"), QStringList() << tr("l") << file);
    pro.waitForFinished();
    QString tmp = QTextCodec::codecForLocale()->toUnicode(pro.readAllStandardOutput());
    QStringList lines = tmp.split(tr("\n"));
    QStringList rez;
    int x = 0;
    bool found = false;
    while (!found){
        if (lines[x].left(20) == "------------------- "){
            found = true;
        }
        if (lines[x].left(6) == "Error:"){
            return QStringList();
        }
        ++x;
    }
    for (int i = x; i < lines.size()-3; ++i){
        tmp = lines[i].remove(0, 53);
#ifdef Q_WS_WIN
        tmp = tmp.left(tmp.length()-1);
#endif
        rez.append(tmp);
    }
    return rez;
}

bool lib7z::test(QString arch)
{
    if (!lib7z::check7z()){
        return false;
    }
    if (!QFile::exists(arch)){
        return false;
    }
    QProcess pro;
    pro.start(tr("7z"), QStringList() << tr("t") << arch);
    pro.waitForFinished();
    QString tmp = QTextCodec::codecForLocale()->toUnicode(pro.readAllStandardOutput());
    if (tmp.indexOf(tr("Everything is Ok")) == -1){
        return false;
    } else {
        return true;
    }
}

bool lib7z::create7z(lib7z::compress7z compres, QString arch, QStringList files, QString *error)
{
    if (QFile::exists(arch)){
        *error = tr("File already exists");
        return false;
    }
    QProcess pro;
    pro.start(tr("7z"), QStringList() << tr("a") << arch << tr("-ms=off") << tr("-mx=%1").arg(compres) << files);
    pro.waitForFinished();
    QString tmp = QTextCodec::codecForLocale()->toUnicode(pro.readAllStandardOutput());
    if (tmp.indexOf(tr("Everything is Ok")) == -1){
        *error = tr("Error of comress");
        return false;
    } else {
        return true;
    }
}

bool lib7z::extractFile(QString arc, QString name, QString outFile, QString *error)
{
    if (!lib7z::check7z()){
        return false;
    }
    if (!QFile::exists(arc)){
        return false;
    }
    QProcess pro;
    pro.start(tr("7z"), QStringList() << tr("e") << arc << tr("-y") << tr("-so") << name);
    pro.waitForFinished();

    QByteArray bytes = pro.readAllStandardOutput();

    QString tmp = QTextCodec::codecForLocale()->toUnicode(pro.readAllStandardOutput());
    if (tmp.indexOf(tr("No files to process")) != -1){
	*error = tr("Not found file in archive");
	return false;
    }

    if (tmp.indexOf(tr("Everything is Ok")) != -1){
        QFile outf(outFile);
        if (!outf.open(QIODevice::WriteOnly)){
            *error = tr("Can't write file %1").arg(outFile);
            return false;
        }
        outf.write(bytes);
        return true;
    } else {
        *error = tr("Error extract");
        return false;
    }
}

bool lib7z::extractFile(QString arc, QString name, QByteArray *data, QString *error)
{
    if (!lib7z::check7z()){
        return false;
    }
    if (!QFile::exists(arc)){
        return false;
    }
    QProcess pro;
    pro.start(tr("7z"), QStringList() << tr("e") << arc << tr("-y") << tr("-so") << name);
    pro.waitForFinished();

    QByteArray bytes = pro.readAllStandardOutput();

    QString tmp = QTextCodec::codecForLocale()->toUnicode(pro.readAllStandardError());
    if (tmp.indexOf(tr("No files to process")) != -1){
        *error = tr("Not found file in archive");
        return false;
    }

    if (tmp.indexOf(tr("Everything is Ok")) != -1){
        *data = bytes;
        return true;
    } else {
        *error = tr("Error extract");
        return false;
    }
}
