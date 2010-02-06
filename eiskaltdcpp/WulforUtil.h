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
        eiBACK = 0,
        eiBALL_GREEN,
        eiBALL_RED,
        eiBALL_YELLOW,
        eiBOOKMARK_ADD,
        eiBOOKMARK_FOLDER,
        eiCHAT,
        eiCHOOSE_LANGUAGE,
        eiCLEAR,
        eiCLIENT,
        eiCOLORS,
        eiCONFIGURE,
        eiCONFIGURE_32x32,
        eiCONNECT,
        eiCONNECT_CREATING,
        eiCONNECT_NO,
        eiCONNECTED,
        eiDEBUG,
        eiDOWN,
        eiDOWNLOAD,
        eiDOWNLOAD_AS,
        eiEDIT,
        eiEDITADD,
        eiEDITCOPY,
        eiEDITDELETE,
        eiEMOTICON,
        eiEXIT,
        eiFILECLOSE,
        eiFILEFIND,
        eiFILETYPE_APPLICATION,
        eiFILETYPE_ARCHIVE,
        eiFILETYPE_DOCUMENT,
        eiFILETYPE_MP3,
        eiFILETYPE_PICTURE,
        eiFILETYPE_UNKNOWN,
        eiFILETYPE_VIDEO,
        eiFILTER,
        eiFIND,
        eiFIND_32x32,
        eiFLAG_BELARUS,
        eiFLAG_BOSNIA,
        eiFLAG_BRAZIL,
        eiFLAG_BRITAIN,
        eiFLAG_CZECH,
        eiFLAG_DENMARK,
        eiFLAG_FINLAND,
        eiFLAG_FRANCE,
        eiFLAG_GERMANY,
        eiFLAG_GREECE,
        eiFLAG_HUNGARY,
        eiFLAG_ICELAND,
        eiFLAG_ITALY,
        eiFLAG_LATVIA,
        eiFLAG_NETHERLANDS,
        eiFLAG_NORWAY,
        eiFLAG_POLAND,
        eiFLAG_ROMANIA,
        eiFLAG_RUSSIA,
        eiFLAG_SERBIA,
        eiFLAG_SLOVAKIA,
        eiFLAG_SPAIN,
        eiFLAG_SWEDEN,
        eiFOLDER_BLUE,
        eiFOLDER_BLUE_OPEN,
        eiFOLDER_RED,
        eiGLOBE,
        eiGO_NEXT,
        eiGO_PREVIOUS,
        eiGO_UP,
        eiGUI,
        eiGV,
        eiHELP,
        eiICON_APPL,
        eiICON_MSG,
        eiICONS,
        eiINFO,
        eiISAWAY,
        eiLOG,
        eiMENU,
        eiMESSAGE,
        eiNEXT,
        eiNOTCONNECTED,
        eiONLINEMANUAL,
        eiOPEN,
        eiOTHER,
        eiQUICKOPTIONS,
        eiRECONNECT,
        eiRELOAD,
        eiRELOAD_32x32,
        eiSAVE,
        eiSERVER,
        eiSLOTS_UP,
        eiSORT_DOWN_ARROW,
        eiSPAM,
        eiSPLASH,
        eiSPY,
        eiSSL_NO,
        eiSSL_YES,
        eiSUPPORT,
        eiTEXT_SELECT_ALL,
        eiTRANSFER,
        eiTRANSLATE,
        eiUP,
        eiUPDATE,
        eiUSERS,
        eiUSERS_32x32,
        eiVIEW_SIDETREE,
        eiVIEW_LIST_DETAILS,
        eiVIEW_LIST_ICONS,
        eiVRU,
        eiZOOM_IN,
        eiZOOM_OUT
    };

    typedef QMap<enum Icons, QPixmap> PixmapMap;

    bool loadUserIcons();
    bool loadIcons();

    QPixmap *getUserIcon(const UserPtr&, bool, bool, const QString&);
    QPixmap &getPixmap(Icons);

    QString getNicks(const QString&);
    QString getNicks(const CID &cid);

    QPixmap getPixmapForFile(const QString&);

    void textToHtml(QString&,bool=true);

    QTextCodec *codecForEncoding(QString);
    //Convert Qt encoding name to internal DC++ representation
    QString qtEnc2DcEnc(QString);
    QString dcEnc2QtEnc(QString);
    QStringList encodings();

    void openUrl(const QString&);
    
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
