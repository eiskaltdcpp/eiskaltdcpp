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
#include "ui_UICmdDebug.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"
#include <dcpp/stdinc.h>
#include <dcpp/Singleton.h>
#include <dcpp/DebugManager.h>

class CmdDebug : public QWidget,
        private Ui::UICmdDebug,
        public ArenaWidget,
        public dcpp::Singleton<CmdDebug>,
        private dcpp::DebugManagerListener
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    friend class dcpp::Singleton<CmdDebug>;
public:
    explicit CmdDebug(QWidget *parent = 0);
    virtual ~CmdDebug();
    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap(){ return WICON(WulforUtil::eiCONSOLE); }
    ArenaWidget::Role role() const { return ArenaWidget::CmdDebug; }

Q_SIGNALS:
    void coreDebugCommand(const QString&, const QString&);

private Q_SLOTS:
    void addOutput(const QString&, const QString&);
    void maxLinesChanged(int);
private:
    int maxLines;
    void on(dcpp::DebugManagerListener::DebugDetection, const std::string& com) noexcept;
    void on(dcpp::DebugManagerListener::DebugCommand, const std::string& mess, int typedir, const std::string& ip) noexcept;

    void addOutput(QString msg);
};
