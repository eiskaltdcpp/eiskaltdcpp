#ifndef SETTINGSSHARING_H
#define SETTINGSSHARING_H

#include <QWidget>
#include <QDirModel>

#include "ui_UISettingsSharing.h"

class ShareDirModel: public QDirModel{
    Q_OBJECT
public:

    ShareDirModel(QObject* = NULL);
    virtual ~ShareDirModel();

    Qt::ItemFlags flags(const QModelIndex& index) const;
    QVariant data(const QModelIndex& index, int role) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role);

    void setAlias(const QModelIndex&, const QString &);

Q_SIGNALS:
    void getName(QModelIndex);
    void expandMe(QModelIndex);
private:
    QSet<QString> checked;
};

class SettingsSharing :
        public QWidget,
        private Ui::UISettingsSharing
{
    Q_OBJECT
public:
    SettingsSharing(QWidget* = NULL);
    virtual ~SettingsSharing();
public slots:
    void ok();

private slots:
    void slotRecreateShare();
    void slotShareHidden(bool);
    void slotGetName(QModelIndex);
    void slotHeaderMenu();

private:
    void init();
    void updateShareView();

    ShareDirModel *model;
};

#endif // SETTINGSSHARING_H
