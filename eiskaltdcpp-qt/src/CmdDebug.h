/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QEvent>
#include <QWidget>

#include "ui_UICmdDebug.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"

#include <dcpp/stdinc.h>
#include <dcpp/Singleton.h>
#include <dcpp/DebugManager.h>
#include <dcpp/Text.h>

class CmdDebugPrivate {
public:
    int maxLines;
};

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
    explicit CmdDebug(QWidget *parent = nullptr);
    ~CmdDebug() override;
    QWidget *getWidget() override;
    QString getArenaTitle() override;
    QString getArenaShortTitle() override;
    QMenu *getMenu() override;
    const QPixmap &getPixmap() override { return WICON(WulforUtil::eiCONSOLE); }
    void requestFilter() override { slotHideFindFrame(); }
    void requestFocus() override { pushButton_ClearLog->setFocus(); }
    ArenaWidget::Role role() const override { return ArenaWidget::CmdDebug; }

Q_SIGNALS:
    void coreDebugCommand(const QString&, const QString&);

private Q_SLOTS:
    void addOutput(const QString&, const QString&);
    void maxLinesChanged(int);
    void slotFindForward() { findText(nullptr); }
    void slotFindBackward(){ findText(QTextDocument::FindBackward); }
    void slotFindTextEdited(const QString &text);
    void slotFindAll();
    void slotHideFindFrame();

protected:
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void on(dcpp::DebugManagerListener::DebugDetection, const std::string& com) noexcept override;
    void on(dcpp::DebugManagerListener::DebugCommand, const std::string& mess, int typedir, const std::string& ip) noexcept override;
    void addOutput(QString msg);
    void findText(QTextDocument::FindFlags );

    Q_DECLARE_PRIVATE(CmdDebug)
    CmdDebugPrivate *d_ptr;
};
