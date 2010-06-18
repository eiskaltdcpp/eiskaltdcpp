/* 
 * File:   SearchFrame.h
 * Author: negativ
 *
 * Created on 18 Июнь 2010 г., 12:34
 */

#ifndef _SEARCHFRAME_H
#define	_SEARCHFRAME_H

#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WTreeView>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>

#include "SearchModel.h"

class SearchFrame: public Wt::WContainerWidget {
public:
    SearchFrame(Wt::WContainerWidget *parent = 0);
    virtual ~SearchFrame();

private:
    SearchFrame(const SearchFrame& orig);
    SearchFrame& operator=(const SearchFrame&){}

    Wt::WContainerWidget *container;
    Wt::WVBoxLayout *vlayout;
    Wt::WHBoxLayout *hlayout;
    Wt::WLabel *label1;
    Wt::WComboBox *comboBox_TYPE;
    Wt::WLineEdit *lineEdit_SEARCH;
    Wt::WPushButton *pushButton_SEARCH;
    Wt::WTreeView *view;

    SearchModel *model;
};

#endif	/* _SEARCHFRAME_H */

