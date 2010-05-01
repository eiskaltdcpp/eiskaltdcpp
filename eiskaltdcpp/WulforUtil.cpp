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
#include "dcpp/SettingsManager.h"
#include "dcpp/Util.h"

#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QInputDialog>
#include <QDesktopServices>
#include <QUrl>
#include <QAbstractItemModel>
#include <QHostAddress>
#include <QMenu>
#include <QHeaderView>

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

using namespace dcpp;

const QString WulforUtil::magnetSignature = "magnet:?xt=urn:tree:tiger:";

WulforUtil::WulforUtil(): http(NULL)
{
    QHttpRequestHeader header("GET", WSGET(WS_APP_DYNDNS_INDEX));
    header.setValue("Host", WSGET(WS_APP_DYNDNS_SERVER));
    QString useragent = QString("EiskaltDCPP");
    header.setValue("User-Agent", useragent);

    http = new QHttp();
    connect(http, SIGNAL(done(bool)), this, SLOT(slotHttpDone(bool)));
    http->setHost(WSGET(WS_APP_DYNDNS_SERVER));
    http->request(header);

    http_timer = new QTimer();
    http_timer->setInterval(30*1000);
    connect(http_timer, SIGNAL(timeout()), this, SLOT(slotHttpTimer()));

    http_timer->start();

    memset(userIconCache, 0, sizeof (userIconCache));

    userIcons = new QImage();

    connectionSpeeds["0.005"]   = 0;
    connectionSpeeds["0.01"]    = 0;
    connectionSpeeds["0.02"]    = 0;
    connectionSpeeds["0.05"]    = 0;
    connectionSpeeds["0.1"]     = 1;
    connectionSpeeds["0.2"]     = 1;
    connectionSpeeds["0.5"]     = 2;
    connectionSpeeds["1"]       = 3;
    connectionSpeeds["2"]       = 4;
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
    QtEnc2DCEnc["WINDOWS-1250"] = "CP1250 (Central Europe)";
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
    delete http_timer;
    delete http;

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

    settings_path = QDir::homePath() + "/.eiskaltdc++/icons/user/default";
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
    QString icon_theme = WSGET(WS_APP_ICONTHEME);

    QString settings_path = QDir::currentPath() + "/icons/appl/" + icon_theme;
    settings_path = QDir::toNativeSeparators(settings_path);

    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = QDir::homePath() + "/.eiskaltdc++/icons/appl/" + icon_theme;
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = bin_path + "/appl/" + icon_theme;
    settings_path = QDir::toNativeSeparators(settings_path);
    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = QDir::homePath()+QString(".dc/icons/appl/" + icon_theme);
    settings_path = QDir::toNativeSeparators(settings_path);

    if (QDir(settings_path).exists())
        return settings_path;

    settings_path = CLIENT_ICONS_DIR "/appl/" + icon_theme;
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

    if (id->isSet(User::PASSIVE)){
        y += 16;

        if (SETTING(INCOMING_CONNECTIONS) == SettingsManager::INCOMING_FIREWALL_PASSIVE)
            x = 7;
    }

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
#if defined(Q_WS_X11) && QT_VERSION >= 0x040600
static const int PXMTHEMESIDE = 22;
#define FROMTHEME(a) QIcon::fromTheme((a)).pixmap(PXMTHEMESIDE, PXMTHEMESIDE)
#define FROMTHEME_SIDE(a, b) QIcon::fromTheme((a)).pixmap((b), (b))
#define HASICONTHEME
#endif

bool WulforUtil::loadIcons(){
    m_bError = false;

    app_icons_path = findAppIconsPath();

    m_PixmapMap.clear();

#ifdef HASICONTHEME
    m_PixmapMap[eiAWAY]         = FROMTHEME("user-away");
    m_PixmapMap[eiBOOKMARK_ADD] = FROMTHEME("bookmark-new");
    m_PixmapMap[eiCLEAR]        = FROMTHEME("edit-clear");
    m_PixmapMap[eiCONFIGURE]    = FROMTHEME("configure");
    m_PixmapMap[eiCONNECT]      = FROMTHEME("network-connect");
    m_PixmapMap[eiCONNECT_NO]   = FROMTHEME("network-disconnect");
    m_PixmapMap[eiDOWN]         = FROMTHEME("arrow-down");
    m_PixmapMap[eiDOWNLIST]     = FROMTHEME("arrow-down-double");
    m_PixmapMap[eiDOWNLOAD]     = FROMTHEME("download");
    m_PixmapMap[eiDOWNLOAD_AS]  = FROMTHEME("download");
    m_PixmapMap[eiEDIT]         = FROMTHEME("document-properties");
    m_PixmapMap[eiEDITADD]      = FROMTHEME("list-add");
    m_PixmapMap[eiEDITCOPY]     = FROMTHEME("edit-copy");
    m_PixmapMap[eiEDITDELETE]   = FROMTHEME("list-remove");
    m_PixmapMap[eiEDITCLEAR]    = FROMTHEME_SIDE("edit-clear-locationbar-rtl", 16);
    m_PixmapMap[eiEMOTICON]     = FROMTHEME("emoticon");
    m_PixmapMap[eiEXIT]         = FROMTHEME("application-exit");
    m_PixmapMap[eiFILECLOSE]    = FROMTHEME("edit-delete");
    m_PixmapMap[eiFILEFIND]     = FROMTHEME("edit-find");
    m_PixmapMap[eiFILTER]       = FROMTHEME("view-filter");
    m_PixmapMap[eiFOLDER_BLUE]  = FROMTHEME("folder-blue");
    m_PixmapMap[eiHIDEWINDOW]   = FROMTHEME("view-close");
    m_PixmapMap[eiUP]           = FROMTHEME("arrow-up");
    m_PixmapMap[eiUPLIST]       = FROMTHEME("arrow-up-double");
    m_PixmapMap[eiZOOM_IN]      = FROMTHEME("zoom-in");
    m_PixmapMap[eiZOOM_OUT]     = FROMTHEME("zoom-out");

    m_PixmapMap[eiFILETYPE_APPLICATION] = FROMTHEME("application-x-executable");
    m_PixmapMap[eiFILETYPE_ARCHIVE]     = FROMTHEME("application-x-archive");
    m_PixmapMap[eiFILETYPE_DOCUMENT]    = FROMTHEME("text-x-generic");
    m_PixmapMap[eiFILETYPE_MP3]         = FROMTHEME("audio-x-generic");
    m_PixmapMap[eiFILETYPE_PICTURE]     = FROMTHEME("image-x-generic");
    m_PixmapMap[eiFILETYPE_UNKNOWN]     = FROMTHEME("unknown");
    m_PixmapMap[eiFILETYPE_VIDEO]       = FROMTHEME("video-x-generic");
#else
    m_PixmapMap[eiAWAY]         = loadPixmap("away");
    m_PixmapMap[eiBOOKMARK_ADD] = loadPixmap("bookmark_add.png");
    m_PixmapMap[eiCLEAR]        = loadPixmap("clear.png");
    m_PixmapMap[eiCONFIGURE]    = loadPixmap("configure.png");
    m_PixmapMap[eiCONNECT]      = loadPixmap("connect.png");
    m_PixmapMap[eiCONNECT_NO]   = loadPixmap("connect_no.png");
    m_PixmapMap[eiDOWN]         = loadPixmap("down.png");
    m_PixmapMap[eiDOWNLIST]     = loadPixmap("downlist.png");
    m_PixmapMap[eiDOWNLOAD]     = loadPixmap("download.png");
    m_PixmapMap[eiDOWNLOAD_AS]  = loadPixmap("download_as.png");
    m_PixmapMap[eiEDIT]         = loadPixmap("edit.png");
    m_PixmapMap[eiEDITADD]      = loadPixmap("editadd.png");
    m_PixmapMap[eiEDITCOPY]     = loadPixmap("editcopy.png");
    m_PixmapMap[eiEDITDELETE]   = loadPixmap("editdelete.png");
    m_PixmapMap[eiEDITCLEAR]    = loadPixmap("edit-clear.png");
    m_PixmapMap[eiEMOTICON]     = loadPixmap("emoticon.png");
    m_PixmapMap[eiEXIT]         = loadPixmap("exit.png");
    m_PixmapMap[eiFILECLOSE]    = loadPixmap("fileclose.png");
    m_PixmapMap[eiFILEFIND]     = loadPixmap("filefind.png");
    m_PixmapMap[eiFILTER]       = loadPixmap("filter.png");
    m_PixmapMap[eiFOLDER_BLUE]  = loadPixmap("folder_blue.png");
    m_PixmapMap[eiHIDEWINDOW]   = loadPixmap("hidewindow.png");
    m_PixmapMap[eiUP]           = loadPixmap("up.png");
    m_PixmapMap[eiUPLIST]       = loadPixmap("uplist.png");
    m_PixmapMap[eiZOOM_IN]      = loadPixmap("zoom-in.png");
    m_PixmapMap[eiZOOM_OUT]     = loadPixmap("zoom-out.png");

    m_PixmapMap[eiFILETYPE_APPLICATION] = loadPixmap("filetype-application.png");
    m_PixmapMap[eiFILETYPE_ARCHIVE]     = loadPixmap("filetype-archive.png");
    m_PixmapMap[eiFILETYPE_DOCUMENT]    = loadPixmap("filetype-document.png");
    m_PixmapMap[eiFILETYPE_MP3]         = loadPixmap("filetype-audio.png");
    m_PixmapMap[eiFILETYPE_PICTURE]     = loadPixmap("filetype-picture.png");
    m_PixmapMap[eiFILETYPE_UNKNOWN]     = loadPixmap("filetype-unknown.png");
    m_PixmapMap[eiFILETYPE_VIDEO]       = loadPixmap("filetype-video.png");
#endif

    m_PixmapMap[eiBALL_GREEN]   = loadPixmap("ball_green.png");
    m_PixmapMap[eiCHAT]         = loadPixmap("chat.png");
    m_PixmapMap[eiERASER]       = loadPixmap("eraser.png");
    m_PixmapMap[eiFAV]          = loadPixmap("fav.png");
    m_PixmapMap[eiFAVADD]       = loadPixmap("favadd.png");
    m_PixmapMap[eiFAVREM]       = loadPixmap("favrem.png");
    m_PixmapMap[eiFAVSERVER]    = loadPixmap("favserver.png");
    m_PixmapMap[eiFAVUSERS]     = loadPixmap("favusers.png");
    m_PixmapMap[eiFIND]         = loadPixmap("find.png");
    m_PixmapMap[eiFREESPACE]    = loadPixmap("freespace.png");
    m_PixmapMap[eiGUI]          = loadPixmap("gui.png");
    m_PixmapMap[eiGV]           = QPixmap(gv_xpm);
    m_PixmapMap[eiHASHING]      = loadPixmap("hashing.png");
    m_PixmapMap[eiHUBMSG]       = loadPixmap("hubmsg.png");
    m_PixmapMap[eiICON_APPL]    = loadPixmap("icon_appl.png");
    m_PixmapMap[eiMESSAGE]      = loadPixmap("message.png");
    m_PixmapMap[eiMESSAGE_TRAY_ICON] = loadPixmap("icon_msg.png");
    m_PixmapMap[eiOWN_FILELIST] = loadPixmap("own_filelist.png");
    m_PixmapMap[eiOPENLIST]     = loadPixmap("openlist.png");
    m_PixmapMap[eiOPEN_LOG_FILE]= loadPixmap("log_file.png");
    m_PixmapMap[eiPMMSG]        = loadPixmap("pmmsg.png");
    m_PixmapMap[eiRECONNECT]    = loadPixmap("reconnect.png");
    m_PixmapMap[eiREFRLIST]     = loadPixmap("refrlist.png");
    m_PixmapMap[eiRELOAD]       = loadPixmap("reload.png");
    m_PixmapMap[eiSERVER]       = loadPixmap("server.png");
    m_PixmapMap[eiSPAM]         = loadPixmap("spam.png");
    m_PixmapMap[eiSPY]          = loadPixmap("spy.png");
    m_PixmapMap[eiSPLASH]       = QPixmap();
    m_PixmapMap[eiSTATUS]       = loadPixmap("status.png");
    m_PixmapMap[eiTRANSFER]     = loadPixmap("transfer.png");
    m_PixmapMap[eiUSERS]        = loadPixmap("users.png");
    m_PixmapMap[eiQT_LOGO]      = loadPixmap("qt-logo.png");

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

const QPixmap &WulforUtil::getPixmap(enum WulforUtil::Icons e){
    return m_PixmapMap[static_cast<qulonglong>(e)];
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
    m_FileTypeMap["JAR"] = eiFILETYPE_ARCHIVE;
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
    m_FileTypeMap["DOCX"]  = eiFILETYPE_DOCUMENT;
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

bool WulforUtil::openUrl(const QString &url){
    if (url.startsWith("http://") || url.startsWith("www.") || url.startsWith(("ftp://")) || url.startsWith("https://")){
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
    else
        return false;

    return true;
}

bool WulforUtil::getUserCommandParams(QString command, dcpp::StringMap &ucParams){
    QString name;
    static QString store = "";
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
            bool bOk;
            QString input = QInputDialog::getText(MainWindow::getInstance(), tr("Enter parameter value"), name, QLineEdit::Normal, store, &bOk);

            if (bOk){
                store = input;
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

QString WulforUtil::formatBytes(int64_t aBytes){
    QString s;
    if(aBytes < 1024)
        s = tr("%1 B").arg((int)(aBytes & 0xffffffff));
    else if(aBytes < 1024*1024)
        s = tr("%1 KiB").arg(static_cast<double>(aBytes)/1024.0, 0, 'f', 1);
    else if(aBytes < 1024*1024*1024)
        s = tr("%1 MiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0), 0, 'f', 1);
    else if(aBytes < static_cast<int64_t>(1024)*1024*1024*1024)
        s = tr("%1 GiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0*1024.0), 0, 'f', 2);
    else if(aBytes < static_cast<int64_t>(1024)*1024*1024*1024*1024)
        s = tr("%1 TiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0*1024.0*1024.0), 0, 'f', 3);
    else
        s = tr("%1 PiB").arg(static_cast<double>(aBytes)/(1024.0*1024.0*1024.0*1024.0*1024.0), 0, 'f', 4);
    return s;
}

QString WulforUtil::makeMagnet(const QString &path, const int64_t size, const QString &tth){
    if (path.isEmpty() || tth.isEmpty())
        return "";

    return magnetSignature + tth + "&xl=" + _q(Util::toString(size)) + "&dn=" + _q(Util::encodeURI(path.toStdString()));
}

void WulforUtil::splitMagnet(const QString &magnet, int64_t &size, QString &tth, QString &name){
    size = 0;
    tth = "";
    name = "";

    if (magnet.isEmpty() || !magnet.contains("urn:tree:tiger"))
        return;

    QUrl url;

    if (!magnet.contains("+"))
        url.setEncodedUrl(magnet.toAscii());
    else {
        QString _l = magnet;

        _l.replace("+", "%20");
        url.setEncodedUrl(_l.toAscii());
    }

    if (url.hasQueryItem("dn"))
        name = url.queryItemValue("dn");

    if (url.hasQueryItem("xl"))
        size = url.queryItemValue("xl").toLongLong();

    if (url.hasQueryItem("xt")){
        tth = url.queryItemValue("xt");

        if (tth.startsWith("urn:tree:tiger:"))
            tth.remove(0, strlen("urn:tree:tiger:"));
    }
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

void WulforUtil::headerMenu(QTreeView *tree){
    if (!tree || !tree->model() || !tree->header())
        return;

    QMenu * mcols = new QMenu(NULL);
    QAbstractItemModel *model = tree->model();
    QAction * column;

    int count = 0;
    for (int i = 0; i < model->columnCount(); ++i)
        count += tree->header()->isSectionHidden(tree->header()->logicalIndex(i))? 0 : 1;

    bool allowDisable = count > 1;
    int index;

    for (int i = 0; i < model->columnCount(); ++i) {
        index = tree->header()->logicalIndex(i);
        column = mcols->addAction(model->headerData(index, Qt::Horizontal).toString());
        column->setCheckable(true);

        bool checked = !tree->header()->isSectionHidden(index);

        column->setChecked(checked);
        column->setData(index);

        if (checked && !allowDisable)
            column->setEnabled(false);
    }

    QAction * chosen = mcols->exec(QCursor::pos());

    if (chosen) {
        index = chosen->data().toInt();

        if (tree->header()->isSectionHidden(index)) {
            tree->header()->showSection(index);
        } else {
            tree->header()->hideSection(index);
        }
    }

    delete mcols;
}

void WulforUtil::slotHttpDone(bool error){
    if (!error){
        QString html = QString(http->readAll());
        int start = html.indexOf(":")+2;
        int end = html.indexOf("</body>", start);


        if ((start == -1) || (end < start)) {
            internetIP = "";
        } else {
            QString ip = html.mid(start, end - start);

            if (QHostAddress().setAddress(ip)) {
                internetIP = ip;
            }
        }
    }
    else
        internetIP = "";

    if (!internetIP.isEmpty()){
        SettingsManager::getInstance()->set(SettingsManager::INTERNETIP, internetIP.toStdString());
        Client::List clients = ClientManager::getInstance()->getClients();

        for(Client::Iter i = clients.begin(); i != clients.end(); ++i) {
            if((*i)->isConnected()) {
                (*i)->reloadSettings(false);
            }
        }
    }
}

void WulforUtil::slotHttpTimer(){
    QHttpRequestHeader header("GET", WSGET(WS_APP_DYNDNS_INDEX));
    header.setValue("Host", WSGET(WS_APP_DYNDNS_SERVER));
    QString useragent = QString("EiskaltDCPP");
    header.setValue("User-Agent", useragent);

    http->request(header);
}

QMenu *WulforUtil::buildUserCmdMenu(const QList<QString> &hub_list, int ctx){
    if (hub_list.empty())
        return NULL;

    dcpp::StringList hubs;
    QMap<QString, QMenu*> registered_menus;

    foreach (QString s, hub_list)
        hubs.push_back(s.toStdString());

    QMenu *usr_menu = new QMenu(tr("User commands"));
    UserCommand::List commands = FavoriteManager::getInstance()->getUserCommands(ctx, hubs);
    bool separator = false;

    for (UserCommand::List::iterator i = commands.begin(); i != commands.end(); ++i){
        UserCommand& uc = *i;

        // Add line separator only if it's not a duplicate
        if (uc.getType() == UserCommand::TYPE_SEPARATOR && !separator){
            QAction *sep = new QAction(usr_menu);
            sep->setSeparator(true);

            usr_menu->addAction(sep);

            separator = true;
        }
        else if (uc.getType() == UserCommand::TYPE_RAW || uc.getType() == UserCommand::TYPE_RAW_ONCE){
            separator = false;

            QString raw_name = _q(uc.getName());
            QAction *action = NULL;

            if (raw_name.contains("\\")){
                QStringList submenus = raw_name.split("\\", QString::SkipEmptyParts);
                if (!submenus.isEmpty()){
                QString name = submenus.takeLast();
                QString key = "";
                QMenu *parent = usr_menu;
                QMenu *submenu;

                foreach (QString s, submenus){
                    key += s + "\\";

                    if (registered_menus.contains(key))
                        parent = registered_menus[key];
                    else {
                        submenu = new QMenu(s, parent);
                        parent->addMenu(submenu);

                        registered_menus.insert(key, submenu);

                        parent = submenu;
                    }
                }

                action = new QAction(name, parent);
                parent->addAction(action);
                }
            }
            else{
                action = new QAction(_q(uc.getName()), usr_menu);
                usr_menu->addAction(action);
            }
            if (action) {
                action->setToolTip(_q(uc.getCommand()));
                action->setStatusTip(_q(uc.getName()));
                action->setData(_q(uc.getHub()));
            }

        }
    }

    return usr_menu;
}
