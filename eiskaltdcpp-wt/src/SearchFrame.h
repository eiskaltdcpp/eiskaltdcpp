/* 
 * File:   SearchFrame.h
 * Author: negativ
 *
 * Created on 18 Июнь 2010 г., 12:34
 */

#ifndef _SEARCHFRAME_H
#define	_SEARCHFRAME_H

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/ClientManager.h"
#include "dcpp/SearchManager.h"
#include "dcpp/SearchResult.h"
#include "dcpp/StringTokenizer.h"
#include "dcpp/Encoder.h"

#include <map>
#include <boost/any.hpp>

#include <Wt/WContainerWidget>
#include <Wt/WLabel>
#include <Wt/WPushButton>
#include <Wt/WTreeView>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>

#include "SearchModel.h"

class SearchFrame:
        public Wt::WContainerWidget,
        private dcpp::SearchManagerListener
{
    typedef std::map<Wt::WString, boost::any> VarMap;

public:
    SearchFrame(Wt::WContainerWidget *parent = 0);
    virtual ~SearchFrame();

protected:
    virtual void on(dcpp::SearchManagerListener::SR, const dcpp::SearchResultPtr& aResult) throw();

private:
    SearchFrame(const SearchFrame& orig);
    SearchFrame& operator=(const SearchFrame&){}

    bool getDownloadParams(VarMap &params, SearchModelItem *item);
    void download(VarMap &params);
    void downloadSelected();

    void startSearch();

    Wt::WContainerWidget *container;
    Wt::WVBoxLayout *vlayout;
    Wt::WHBoxLayout *hlayout;
    Wt::WLabel *label1;
    Wt::WComboBox *comboBox_TYPE;
    Wt::WLineEdit *lineEdit_SEARCH;
    Wt::WPushButton *pushButton_SEARCH;
    Wt::WPushButton *pushButton_DOWNLOAD;
    Wt::WTreeView *view;

    SearchModel *model;

    std::string token;
    dcpp::TStringList currentSearch;
};

#endif	/* _SEARCHFRAME_H */

