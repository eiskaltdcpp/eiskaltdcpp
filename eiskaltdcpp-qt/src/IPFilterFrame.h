/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#pragma once

#include <QDialog>
#include <QTreeWidgetItem>
#include <QCloseEvent>
#include <QRadioButton>
#include <QList>

#include "extra/ipfilter.h"
#include <IPFilterModel.h>

#include "ui_UIIPFilter.h"

class IPFilterFrame : public QDialog, private Ui::UIIPFilter {
    Q_OBJECT

public:
    /** */
    IPFilterFrame(QWidget *parent = nullptr);
    /** */
    ~IPFilterFrame() override;

private:
    /** */
    void InitDocument();
    /** */
    void loadItems();

    /** */
    IPFilterModel *model;
protected:
    void closeEvent(QCloseEvent *e) override;

private slots:
    /** */
    void slotCheckBoxClick();

    /** */
    void slotRuleAdded(QString, eDIRECTION);

    /** */
    void slotTreeViewContextMenu(QPoint);

    /** */
    void slotExport();
    /** */
    void slotImport();

    /** */
    void slotUpDownClick();

    /** */
    void slotAddRule();
};
