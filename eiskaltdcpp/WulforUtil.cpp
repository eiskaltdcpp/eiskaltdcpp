#include "MainWindow.h"
#include "Magnet.h"
#include "WulforUtil.h"
#include "icons/gv.xpm"

#ifdef HAVE_IFADDRS_H
#include <sys/types.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <fcntl.h>
#endif

#include "dcpp/ClientManager.h"
#include "dcpp/Util.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>

#ifndef CLIENT_DATA_DIR
#define CLIENT_DATA_DIR ""
#endif

#ifndef CLIENT_ICONS_DIR
#define CLIENT_ICONS_DIR ""
#endif

#ifndef CLIENT_TRANSLATIONS_DIR
#define CLIENT_TRANSLATIONS_DIR ""
#endif

#ifndef CLIENT_SOUNDS_DIR
#define CLIENT_SOUNDS_DIR ""
#endif

const QString WulforUtil::magnetSignature = "magnet:?xt=urn:tree:tiger:";

WulforUtil::WulforUtil()
{
    memset(userIconCache, 0, sizeof (userIconCache));

    userIcons = new QImage();

    connectionSpeeds["0.005"]   = 0;
    connectionSpeeds["0.01"]    = 0;
    connectionSpeeds["0.02"]    = 0;
    connectionSpeeds["0.05"]    = 0;
    connectionSpeeds["0.1"]     = 0;
    connectionSpeeds["0.2"]     = 0;
    connectionSpeeds["0.5"]     = 0;
    connectionSpeeds["1"]       = 3;
    connectionSpeeds["2"]       = 5;
    connectionSpeeds["5"]       = 5;
    connectionSpeeds["10"]      = 5;
    connectionSpeeds["20"]      = 5;
    connectionSpeeds["50"]      = 5;
    connectionSpeeds["100"]     = 5;
    connectionSpeeds["1000"]    = 6;

    QtEnc2DCEnc["CP949"]        = "CP949 (Korean)";
    QtEnc2DCEnc["GB18030-0"]    = "GB18030 (Chinese)";
    QtEnc2DCEnc["ISO 8859-2"]   = "ISO-8859-2 (Central Europe)";
    QtEnc2DCEnc["ISO 8859-7"]   = "ISO-8859-7 (Greek)";
    QtEnc2DCEnc["ISO 8859-8"]   = "ISO-8859-8 (Hebrew)";
    QtEnc2DCEnc["ISO 8859-9"]   = "ISO-8859-9 (Turkish)";
    QtEnc2DCEnc["ISO 2022-JP"]  = "ISO-2022-JP (Japanese)";
    QtEnc2DCEnc["KOI8-R"]       = "KOI8-R (Cyrillic)";
    QtEnc2DCEnc["UTF-8"]        = "UTF-8 (Unicode)";
    QtEnc2DCEnc["TIS-620"]      = "TIS-620 (Thai)";
    QtEnc2DCEnc["WINDOWS-1250"] = "CP1252 (Central Europe)";
    QtEnc2DCEnc["WINDOWS-1251"] = "CP1251 (Cyrillic)";
    QtEnc2DCEnc["WINDOWS-1252"] = "CP1252 (Western Europe)";
    QtEnc2DCEnc["WINDOWS-1256"] = "CP1256 (Arabic)";
    QtEnc2DCEnc["WINDOWS-1257"] = "CP1257 (Baltic)";

    bin_path = QApplication::applicationDirPath();

    if (!bin_path.endsWith(PATH_SEPARATOR))
        bin_path += PATH_SEPARATOR_STR;

    initFileTypes();
}

WulforUtil::~WulforUtil(){
    delete userIcons;

    clearUserIconCache();
}

bool WulforUtil::loadUserIcons(){
    //Try to find icons directory
    QString settings_path = QDir::currentPath() + "/icons/user/default";
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return loadUserIconsFromFile(settings_path + PATH_SEPARATOR_STR + QString("usericons.png"));

    settings_path = bin_path + "icons/user/default";
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return loadUserIconsFromFile(settings_path + PATH_SEPARATOR_STR + QString("usericons.png"));

    settings_path = QDir::homePath()+QString(".dc/icons/user/default");
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return loadUserIconsFromFile(settings_path + PATH_SEPARATOR_STR + QString("usericons.png"));

    settings_path = CLIENT_ICONS_DIR "/user/default";
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return loadUserIconsFromFile(settings_path + PATH_SEPARATOR_STR + QString("usericons.png"));

    return false;
}

QString WulforUtil::findAppIconsPath(){
    //Try to find icons directory
    QString settings_path = QDir::currentPath() + "/icons/appl/default";
    settings_path = QDir::toNativeSeparators(settings_path);

    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = bin_path + "/appl/default";
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = QDir::homePath()+QString(".dc/icons/appl/default");
    settings_path = QDir::toNativeSeparators(settings_path);

    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = CLIENT_ICONS_DIR "/appl/default";
    settings_path = QDir::toNativeSeparators(settings_path);

    if (QDir(settings_path).exists())
        return settings_path;

    return "";
}

bool WulforUtil::loadUserIconsFromFile(QString file){
    if (userIcons->load(file, "PNG")){
        clearUserIconCache();

        return true;
    }

    return false;
}

void WulforUtil::clearUserIconCache(){
    for (int x = 0; x < USERLIST_XPM_COLUMNS; ++x) {
        for (int y = 0; y < USERLIST_XPM_ROWS; ++y) {
            if (userIconCache[x][y]) {
                delete userIconCache[x][y];
                userIconCache[x][y] = 0;
            }
        }
    }
}

QPixmap *WulforUtil::getUserIcon(const UserPtr &id, bool isAway, bool isOp, const QString &sp){
    int x = connectionSpeeds[sp];
    int y = 0;

    if (isAway)
        y += 1;

    if (id->isSet(User::TLS))
        y += 2;

    if (id->isSet(User::DCPLUSPLUS))
        y += 4;

    if (isOp)
        y += 8;

    if (id->isSet(User::PASSIVE))
        y += 16;

    if (userIconCache[x][y] == 0) {
        userIconCache[x][y] = new QPixmap(
                QPixmap::fromImage(
                                    userIcons->copy(
                                                    x * USERLIST_ICON_SIZE,
                                                    y * USERLIST_ICON_SIZE,
                                                    USERLIST_ICON_SIZE,
                                                    USERLIST_ICON_SIZE
                                                   )
                                    )
                );
    }

    return userIconCache[x][y];
}

bool WulforUtil::loadIcons(){
    m_bError = false;

    app_icons_path = findAppIconsPath();

    m_PixmapMap[eiBALL_GREEN] = loadPixmap("ball_green.png");
    m_PixmapMap[eiBOOKMARK_ADD] = loadPixmap("bookmark_add.png");
    m_PixmapMap[eiCHAT] = loadPixmap("chat.png");
    m_PixmapMap[eiCLEAR] = loadPixmap("clear.png");
    m_PixmapMap[eiCONFIGURE] = loadPixmap("configure.png");
    m_PixmapMap[eiCONNECT] = loadPixmap("connect.png");
    m_PixmapMap[eiCONNECT_NO] = loadPixmap("connect_no.png");
    m_PixmapMap[eiDOWN] = loadPixmap("down.png");
    m_PixmapMap[eiDOWNLOAD] = loadPixmap("download.png");
    m_PixmapMap[eiDOWNLOAD_AS] = loadPixmap("download_as.png");
    m_PixmapMap[eiEDIT] = loadPixmap("edit.png");
    m_PixmapMap[eiEDITADD] = loadPixmap("editadd.png");
    m_PixmapMap[eiEDITCOPY] = loadPixmap("editcopy.png");
    m_PixmapMap[eiEDITDELETE] = loadPixmap("editdelete.png");
    m_PixmapMap[eiEXIT] = loadPixmap("exit.png");
    m_PixmapMap[eiFILECLOSE] = loadPixmap("fileclose.png");
    m_PixmapMap[eiFILEFIND] = loadPixmap("filefind.png");
    m_PixmapMap[eiFILTER] = loadPixmap("filter.png");
    m_PixmapMap[eiFOLDER_BLUE] = loadPixmap("folder_blue.png");
    m_PixmapMap[eiGUI] = loadPixmap("gui.png");
    m_PixmapMap[eiGV] = QPixmap(gv_xpm);
    m_PixmapMap[eiHASHING] = loadPixmap("hashing.png");
    m_PixmapMap[eiICON_APPL] = loadPixmap("icon_appl.png");
    m_PixmapMap[eiMESSAGE] = loadPixmap("message.png");
    m_PixmapMap[eiOWN_FILELIST] = loadPixmap("own_filelist.png");
    m_PixmapMap[eiRECONNECT] = loadPixmap("reconnect.png");
    m_PixmapMap[eiRELOAD] = loadPixmap("reload.png");
    m_PixmapMap[eiSERVER] = loadPixmap("server.png");
    m_PixmapMap[eiSPAM] = loadPixmap("spam.png");
    m_PixmapMap[eiSPLASH] = loadPixmap("splash.png");
    m_PixmapMap[eiTRANSFER] = loadPixmap("transfer.png");
    m_PixmapMap[eiUP] = loadPixmap("up.png");
    m_PixmapMap[eiUSERS] = loadPixmap("users.png");
    m_PixmapMap[eiZOOM_IN] = loadPixmap("zoom-in.png");
    m_PixmapMap[eiZOOM_OUT] = loadPixmap("zoom-out.png");

    m_PixmapMap[eiFILETYPE_APPLICATION] = loadPixmap("filetype-application.png");
    m_PixmapMap[eiFILETYPE_ARCHIVE] = loadPixmap("filetype-archive.png");
    m_PixmapMap[eiFILETYPE_DOCUMENT] = loadPixmap("filetype-document.png");
    m_PixmapMap[eiFILETYPE_MP3] = loadPixmap("filetype-audio.png");
    m_PixmapMap[eiFILETYPE_PICTURE] = loadPixmap("filetype-picture.png");
    m_PixmapMap[eiFILETYPE_UNKNOWN] = loadPixmap("filetype-unknown.png");
    m_PixmapMap[eiFILETYPE_VIDEO] = loadPixmap("filetype-video.png");

    m_PixmapMap[eiFLAG_GB] = loadPixmap("flag_gb.png");
    m_PixmapMap[eiFLAG_RU] = loadPixmap("flag_ru.png");

    return !m_bError;
}

QPixmap WulforUtil::loadPixmap(const QString &file){
    QString f;
    QPixmap p;

    f = app_icons_path + "/" + file;
    f = QDir::toNativeSeparators(f);

    if (p.load(f))
        return p;

    printf("IconLoader::LoadPixmap: Can't load '%s'\n", file.toAscii().constData());

    m_bError = true;

    p = QPixmap(gv_xpm);

    return p;
}

QPixmap &WulforUtil::getPixmap(enum WulforUtil::Icons e){
    return m_PixmapMap[e];
}

QString WulforUtil::getNicks(const QString &cid){
    return getNicks(CID(cid.toStdString()));
}

QString WulforUtil::getNicks(const CID &cid){
    return QString::fromStdString(Util::toString(ClientManager::getInstance()->getNicks(cid)));
}

void WulforUtil::textToHtml(QString &str, bool print){
    if (print){
        str.replace(";", "&#59;");
        str.replace("<", "&lt;");
        str.replace(">", "&gt;");
    }
    else {
        str.replace("\n", "<br/>");
        str.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
    }
}

void WulforUtil::initFileTypes(){
    m_FileTypeMap.clear();

    // MP3 2
    m_FileTypeMap["A52"]  = eiFILETYPE_MP3;
    m_FileTypeMap["AAC"]  = eiFILETYPE_MP3;
    m_FileTypeMap["AC3"]  = eiFILETYPE_MP3;
    m_FileTypeMap["APE"]  = eiFILETYPE_MP3;
    m_FileTypeMap["AIFF"] = eiFILETYPE_MP3;
    m_FileTypeMap["AU"]   = eiFILETYPE_MP3;
    m_FileTypeMap["DTS"]  = eiFILETYPE_MP3;
    m_FileTypeMap["FLA"]  = eiFILETYPE_MP3;
    m_FileTypeMap["FLAC"] = eiFILETYPE_MP3;
    m_FileTypeMap["MID"]  = eiFILETYPE_MP3;
    m_FileTypeMap["MOD"]  = eiFILETYPE_MP3;
    m_FileTypeMap["M4A"]  = eiFILETYPE_MP3;
    m_FileTypeMap["M4P"]  = eiFILETYPE_MP3;
    m_FileTypeMap["MPC"]  = eiFILETYPE_MP3;
    m_FileTypeMap["MP1"]  = eiFILETYPE_MP3;
    m_FileTypeMap["MP2"]  = eiFILETYPE_MP3;
    m_FileTypeMap["MP3"]  = eiFILETYPE_MP3;
    m_FileTypeMap["OGG"]  = eiFILETYPE_MP3;
    m_FileTypeMap["RA"]   = eiFILETYPE_MP3;
    m_FileTypeMap["SHN"]  = eiFILETYPE_MP3;
    m_FileTypeMap["SPX"]  = eiFILETYPE_MP3;
    m_FileTypeMap["WAV"]  = eiFILETYPE_MP3;
    m_FileTypeMap["WMA"]  = eiFILETYPE_MP3;
    m_FileTypeMap["WV"]   = eiFILETYPE_MP3;

        // ARCHIVE 3
    m_FileTypeMap["7Z"]  = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["ACE"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["BZ2"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["CAB"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["EX_"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["GZ"]  = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["LZH"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["RAR"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["RPM"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["TAR"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["TGZ"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["ZIP"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["ZOO"] = eiFILETYPE_ARCHIVE;
    m_FileTypeMap["Z"]   = eiFILETYPE_ARCHIVE;

        // DOCUMENT 4
    m_FileTypeMap["CFG"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["CONF"]  = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["CPP"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["CSS"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["C"]     = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["DIZ"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["DOC"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["H"]     = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["HLP"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["HTM"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["HTML"]  = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["INI"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["INF"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["LOG"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["NFO"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["ODG"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["ODP"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["ODS"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["ODT"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["PDF"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["PHP"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["PPT"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["PS"]    = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["PDF"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["SHTML"] = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["SXC"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["SXD"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["SXI"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["SXW"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["TXT"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["RFT"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["RDF"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["XML"]   = eiFILETYPE_DOCUMENT;
    m_FileTypeMap["XLS"]   = eiFILETYPE_DOCUMENT;

        // APPL 5
    m_FileTypeMap["BAT"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["CGI"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["COM"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["DLL"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["EXE"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["HQX"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["JS"]  = eiFILETYPE_APPLICATION;
    m_FileTypeMap["SH"]  = eiFILETYPE_APPLICATION;
    m_FileTypeMap["SO"]  = eiFILETYPE_APPLICATION;
    m_FileTypeMap["SYS"] = eiFILETYPE_APPLICATION;
    m_FileTypeMap["VXD"] = eiFILETYPE_APPLICATION;

        // PICTURE 6
    m_FileTypeMap["3DS"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["A11"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["ACB"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["ADC"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["ADI"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["AFI"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["AI"]   = eiFILETYPE_PICTURE;
    m_FileTypeMap["AIS"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["ANS"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["ART"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["B8"]   = eiFILETYPE_PICTURE;
    m_FileTypeMap["BMP"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["CBM"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["EPS"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["GIF"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["ICO"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["IMG"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["JPEG"] = eiFILETYPE_PICTURE;
    m_FileTypeMap["JPG"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["PCT"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["PCX"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["PIC"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["PICT"] = eiFILETYPE_PICTURE;
    m_FileTypeMap["PNG"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["PS"]   = eiFILETYPE_PICTURE;
    m_FileTypeMap["PSP"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["RLE"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["TGA"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["TIF"]  = eiFILETYPE_PICTURE;
    m_FileTypeMap["TIFF"] = eiFILETYPE_PICTURE;
    m_FileTypeMap["XPM"]  = eiFILETYPE_PICTURE;

    // VIDEO 7
    m_FileTypeMap["AVI"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["ASF"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["ASX"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["DAT"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["DIVX"]  = eiFILETYPE_VIDEO;
    m_FileTypeMap["DV"]    = eiFILETYPE_VIDEO;
    m_FileTypeMap["FLV"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["M1V"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["M2V"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["M4V"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["MKV"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["MOV"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["MOVIE"] = eiFILETYPE_VIDEO;
    m_FileTypeMap["MP4"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["MPEG"]  = eiFILETYPE_VIDEO;
    m_FileTypeMap["MPEG1"] = eiFILETYPE_VIDEO;
    m_FileTypeMap["MPEG2"] = eiFILETYPE_VIDEO;
    m_FileTypeMap["MPEG4"] = eiFILETYPE_VIDEO;
    m_FileTypeMap["MPG"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["OGM"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["PXP"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["QT"]    = eiFILETYPE_VIDEO;
    m_FileTypeMap["RM"]    = eiFILETYPE_VIDEO;
    m_FileTypeMap["RMVB"]  = eiFILETYPE_VIDEO;
    m_FileTypeMap["VIV"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["VOB"]   = eiFILETYPE_VIDEO;
    m_FileTypeMap["WMV"]   = eiFILETYPE_VIDEO;
}

QPixmap WulforUtil::getPixmapForFile(const QString &file){
    QString ext = QFileInfo(file).suffix().toUpper();

    if (m_FileTypeMap.contains(ext))
        return getPixmap(m_FileTypeMap[ext]);
    else
        return getPixmap(eiFILETYPE_UNKNOWN);
}

QString WulforUtil::qtEnc2DcEnc(QString name){
    if (QtEnc2DCEnc.contains(name))
        return QtEnc2DCEnc[name].left(QtEnc2DCEnc[name].indexOf(" "));
    else
        return "";
}

QString WulforUtil::dcEnc2QtEnc(QString name){
    QMap<QString, QString>::iterator it = QtEnc2DCEnc.begin();

    for (; it != QtEnc2DCEnc.end(); ++it){
        if (it.value() == name || it.value().indexOf(name) == 0)
            return it.key();
    }

    return tr("System default");
}

QStringList WulforUtil::encodings(){
    QStringList encs;

    QMap<QString, QString>::iterator it = QtEnc2DCEnc.begin();

    for (; it != QtEnc2DCEnc.end(); ++it)
        encs << it.key();

    return encs;
}

QTextCodec *WulforUtil::codecForEncoding(QString name){
    if (!QtEnc2DCEnc.contains(name))
        return QTextCodec::codecForLocale();

    return QTextCodec::codecForName(name.toAscii());
}

void WulforUtil::openUrl(const QString &url){
    if (url.startsWith("http://") || url.startsWith("www.") || url.startsWith(("ftp://"))){
        QDesktopServices::openUrl(QUrl::fromEncoded(url.toAscii()));
    }
    else if (url.startsWith("adc://") || url.startsWith("adcs://")){
        MainWindow::getInstance()->newHubFrame(url, "UTF-8");
    }
    else if (url.startsWith("dchub://")){
        MainWindow::getInstance()->newHubFrame(url, WSGET(WS_DEFAULT_LOCALE));
    }
    else if (url.startsWith("magnet:")){
        QString magnet = url;

        Magnet *m = new Magnet(MainWindow::getInstance());

        m->setLink(magnet);
        m->setModal(true);

        m->exec();

        delete m;
    }
}

bool WulforUtil::getUserCommandParams(QString command, dcpp::StringMap &ucParams){
    QString name;
    StringMap done;
    int i = 0;
    int j = 0;

    i = command.indexOf(QString("%[line:"), 0, Qt::CaseInsensitive);

    if (i < 0)
        return true;
    else
        command.remove(0, i+7);//also remove "%[line:" string

    while (!command.isEmpty()){
        j = command.indexOf("]");

        if (j < 0)
            break;

        name = command.left(j);

        if (done.find(name.toStdString()) == done.end()){
            QString input = QInputDialog::getText(MainWindow::getInstance(), tr("Enter parameter value"), name, QLineEdit::Normal);

            if (!input.isEmpty()){
                ucParams["line:" + name.toStdString()] = input.toStdString();
                done[name.toStdString()] = input.toStdString();
            }
            else
                return false;
        }

        command.remove(0, j);

        i = command.indexOf("%[line:");

        if (i < 0)
            break;
        else
            command.remove(0, i+7);//also remove "%[line:" string
    }

    return true;
}

QStringList WulforUtil::getLocalIPs(){
    QStringList addresses;

#if defined(HAVE_IFADDRS_H) || defined(HAVE_IF_ADDRS_H)
    struct ifaddrs *ifap;

    if (getifaddrs(&ifap) == 0){
        for (struct ifaddrs *i = ifap; i != NULL; i = i->ifa_next){
            struct sockaddr *sa = i->ifa_addr;

            // If the interface is up, is not a loopback and it has an address
            if ((i->ifa_flags & IFF_UP) && !(i->ifa_flags & IFF_LOOPBACK) && sa != NULL){
                void* src = NULL;
                socklen_t len;

                // IPv4 address
                if (sa->sa_family == AF_INET){
                    struct sockaddr_in* sai = (struct sockaddr_in*)sa;
                    src = (void*) &(sai->sin_addr);
                    len = INET_ADDRSTRLEN;
                }
                // IPv6 address
                else if (sa->sa_family == AF_INET6){
                    struct sockaddr_in6* sai6 = (struct sockaddr_in6*)sa;
                        src = (void*) &(sai6->sin6_addr);
                        len = INET6_ADDRSTRLEN;
                }

                // Convert the binary address to a string and add it to the output list
                if (src != NULL){
                    char address[len];
                    inet_ntop(sa->sa_family, src, address, len);
                    addresses.push_back(address);
                }
            }
        }

        freeifaddrs(ifap);
    }
#else
    addresses.push_back(Util::getLocalIp().c_str());
#endif

    return addresses;
}

QString WulforUtil::makeMagnet(const QString &path, const int64_t size, const QString &tth){
    if (path.isEmpty() || tth.isEmpty())
        return "";

    return magnetSignature + tth + "&xl=" + _q(Util::toString(size)) + "&dn=" + _q(Util::encodeURI(path.toStdString()));
}

int WulforUtil::sortOrderToInt(Qt::SortOrder order){
    if (order == Qt::AscendingOrder)
        return 0;
    else
        return 1;
}

Qt::SortOrder WulforUtil::intToSortOrder(int i){
    if (i == 0)
        return Qt::AscendingOrder;
    else
        return Qt::DescendingOrder;
}

QString WulforUtil::getHubNames(const dcpp::CID &cid){
    StringList hubs = ClientManager::getInstance()->getHubNames(cid);

    if (hubs.empty())
        return tr("Offline");
    else
        return _q(Util::toString(hubs));
}

QString WulforUtil::getHubNames(const dcpp::UserPtr &user){
    return getHubNames(user->getCID());
}

QString WulforUtil::getHubNames(const QString &cid){
    return getHubNames(CID(_tq(cid)));
}
