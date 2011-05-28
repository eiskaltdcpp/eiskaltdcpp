#include "ShareBrowserSearch.h"
#include "WulforUtil.h"
#include "ShareBrowser.h"
#include "FileBrowserModel.h"

#include <QCloseEvent>
#include <QDir>

#include <boost/bind.hpp>
#include <boost/function.hpp>

ShareBrowserSearch::ShareBrowserSearch(QWidget *parent): QDialog(parent), searchRoot(NULL) {
    setupUi(this);

    if (WVGET("sharebrowsersearch/size").isValid())
        resize(WVGET("sharebrowsersearch/size").toSize());

    treeWidget->header()->restoreState(WVGET("sharebrowsersearch/columnstate").toByteArray());

    setAttribute(Qt::WA_DeleteOnClose, true);

    connect(pushButton_SEARCH, SIGNAL(clicked()), this, SLOT(slotStartSearch()));
    connect(this, SIGNAL(gotItem(QString,FileBrowserItem*)), this, SLOT(slotGotItem(QString,FileBrowserItem*)), Qt::QueuedConnection);
    connect(treeWidget, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(slotItemActivated(QTreeWidgetItem*,int)));

    show();
}

ShareBrowserSearch::~ShareBrowserSearch(){
    hash.clear();

    treeWidget->clear();
}

void ShareBrowserSearch::closeEvent(QCloseEvent *e){
    WVSET("sharebrowsersearch/size", size());
    WVSET("sharebrowsersearch/columnstate", treeWidget->header()->saveState());

    QDialog::closeEvent(e);
}

void ShareBrowserSearch::setSearchRoot(FileBrowserItem *root){
    searchRoot = root;
}

void ShareBrowserSearch::slotStartSearch(){
    treeWidget->clear();

    hash.clear();

    label_STATS->setText("");

    if (lineEdit_SEARCHSTR->text().isEmpty())
        return;

    setWindowTitle(tr("Search - %1").arg(lineEdit_SEARCHSTR->text()));

    AsyncRunner *runner = new AsyncRunner(this);
    boost::function<void()> f = boost::bind(&ShareBrowserSearch::findMatches, this, searchRoot);
    runner->setRunFunction(f);

    connect(runner, SIGNAL(finished()), runner, SLOT(deleteLater()));

    regexp = QRegExp(lineEdit_SEARCHSTR->text(), Qt::CaseInsensitive, QRegExp::Wildcard);

    runner->start();
}

static QString getPath(FileBrowserItem *path){
    QString p = "";

    while (path){
        p = path->data(COLUMN_FILEBROWSER_NAME).toString() + QDir::separator() + p;

        path = path->parent();
    }

    return p;
}

void ShareBrowserSearch::slotGotItem(QString item, FileBrowserItem *path){
    if (!path || item.isEmpty())
        return;

    QTreeWidgetItem *i = new QTreeWidgetItem(QStringList() << item << getPath(path), 0);

    items.push_back(i);
    hash.insert(i, path);

    treeWidget->insertTopLevelItem(0, i);

    label_STATS->setText(QString("Found %1 items").arg(items.size()));
}

void ShareBrowserSearch::slotItemActivated(QTreeWidgetItem *item, int){
    QHash<QTreeWidgetItem*,FileBrowserItem*>::iterator it = hash.find(item);

    if (it != hash.end())
        emit indexClicked(it.value());
}

void ShareBrowserSearch::findMatches(FileBrowserItem *item){
    if (!item)
        return;

    foreach(FileBrowserItem *i, item->childItems){
        if (i->dir){
            findMatches(i);

            DirectoryListing::File::List *files = &i->dir->files;
            DirectoryListing::File::Iter it_file;

            for (it_file = files->begin(); it_file != files->end(); ++it_file){
                if (regexp.exactMatch(_q((*it_file)->getName())))
                    emit gotItem(_q((*it_file)->getName()), i);
            }
        }
    }
}
