/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QWidget>
#include <QMap>
#include <QCloseEvent>
#include <QMetaType>

#include "ui_UIFavoriteHubs.h"
#include "ui_UIFavoriteHubEditor.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/FavoriteManagerListener.h"

class FavoriteHubModel;
class FavoriteHubItem;

using namespace dcpp;

class FavoriteHubEditor:
        public QDialog,
        public Ui::UIFavoriteHubEditor
{
    Q_OBJECT

    public:
        FavoriteHubEditor(QWidget *parent = NULL):
                QDialog(parent)
        {
            setupUi(this);
        }
};

class FavoriteHubs :
        public QWidget,
        private Ui::UIFavoriteHubs,
        public ArenaWidget,
        private dcpp::FavoriteManagerListener,
        public dcpp::Singleton<FavoriteHubs>
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    friend class dcpp::Singleton<FavoriteHubs>;

    typedef QMap<QString,QVariant> StrMap;
public:
    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiFAVSERVER); }
    ArenaWidget::Role role() const { return ArenaWidget::FavoriteHubs; }

protected:
    virtual void closeEvent(QCloseEvent *);

private Q_SLOTS:
    void slotContexMenu(const QPoint&);
    void slotClicked(const QModelIndex&);
    void slotDblClicked();
    void slotHeaderMenu();

    void slotSettingsChanged(const QString &key, const QString &value);

    void slotAdd_newButtonClicked();
    void slotChangeButtonClicked();
    void slotRemoveButtonClicked();
    void slotConnectButtonClicked();
    void slotUpdateComboBox_CID();

private:
    FavoriteHubs(QWidget* = NULL);
    virtual ~FavoriteHubs();

    void load();
    void save();

    void init();
    void initHubEditor(FavoriteHubEditor &);
    void initHubEditor(FavoriteHubEditor &, StrMap&);
    /** Init StrMap for importing into the FavoriteHubEditor */
    void getParams(const FavoriteHubEntry*, StrMap&);
    /** Init StrMap for importing into the FavoriteHubEntry */
    void getParams(const FavoriteHubEditor&, StrMap&);
    void updateEntry(FavoriteHubEntry&, StrMap&);
    void updateItem(FavoriteHubItem*, StrMap&);

    FavoriteHubItem *getItem();

    FavoriteHubModel *model;

    QStringList fakeNMDCTags;
    QStringList fakeADCTags;

    virtual void on(FavoriteAdded, const FavoriteHubEntryPtr) noexcept;
    virtual void on(FavoriteRemoved, const FavoriteHubEntryPtr) noexcept;
};

Q_DECLARE_METATYPE (FavoriteHubs*)
