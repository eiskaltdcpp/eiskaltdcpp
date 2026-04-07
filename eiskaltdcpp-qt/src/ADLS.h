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

#include "ui_UIADLSearch.h"
#include "ui_UIADLSearchEditor.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"
#include <dcpp/stdinc.h>
#include <dcpp/ADLSearch.h>

class ADLSModel;
class ADLSItem;

using namespace dcpp;

class ADLSEditor:
        public QDialog,
        public Ui::UIADLSEditor
{
    public:
        ADLSEditor(QWidget *parent = nullptr):
                QDialog(parent)
        {
            setupUi(this);

            setFixedSize(sizeHint());
        }
};

#include "QtContextAware.h"
#include "QtContext.h"

class ADLS :
        public QWidget,
        private Ui::UIADLS,
        public ArenaWidget,
        public QtContextAware
{
    Q_OBJECT
    Q_INTERFACES(ArenaWidget)

    friend class QtContext;

    typedef QMap<QString,QVariant> StrMap;
public:
    QWidget *getWidget();
    QString getArenaTitle();
    QString getArenaShortTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap(){ return qtCtx()->wulforUtil()->getPixmap(WulforUtil::eiADLS); }
    ArenaWidget::Role role() const { return ArenaWidget::ADLS; }

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
    void slotUpButtonClicked();
    void slotDownButtonClicked();

public:
    ADLS(dcpp::DCContext& ctx, QWidget* = nullptr);
    ~ADLS() override;


private:

    void load();
    void save();

    void init();
    void initEditor(ADLSEditor &);
    void initEditor(ADLSEditor &, StrMap&);
    /** Init StrMap for importing into the ADLSEditor */
    void getParams(/*const*/ ADLSearch&, StrMap&);
    /** Init StrMap for importing into the ADLSearchManager::SearchCollection */
    void getParams(const ADLSEditor&, StrMap&);
    void updateEntry(ADLSearch&, StrMap&);
    void updateItem(ADLSItem*, StrMap&);
    void addItem(const ADLSearch &);
    QString SizeTypeToString(ADLSearch::SizeType);
    QString SourceTypeToString(ADLSearch::SourceType);
    ADLSItem *getItem();
    /*VectorSize*/int findEntry(StrMap&);
    ADLSModel *model;

};
