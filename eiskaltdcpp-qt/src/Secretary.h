/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/
/*
 * Copyright (C) 2026 Joe Rivera <transfix@sublevels.net>
 */

#pragma once

#include <QEvent>
#include <QStringList>
#include <QWidget>

#include "ui_UISecretary.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"

#include <dcpp/stdinc.h>
#include <dcpp/Text.h>

class HubFrame;

class SecretaryPrivate {
public:
    int maxLines = 0;

    QStringList origMessages;
};

#include "QtContextAware.h"
#include "QtContext.h"

class Secretary :
        public  QWidget,
        private Ui::UISecretary,
        public  ArenaWidget,
        public QtContextAware
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    friend class QtContext;
    friend class HubFrame;

public:
    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap(){ return qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiMAGNET); }
    void requestClear() { clearNotes(); }
    void requestFilter() { slotShowSearchBar(); }
    void requestFocus() { pushButton_ClearLog->setFocus(); }
    ArenaWidget::Role role() const { return ArenaWidget::Secretary; }

Q_SIGNALS:
    void coreStatusMsg(const QString, const QString, const QString, const QString);
    void coreChatMessage(const QString, const QString, const QString, const QString);
    void corePrivateMsg(const QString, const QString, const QString, const QString);

private Q_SLOTS:
    void clearNotes();
    void searchMagnetLinks(bool);
    void maxLinesChanged(int);
    void slotChatMenu(const QPoint&);
    void slotFindForward() { findText(QTextDocument::FindFlags()); }
    void slotFindBackward(){ findText(QTextDocument::FindBackward); }
    void slotFindTextEdited(const QString &text);
    void slotFindAll();
    void slotShowSearchBar();
    void slotHideSearchBar();
    void slotSettingsChanged(const QString&, const QString&);
    void addStatus(const QString &nick, const QString &htmlMsg, const QString &origMsg, const QString &url);
    void newChatMsg(const QString &nick, const QString &htmlMsg, const QString &origMsg, const QString &url);
    void newPrivMsg(const QString &nick, const QString &htmlMsg, const QString &origMsg, const QString &url);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *e);

public:
    explicit Secretary(dcpp::DCContext& ctx, QWidget *parent = nullptr);
    ~Secretary() override;


private:
    Secretary(const Secretary&) = delete;
    const Secretary& operator=(const Secretary&) = delete;

    void updateStyles();
    void findText(QTextDocument::FindFlags );
    void addOutput(const QString &htmlMsg, const QString &origMsg, const QString &url);
    void addUserData(const QString &);

    Q_DECLARE_PRIVATE(Secretary)
    SecretaryPrivate *d_ptr;
};
