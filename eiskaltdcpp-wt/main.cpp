/* 
 * File:   main.cpp
 * Author: negativ
 *
 * Created on 17 Июнь 2010 г., 16:13
 */

#include <stdlib.h>

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>

#include <Wt/Ext/Menu>
#include <Wt/Ext/ToolBar>
#include <Wt/Ext/Button>

#include <Wt/WString>

#include "SearchFrame.h"

using namespace Wt;

class WApp: public Wt::WApplication{
public:
    WApp(const Wt::WEnvironment &env): Wt::WApplication(env){
        setTitle("EiskaltDC++ Web Control");

        useStyleSheet("eiskaltdcpp.css");
        messageResourceBundle().use("eiskaltdcpp");

        toolbar = new Ext::ToolBar(root());
        
        search_btn = toolbar->addButton("Search");
        search_btn->setIcon("resources/edit-find.png");

        another_btn = toolbar->addButton("Another");
        another_btn->setIcon("resources/edit-find.png");

        root()->addWidget(sfr = new SearchFrame());

        currentWidget = sfr;
        another_widget = new WContainerWidget();

        search_btn->clicked().connect(this, &WApp::showSearchFrame);
        another_btn->clicked().connect(this, &WApp::showAnotherFrame);
    }

    void showSearchFrame(){
        if (currentWidget == sfr)
            return;

        root()->removeWidget(currentWidget);
        root()->addWidget(sfr);

        currentWidget = sfr;
    }

    void showAnotherFrame(){
        if (currentWidget == another_widget)
            return;

        root()->removeWidget(currentWidget);
        root()->addWidget(another_widget);

        currentWidget = another_widget;
    }

    virtual ~WApp() {
        delete toolbar;
        delete sfr;
    }

private:
    Ext::ToolBar *toolbar;
    Ext::Button *search_btn;
    Ext::Button *another_btn;

    SearchFrame *sfr;
    WContainerWidget *another_widget;

    WContainerWidget *currentWidget;
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  /*
   * You could read information from the environment to decide whether
   * the user has permission to start a new application
   */
  return new WApp(env);
}

int main(int argc, char** argv) {
    return Wt::WRun(argc, argv, &createApplication);
}

