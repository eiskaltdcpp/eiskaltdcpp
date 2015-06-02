/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "IPFilterFrame.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHash>
#include <QHashIterator>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QInputDialog>
#include <QDir>

#include <QFileDialog>

#include "WulforSettings.h"
#include "WulforUtil.h"

IPFilterFrame::IPFilterFrame(QWidget *parent) : QDialog(parent) {
    setupUi(this);

    model = NULL;

    InitDocument();
}

IPFilterFrame::~IPFilterFrame() {
    if (model)
        delete model;
}

void IPFilterFrame::InitDocument() {
    if (WBGET(WB_IPFILTER_ENABLED)) {
        checkBox_ENABLE->setChecked(true);
    } else
        checkBox_ENABLE->setChecked(false);

    if (!model)
        model = new IPFilterModel(this);

    treeView_RULES->setModel(model);
    treeView_RULES->setContextMenuPolicy(Qt::CustomContextMenu);
    treeView_RULES->setAlternatingRowColors(true);

    slotCheckBoxClick();

    connect(checkBox_ENABLE, SIGNAL(clicked()), this, SLOT(slotCheckBoxClick()));

    connect(pushButton_EXPORT, SIGNAL(clicked()), this, SLOT(slotExport()));
    connect(pushButton_IMPORT, SIGNAL(clicked()), this, SLOT(slotImport()));
    connect(pushButton_ADD, SIGNAL(clicked()), this, SLOT(slotAddRule()));

    connect(pushButton_UP, SIGNAL(clicked()), this, SLOT(slotUpDownClick()));
    connect(pushButton_DOWN, SIGNAL(clicked()), this, SLOT(slotUpDownClick()));

    connect(treeView_RULES, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotTreeViewContextMenu(QPoint)));
}

void IPFilterFrame::slotCheckBoxClick() {
    bool b = checkBox_ENABLE->isChecked();

    pushButton_EXPORT->setEnabled(b);
    pushButton_IMPORT->setEnabled(b);
    treeView_RULES->setEnabled(b);
    groupBox_DIRECTION->setEnabled(b);
    groupBox_ACTION->setEnabled(b);
    lineEdit_RULE->setEnabled(b);
    pushButton_ADD->setEnabled(b);
    pushButton_DOWN->setEnabled(b);
    pushButton_UP->setEnabled(b);

    if (b) {
        if (!IPFilter::getInstance()){
            IPFilter::newInstance();

            IPFilter::getInstance()->loadList();
        }

        loadItems();

        connect(IPFilter::getInstance(), SIGNAL(ruleAdded(QString,eDIRECTION,eTableAction)), this, SLOT(slotRuleAdded(QString,eDIRECTION,eTableAction)));

    } else {
        if (IPFilter::getInstance()) {
            IPFilter::getInstance()->saveList();

            IPFilter::deleteInstance();

            model->clearModel();
        }
    }

    WBSET(WB_IPFILTER_ENABLED, b);
}

void IPFilterFrame::slotRuleAdded(QString exp, eDIRECTION direction, eTableAction act) {
    QString type = "OUT";
    QString action  = "Forbid";

    switch (direction) {
        case eDIRECTION_BOTH:
            type = "BOTH";

            break;
        case eDIRECTION_IN:
            type = "IN";

            break;
        default:
            break;
    }

    if (act == etaACPT)
        action = "Allow";

    model->addResult(exp, type, tr(action.toStdString().c_str()));
}

void IPFilterFrame::loadItems() {
    if (!IPFilter::getInstance() || !model)
        return;

    model->clearModel();

    QIPList list = IPFilter::getInstance()->getRules();

    for (int i = 0; i < list.size(); i++){

        IPFilterElem *el = list.at(i);
        QString action = (el->action == etaDROP ? "Forbid" : "Allow");
        QString type = "OUT";

        switch (el->direction) {
            case eDIRECTION_BOTH:
                type = "BOTH";

                break;
            case eDIRECTION_IN:
                type = "IN";

                break;
            default:
                break;
        }

        model->addResult(QString(IPFilter::Uint32ToString(el->ip)) + "/" + QString().setNum(IPFilter::MaskToCIDR(el->mask)), type, tr(action.toStdString().c_str()));
    }
}

void IPFilterFrame::slotTreeViewContextMenu(QPoint){
    if (!IPFilter::getInstance() || !model)
        return;

    WulforUtil *WU = WulforUtil::getInstance();

    eTableAction act = etaACPT;
    eDIRECTION   direction = eDIRECTION_OUT;
    QString      str_ip, str_d, str_a;

    QModelIndexList mindexes = treeView_RULES->selectionModel()->selectedIndexes();

    if (mindexes.isEmpty()){
        return;
    }

    IPFilterModelItem *elem = (IPFilterModelItem*)mindexes.at(0).internalPointer();//Selection mode: single item
    if (!elem)
        return;

    str_ip = elem->data(COLUMN_RULE_NAME).toString();
    str_d  = elem->data(COLUMN_RULE_DIRECTION).toString();
    str_a  = elem->data(COLUMN_RULE_ACTION).toString();

    if(str_a == tr("Forbid")){
        act = etaDROP;
    }

    if (str_d == "BOTH")
        direction = eDIRECTION_BOTH;
    else if (str_d == "IN")
        direction = eDIRECTION_IN;

    QMenu *m = new QMenu(this);

    QMenu *ch_d = new QMenu(this);
    ch_d->setTitle(tr("Change rule direction"));

    QList<QAction*> directions;
    directions << (ch_d->addAction(QIcon(), "BOTH"));
    directions << (ch_d->addAction(QIcon(), "IN"));
    directions << (ch_d->addAction(QIcon(), "OUT"));

    QMenu *ch_a = new QMenu(this);
    ch_a->setTitle(tr("Change rule action"));
    QList<QAction*> actions;
    actions << (ch_a->addAction(QIcon(), tr("Forbid")));
    actions[actions.length()-1]->setData(etaDROP);
    actions << (ch_a->addAction(QIcon(), tr("Allow")));
    actions[actions.length()-1]->setData(etaACPT);

    m->addMenu(ch_d);
    m->addMenu(ch_a);
    QAction *ch  = m->addAction(QIcon(), tr("Change IP/Mask"));
    QAction *del = m->addAction(WU->getPixmap(WulforUtil::eiEDITDELETE), tr("Delete rule"));

    QAction *result = m->exec(QCursor::pos());

    if (!result)
        return;

    if (directions.contains(result)){
        QString txt = result->text();
        eDIRECTION d = eDIRECTION_OUT;

        if (txt == "BOTH")
            d = eDIRECTION_BOTH;
        else if (txt == "IN")
            d = eDIRECTION_IN;

        if (d != direction){
            IPFilter::getInstance()->changeRuleDirection(str_ip, d, act);

            elem->updateColumn(COLUMN_RULE_DIRECTION, txt);
        }
    }
    else if (actions.contains(result)){
        eTableAction a = etaDROP;

        if (result->data() == etaACPT)
            a = etaACPT;

        if (a != act){
            IPFilter::getInstance()->changeRuleDirection(str_ip, direction, a);

            elem->updateColumn(COLUMN_RULE_ACTION, result->text());
        }
    }
    else if (result == del){
        IPFilter::getInstance()->remFromRules(str_ip, act);
        model->removeItem(elem);
    }
    else if (result == ch){
        quint32 ip, mask, old_ip, old_mask;

        bool ok = false;

        QString input = QInputDialog::getText(this, tr("Enter new rule"), tr("Rule:"),
                                              QLineEdit::Normal, elem->data(COLUMN_RULE_NAME).toString(), &ok);

        if (ok &&
            IPFilter::ParseIPString(input, ip, mask) &&
            IPFilter::ParseIPString(elem->data(COLUMN_RULE_NAME).toString(), old_ip, old_mask))
        {
            if ((ip != old_ip) || (mask != old_mask)){
                const QIPHash &hash = IPFilter::getInstance()->getHash();
                auto it = hash.find(old_ip);
                IPFilterElem *el = NULL;

                while (it != hash.end() && it.key() == old_ip){
                    IPFilterElem *t = it.value();

                    if (t->mask == old_mask){
                        el = t;

                        break;
                    }

                    ++it;
                }

                if (el){
                    el->ip     = ip;
                    el->mask   = mask;

                    QString str_ip     = IPFilter::Uint32ToString(ip);
                    QString str_mask   = QString().setNum(IPFilter::MaskToCIDR(mask));

                    elem->updateColumn(COLUMN_RULE_NAME, str_ip+"/"+str_mask);
                }
            }
        }
    }

    model->repaint();

    delete m;
}

void IPFilterFrame::closeEvent(QCloseEvent *e) {
    if (IPFilter::getInstance()) {
        IPFilter::getInstance()->saveList();
    }

    e->accept();
}

void IPFilterFrame::slotAddRule() {
    if (!IPFilter::getInstance())
        return;

    QString rule = lineEdit_RULE->text();

    if (!(rule.trimmed().isEmpty() || rule.trimmed().isNull())) {
        eDIRECTION direction = eDIRECTION_OUT;

        if (radioButton_BOTH->isChecked())
            direction = eDIRECTION_BOTH;
        else if (radioButton_IN->isChecked())
            direction = eDIRECTION_IN;

        eTableAction act = etaDROP;
        if (checkBox_ALLOW->isChecked())
            act = etaACPT;

        QStringList rule_list;

        if (rule.indexOf(",") > 0)
            rule_list = rule.split(",");
        else
            rule_list << rule;

        for (int i = 0; i < rule_list.size(); i++){
            QString ip = rule_list.at(i);

            ip.replace(" ", "");

            IPFilter::getInstance()->addToRules(ip.trimmed(), direction, act);
        }
    }

    lineEdit_RULE->selectAll();
    lineEdit_RULE->setFocus();
}

void IPFilterFrame::slotImport() {
    if (!IPFilter::getInstance() || !model)
        return;

    QString fname = QFileDialog::getOpenFileName(this, tr("Import list"), QDir::homePath());

    if (fname != ""){
        fname = QDir::toNativeSeparators(fname);

        model->clearModel();

        IPFilter::getInstance()->importFrom(fname);

        loadItems();
    }
}

void IPFilterFrame::slotUpDownClick(){
    QModelIndexList mindexes = treeView_RULES->selectionModel()->selectedIndexes();

    if (mindexes.isEmpty() || !IPFilter::getInstance()){
        return;
    }

    const QModelIndex &index = mindexes.at(0);
    IPFilterModelItem *elem = (IPFilterModelItem*)mindexes.at(0).internalPointer();//Selection mode: single item
    QString exp = elem->data(COLUMN_RULE_NAME).toString();

    quint32 ip, mask;
    eTableAction act;

    if (!IPFilter::ParseString(exp, ip, mask, act))
        return;

    if (sender() == pushButton_UP){
        model->moveUp(index);
        IPFilter::getInstance()->moveRuleUp(ip, act);
    }
    else{
        model->moveDown(index);
        IPFilter::getInstance()->moveRuleDown(ip, act);
    }

    model->repaint();

    treeView_RULES->selectionModel()->setCurrentIndex(index,
                                                      QItemSelectionModel::Clear);

    for (int i = COLUMN_RULE_NAME; i <= COLUMN_RULE_DIRECTION; i++)
        treeView_RULES->selectionModel()->setCurrentIndex(model->index(elem->row(), i, QModelIndex()),
                                                          QItemSelectionModel::Select);
}

void IPFilterFrame::slotExport() {
    if (!IPFilter::getInstance())
        return;

    QString fname = QFileDialog::getSaveFileName(this, tr("Export list"), QDir::homePath(),
            tr("All Files (*)"));

    if (fname != ""){
        fname = QDir::toNativeSeparators(fname);
        IPFilter::getInstance()->exportTo(fname);
    }
}

