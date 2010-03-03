#ifndef SPYFRAME_H
#define SPYFRAME_H

#include <QWidget>

#include "ui_UISpy.h"

class SpyFrame :
        public QWidget,
        private Ui::UISpy
{
Q_OBJECT
public:
    explicit SpyFrame(QWidget *parent = 0);

signals:

public slots:

};

#endif // SPYFRAME_H
