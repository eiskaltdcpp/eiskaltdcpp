#ifndef FAVORITEHUBS_H
#define FAVORITEHUBS_H

#include <QWidget>
#include <QMap>
#include <QCloseEvent>

#include "ui_UIFavoriteHubs.h"
#include "ui_UIFavoriteHubEditor.h"
#include "ArenaWidget.h"
#include "WulforUtil.h"

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/Singleton.h"
#include "dcpp/FavoriteManager.h"
#include "dcpp/FavoriteManagerListener.h"

class FavoriteHubModel;
class FavoriteHubItem;

using namespace dcpp;

class FavoriteHubEditor:
        public QDialog,
        public Ui::UIFavoriteHubEditor
{
    public:
        FavoriteHubEditor(QWidget *parent = NULL):
                QDialog(parent)
        {
            setupUi(this);
        }
};

class FavoriteHubs :
        public QWidget,
        private Ui::UIFavoriteHubs,
        public ArenaWidget,
        private dcpp::FavoriteManagerListener,
        public dcpp::Singleton<FavoriteHubs>
{
    Q_OBJECT

    friend class dcpp::Singleton<FavoriteHubs>;

    typedef QMap<QString,QVariant> StrMap;
public:
    QWidget *getWidget();
    QString getArenaTitle();
    QMenu *getMenu();
    const QPixmap &getPixmap(){ return WulforUtil::getInstance()->getPixmap(WulforUtil::eiSERVER); }

    void setUnload(bool b) { unload = b; }

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void slotContexMenu(const QPoint&);
    void slotClicked(const QModelIndex&);

private:
    FavoriteHubs(QWidget* = NULL);
    virtual ~FavoriteHubs();

    void load();
    void save();

    void init();
    void initHubEditor(FavoriteHubEditor &);
    void initHubEditor(FavoriteHubEditor &, StrMap&);
    /** Init StrMap for importing into the FavoriteHubEditor */
    void getParams(const FavoriteHubEntry*, StrMap&);
    /** Init StrMap for importing into the FavoriteHubEntry */
    void getParams(const FavoriteHubEditor&, StrMap&);
    void updateEntry(FavoriteHubEntry&, StrMap&);
    void updateItem(FavoriteHubItem*, StrMap&);

    FavoriteHubModel *model;

    virtual void on(FavoriteAdded, const FavoriteHubEntryPtr) throw();
    virtual void on(FavoriteRemoved, const FavoriteHubEntryPtr) throw();

    bool unload;
};

#endif // FAVORITEHUBS_H
