/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include "ui_PrivateMessage.h"
#include "ArenaWidget.h"
#include "HubFrame.h"

class QKeyEvent;
class QEvent;
class QObject;
class QCloseEvent;
class QMenu;
class QShowEvent;

class PMWindow: public  QWidget,
                private Ui::UIPrivateMessage,
                public  ArenaWidget
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

public:
    friend class HubFrame;

    explicit PMWindow(const QString &cid_, const QString &hubUrl_);
    virtual ~PMWindow();

    QString  getArenaTitle();
    QString  getArenaShortTitle();
    QWidget *getWidget();
    QMenu   *getMenu();
    const QPixmap &getPixmap();
    ArenaWidget::Role role() const;
    void requestFilter();
    void requestFocus();
    void setCompleter(QCompleter *, UserListModel *);

    void addStatus(QString);
    void sendMessage(QString, const bool = false, const bool = false);
    QWidget *inputWidget() const;

    void setHasHighlightMessages(bool h);
    bool hasNewMessages();

public Q_SLOTS:
    void slotActivate();
    void clearChat();
    void nextMsg();
    void prevMsg();

private Q_SLOTS:
    void slotHub();
    void slotShare();
    void slotSmile();
    void slotSettingChanged(const QString&, const QString&);
    void slotSmileContextMenu();
    void slotSmileClicked();
    void slotHideFindFrame();
    void slotFindTextEdited(const QString &);
    void slotFindAll();
    void slotFindForward() { findText(nullptr); }
    void slotFindBackward(){ findText(QTextDocument::FindBackward); }
    void slotClose();

Q_SIGNALS:
    void privateMessageClosed(QString);
    void inputTextChanged();
    void inputTextMenu();

protected:
    virtual bool eventFilter(QObject*, QEvent*);
    virtual void closeEvent(QCloseEvent *);
    virtual void showEvent(QShowEvent *);

private:
    void addStatusMessage(const QString &);
    void addOutput(QString);

    void updateStyles();

    void findText(QTextDocument::FindFlags);

    bool hasMessages;
    bool hasHighlightMessages;

    QString cid;
    QString hubUrl;
    QString nick_;
    QMenu *arena_menu;

    QStringList out_messages;
    int out_messages_index;
    bool out_messages_unsent;
};
