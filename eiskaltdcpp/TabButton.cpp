#include "TabButton.h"

#include <QResizeEvent>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionButton>
#include <QApplication>

#include "WulforUtil.h"

static int margin = 2;
int TabButton::maxWidth = 0;

TabButton::TabButton(QWidget *parent) :
    QPushButton(parent)
{
    setFlat(true);
    setCheckable(true);
    setAutoExclusive(true);
    setAutoDefault(false);

    parentHeight = QPushButton::sizeHint().height();

    label = new QLabel(this);
    label->setPixmap(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE).scaled(14, 14, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    label->setFixedSize(QSize(18, 18));
    label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    px_label = new QLabel(this);
    px_label->setFixedSize(QSize(18, 18));
    px_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    installEventFilter(this);
    label->installEventFilter(this);

    updateGeometry();
}

void TabButton::resizeEvent(QResizeEvent *e){
    e->accept();

    updateGeometry();
}

bool TabButton::eventFilter(QObject *obj, QEvent *e){
    bool ret = QPushButton::eventFilter(obj, e);

    if (e->type() == QEvent::MouseButtonPress){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if ((m_e->button() == Qt::MidButton) || (label == static_cast<QLabel*>(obj)))
            emit closeRequest();
    }

    return ret;
}

QSize TabButton::sizeHint() const {
    ensurePolished();

    int h = normalHeight();
    int w = normalWidth();

    return QSize(w, h);

    /*maxWidth = qMax(w, maxWidth);

    return QSize(maxWidth, h);*/

    /*QStyleOptionButton opt;

    initStyleOption(&opt);
    return (style()->sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h).expandedTo(QApplication::globalStrut()), this));*/
}

int TabButton::normalWidth() const {
    QFontMetrics metrics = qApp->fontMetrics();

    return label->width()+px_label->width()+metrics.width(text())+margin*3;
}

int TabButton::normalHeight() const {
#if QT_VERSION >= 0x040600
    return ((label->height()+px_label->height())/2)+contentsMargins().top()+contentsMargins().bottom()+margin;
#else
    return ((label->height()+px_label->height())/2)+2+margin;
#endif
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

