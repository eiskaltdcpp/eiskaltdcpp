#ifndef WULFORUTIL_H
#define WULFORUTIL_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QMap>
#include <QTextCodec>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"

#include "WulforSettings.h"

#define USERLIST_ICON_SIZE      16
#define USERLIST_XPM_COLUMNS    9
#define USERLIST_XPM_ROWS       32

using namespace dcpp;

inline QString _q (const std::string &s) __attribute__((always_inline));
inline std::string _tq(const QString &s) __attribute__((always_inline));

inline QString _q (const std::string &s) { return QTextCodec::codecForCStrings()->fromUnicode(s.c_str()); }
inline std::string _tq(const QString &s) { return QTextCodec::codecForCStrings()->toUnicode(s.toAscii()).toStdString(); }

class WulforUtil :
        public QObject,
        public dcpp::Singleton<WulforUtil>
{
    Q_OBJECT

friend class dcpp::Singleton<WulforUtil>;

public:
    enum Icons {
        eiBALL_GREEN = 0,
        eiBOOKMARK_ADD,
        eiCHAT,
        eiCLEAR,
        eiCONFIGURE,
        eiCONNECT,
        eiCONNECT_NO,
        eiDOWN,
        eiDOWNLOAD,
        eiDOWNLOAD_AS,
        eiEDIT,
        eiEDITADD,
        eiEDITCOPY,
        eiEDITDELETE,
        eiEXIT,
        eiFILECLOSE,
        eiFILEFIND,
        eiFILTER,
        eiFOLDER_BLUE,
        eiGUI,
        eiGV,
        eiHASHING,
        eiICON_APPL,
        eiMESSAGE,
        eiMESSAGE_TRAY_ICON,
        eiOWN_FILELIST,
        eiRECONNECT,
        eiRELOAD,
        eiSERVER,
        eiSPAM,
        eiSPLASH,
        eiTRANSFER,
        eiUP,
        eiUSERS,
        eiZOOM_IN,
        eiZOOM_OUT,

        eiFILETYPE_APPLICATION,
        eiFILETYPE_ARCHIVE,
        eiFILETYPE_DOCUMENT,
        eiFILETYPE_MP3,
        eiFILETYPE_PICTURE,
        eiFILETYPE_UNKNOWN,
        eiFILETYPE_VIDEO,

        eiFLAG_GB,
        eiFLAG_RU
    };

    typedef QMap<enum Icons, QPixmap> PixmapMap;

    bool loadUserIcons();
    bool loadIcons();

    QPixmap *getUserIcon(const UserPtr&, bool, bool, const QString&);
    const QPixmap &getPixmap(Icons);

    QString getNicks(const QString&);
    QString getNicks(const CID &cid);

    const QString &getIconsPath() { return app_icons_path; }

    QPixmap getPixmapForFile(const QString&);

    void textToHtml(QString&,bool=true);

    QTextCodec *codecForEncoding(QString);
    //Convert Qt encoding name to internal DC++ representation
    QString qtEnc2DcEnc(QString);
    QString dcEnc2QtEnc(QString);
    QStringList encodings();

    bool openUrl(const QString&);
    
    bool getUserCommandParams(QString, dcpp::StringMap &);

    QStringList getLocalIPs();

    QString makeMagnet(const QString&, const int64_t, const QString&);

    int sortOrderToInt(Qt::SortOrder);
    Qt::SortOrder intToSortOrder(int);

    QString getHubNames(const dcpp::CID&);
    QString getHubNames(const dcpp::UserPtr&);
    QString getHubNames(const QString&);

private:

    WulforUtil();
    virtual ~WulforUtil();

    bool loadUserIconsFromFile(QString);
    void clearUserIconCache();
    void initFileTypes();

    QPixmap loadPixmap(const QString& file);

    PixmapMap m_PixmapMap;
    bool m_bError;

    QString findAppIconsPath();

    QString app_icons_path;
    QString bin_path;

    QPixmap *userIconCache[USERLIST_XPM_COLUMNS][USERLIST_XPM_ROWS];
    QImage  *userIcons;

    QMap<QString, int> connectionSpeeds;
    QMap<QString, Icons> m_FileTypeMap;
    QMap<QString, QString> QtEnc2DCEnc;

    static const QString magnetSignature;
};

#endif // WULFORUTIL_H
