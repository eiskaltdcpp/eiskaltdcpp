/* 
 * File:   SearchFrame.cpp
 * Author: negativ
 * 
 * Created on 18 Июнь 2010 г., 12:34
 */

#include "SearchFrame.h"

#include <stdio.h>

using namespace Wt;

SearchFrame::SearchFrame(Wt::WContainerWidget *parent): WContainerWidget(parent) {
    resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));
    
    vlayout = new WVBoxLayout();
    setLayout(vlayout);

    hlayout = new WHBoxLayout();

    vlayout->addLayout(hlayout);

    {//Header
        container = new WContainerWidget();
        container->setStyleClass("search-control-container");

        label1 = new WLabel("File type");
        label1->setStyleClass("search-label");

        comboBox_TYPE = new WComboBox();
        comboBox_TYPE->setStyleClass("combobox-types");

        comboBox_TYPE->addItem("Any");
        comboBox_TYPE->addItem("Audio");
        comboBox_TYPE->addItem("Archive");
        comboBox_TYPE->addItem("Document");
        comboBox_TYPE->addItem("Executable");
        comboBox_TYPE->addItem("Image");
        comboBox_TYPE->addItem("Video");
        comboBox_TYPE->addItem("Directory");
        comboBox_TYPE->addItem("TTH");

        comboBox_TYPE->setReadOnly(true);
        comboBox_TYPE->resize(WLength::Auto, label1->lineHeight());

        lineEdit_SEARCH = new WLineEdit();
        lineEdit_SEARCH->setStyleClass("search-edit");
        lineEdit_SEARCH->setEmptyText("Search for...");

        pushButton_SEARCH = new WPushButton("Search");
        pushButton_SEARCH->setStyleClass("search-button");

        container->addWidget(label1);
        container->addWidget(comboBox_TYPE);
        container->addWidget(lineEdit_SEARCH);
        container->addWidget(pushButton_SEARCH);

        hlayout->addWidget(container, 1, AlignLeft);
    }
    {//Body
        view = new WTreeView(this);
        view->setModel(model = new SearchModel(view));
        view->setAlternatingRowColors(true);
        view->resize(WLength(100, WLength::Percentage), WLength(100, WLength::Percentage));

        vlayout->addWidget(view, 5);
    }
}

SearchFrame::SearchFrame(const SearchFrame& orig) {
}

SearchFrame::~SearchFrame() {
}

