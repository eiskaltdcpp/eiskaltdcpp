#include "TabButton.h"

#include <QResizeEvent>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionButton>
#include <QApplication>

#include "WulforUtil.h"

static int margin = 2;

TabButton::TabButton(QWidget *parent) :
    QPushButton(parent)
{
    setFlat(true);
    setCheckable(true);
    setAutoExclusive(true);

    parentHeight = QPushButton::sizeHint().height();

    label = new QLabel(this);
    label->setPixmap(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE).scaled(14, 14, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    label->setFixedSize(QSize(18, 18));
    label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    px_label = new QLabel(this);
    px_label->setFixedSize(QSize(18, 18));
    px_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    label->installEventFilter(this);

    updateGeometry();
}

void TabButton::resizeEvent(QResizeEvent *e){
    e->accept();

    updateGeometry();
}

bool TabButton::eventFilter(QObject *obj, QEvent *e){
    bool ret = QPushButton::eventFilter(obj, e);

    if (obj == static_cast<QLabel*>(obj)){
        if (e->type() == QEvent::MouseButtonPress)
            emit closeRequest();
    }

    return ret;
}

QSize TabButton::sizeHint() const {
    ensurePolished();

    int h = parentHeight;
    int w = QPushButton::sizeHint().width();
    QStyleOptionButton opt;

    initStyleOption(&opt);
    return (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h).expandedTo(QApplication::globalStrut()), this));
}

void TabButton::setWidgetIcon(const QPixmap &px){
    px_label->setPixmap(px.scaled(16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void TabButton::updateStyles() {
    label->setStyleSheet(QString("QLabel { margin-left: %1; }").arg(margin));
    px_label->setStyleSheet(QString("QLabel { margin-right: %1; }").arg(margin));
    setStyleSheet(QString("QPushButton { padding-right: %1; padding-left: %1;}").arg(label->width()));
}

void TabButton::updateGeometry() {
    label->setGeometry(width()-label->width()-margin*2, (height()-label->height())/2, 18, 18);
    px_label->setGeometry(margin*2, (height()-label->height())/2, 18, 18);

    updateStyles();
}

