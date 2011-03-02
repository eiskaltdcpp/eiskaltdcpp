/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef WULFORUTIL_H
#define WULFORUTIL_H

#include <QObject>
#include <QPixmap>
#include <QImage>
#include <QMap>
#include <QHash>
#include <QTextCodec>
#include <QTreeView>
#include <QAbstractItemModel>
#include <QHttp>
#include <QHttpHeader>

#include <boost/pool/pool_alloc.hpp>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/ClientManager.h"
#include "dcpp/User.h"
#include "dcpp/CID.h"

#include "WulforSettings.h"

#define USERLIST_ICON_SIZE      16
#define USERLIST_XPM_COLUMNS    9
#define USERLIST_XPM_ROWS       32

#define WICON(x)(WulforUtil::getInstance()->getPixmap((x)))

using namespace dcpp;

inline QString _q (const std::string &s) __attribute__((always_inline));
inline std::string _tq(const QString &s) __attribute__((always_inline));

inline QString _q (const std::string &s) { return QString::fromStdString(s); }
inline std::string _tq(const QString &s) { return s.toStdString(); }

class WulforUtil :
        public QObject,
        public dcpp::Singleton<WulforUtil>
{
    Q_OBJECT

friend class dcpp::Singleton<WulforUtil>;

public:
    Q_ENUMS (Icons)

    enum Icons {
        eiADLS = 0,
        eiAWAY,
        eiBALL_GREEN,
        eiBOOKMARK_ADD,
        eiCHAT,
        eiCLEAR,
        eiCONFIGURE,
        eiCONNECT,
        eiCONNECT_NO,
        eiCONSOLE,
        eiDOWN,
        eiDOWNLIST,
        eiDOWNLOAD,
        eiDOWNLOAD_AS,
        eiEDIT,
        eiEDITADD,
        eiEDITCOPY,
        eiEDITDELETE,
        eiEDITCLEAR,
        eiEMOTICON,
        eiERASER,
        eiEXIT,
        eiFAV,
        eiFAVADD,
        eiFAVREM,
        eiFAVSERVER,
        eiFAVUSERS,
        eiFILECLOSE,
        eiFILEFIND,
        eiFILTER,
        eiFIND,
        eiFOLDER_BLUE,
        eiFREESPACE,
        eiGUI,
        eiGV,
        eiHASHING,
        eiHIDEWINDOW,
        eiHUBMSG,
        eiICON_APPL,
        eiMESSAGE,
        eiMESSAGE_TRAY_ICON,
        eiOPENLIST,
        eiOPEN_LOG_FILE,
        eiOWN_FILELIST,
        eiPLUGIN,
        eiPMMSG,
        eiRECONNECT,
        eiREFRLIST,
        eiRELOAD,
        eiSERVER,
        eiSPAM,
        eiSPY,
        eiSPLASH,
        eiSTATUS,
        eiTRANSFER,
        eiUP,
        eiUPLIST,
        eiUSERS,
        eiZOOM_IN,
        eiZOOM_OUT,
        eiQT_LOGO,
        eiSPEED_LIMIT_ON,
        eiSPEED_LIMIT_OFF,

        eiFILETYPE_APPLICATION,
        eiFILETYPE_ARCHIVE,
        eiFILETYPE_DOCUMENT,
        eiFILETYPE_MP3,
        eiFILETYPE_PICTURE,
        eiFILETYPE_UNKNOWN,
        eiFILETYPE_VIDEO
    };

    typedef QHash<qulonglong, QPixmap> PixmapMap;

    bool loadUserIcons();
    bool loadIcons();

    QPixmap *getUserIcon(const UserPtr&, bool, bool, const QString&);

    QString getNicks(const CID &cid);

    const QString &getIconsPath() { return app_icons_path; }

    QPixmap getPixmapForFile(const QString&);

    void textToHtml(QString&,bool=true);

    QTextCodec *codecForEncoding(QString);
    //Convert Qt encoding name to internal DC++ representation
    QString qtEnc2DcEnc(QString);
    QString dcEnc2QtEnc(QString);
    QStringList encodings();

    bool getUserCommandParams(QString, dcpp::StringMap &);

    QStringList getLocalIPs();

    QString makeMagnet(const QString&, const int64_t, const QString&);
    static void splitMagnet(const QString &magnet, int64_t &size, QString &tth, QString &name);

    int sortOrderToInt(Qt::SortOrder);
    Qt::SortOrder intToSortOrder(int);

    static QString formatBytes(int64_t bytes);

    static void headerMenu(QTreeView*);

    QString getHubNames(const dcpp::CID&);
    QString getHubNames(const dcpp::UserPtr&);
    QString getHubNames(const QString&);

    QString compactToolTipText(QString, int, QString);

    std::string getInternetIP() const { return _tq(internetIP); }

    QMenu *buildUserCmdMenu(const QList<QString> &hub_list, int ctx, QWidget* = 0);

public Q_SLOTS:
    const QPixmap &getPixmap(Icons);
    QString getNicks(const QString&);
    bool openUrl(const QString&);

private Q_SLOTS:
    void slotHttpDone(bool);
    void slotHttpTimer();

private:

    WulforUtil();
    virtual ~WulforUtil();

    bool loadUserIconsFromFile(QString);
    void clearUserIconCache();
    void initFileTypes();

    static QString formatBytes(int64_t bytes, int base);

    QPixmap loadPixmap(const QString& file);

    PixmapMap m_PixmapMap;
    bool m_bError;

    QPixmap FROMTHEME(const QString &name, bool resource);
    QPixmap FROMTHEME_SIDE(const QString &name, bool resource, const int side);

    QString findAppIconsPath();

    QString app_icons_path;
    QString bin_path;

    QPixmap *userIconCache[USERLIST_XPM_COLUMNS][USERLIST_XPM_ROWS];
    QImage  *userIcons;

    QMap<QString, int> connectionSpeeds;
    QMap<QString, Icons> m_FileTypeMap;
    QMap<QString, QString> QtEnc2DCEnc;

    static const QString magnetSignature;

    QHttp *http;
    QTimer *http_timer;
    QString internetIP;
};

Q_DECLARE_METATYPE(WulforUtil*);
Q_DECLARE_METATYPE(WulforUtil::Icons);

#endif // WULFORUTIL_H
