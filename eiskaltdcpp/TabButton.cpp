#include "TabButton.h"

#include <QResizeEvent>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionButton>
#include <QApplication>

#include "WulforUtil.h"

#include <QDataStream>

static const int margin         = 2;
static const int LABELWIDTH     = 18;
static const int CLOSEPXWIDTH   = 14;
static const int PXWIDTH        = 16;

int TabButton::maxWidth = 0;

TabButton::TabButton(QWidget *parent) :
    QPushButton(parent), isLeftBtnHold(false)
{
    setFlat(true);
    setCheckable(true);
    setAutoExclusive(true);
    setAutoDefault(false);
    setAcceptDrops(true);

    parentHeight = QPushButton::sizeHint().height();

    label = new QLabel(this);
    label->setPixmap(WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITDELETE).scaled(CLOSEPXWIDTH, CLOSEPXWIDTH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    label->setFixedSize(QSize(LABELWIDTH, LABELWIDTH));
    label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    px_label = new QLabel(this);
    px_label->setFixedSize(QSize(LABELWIDTH, LABELWIDTH));
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

    if (e->type() == QEvent::MouseButtonRelease){
        QMouseEvent *m_e = reinterpret_cast<QMouseEvent*>(e);

        if ((m_e->button() == Qt::MidButton) || (childAt(m_e->pos()) == static_cast<QWidget*>(label)))
            emit closeRequest();
    }

    return ret;
}

void TabButton::dragEnterEvent(QDragEnterEvent *event){
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void TabButton::dragMoveEvent(QDragMoveEvent *event){
    if (event->mimeData()->hasFormat("application/x-dnditemdata")) {
        if (event->source() == this) {
            event->setDropAction(Qt::MoveAction);
            event->accept();
        } else {
            event->acceptProposedAction();
        }
    } else {
        event->ignore();
    }
}

void TabButton::dropEvent(QDropEvent *e){
    if (qobject_cast<TabButton*>(e->source()) && this != qobject_cast<TabButton*>(e->source()))
        emit dropped(qobject_cast<TabButton*>(e->source()));

    e->ignore();
}

void TabButton::mousePressEvent(QMouseEvent *e){
    QPushButton::mousePressEvent(e);

    emit clicked();

    if (e->button() == Qt::LeftButton)
        isLeftBtnHold = true;
}

void TabButton::mouseMoveEvent(QMouseEvent *e){
    if (!isLeftBtnHold){
        QPushButton::mouseMoveEvent(e);

        return;
    }

    QPixmap pxm = QPixmap();
    pxm = QPixmap::grabWidget(this, rect());

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << pxm << QPoint(mapFromGlobal(QCursor::pos()));

    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/x-dnditemdata", data);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pxm);
    drag->setHotSpot(mapFromGlobal(QCursor::pos()));

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);

    e->accept();
}

void TabButton::mouseReleaseEvent(QMouseEvent *e){
    QPushButton::mouseReleaseEvent(e);

    isLeftBtnHold = false;
}

QSize TabButton::sizeHint() const {
    ensurePolished();

    int h = normalHeight();
    int w = normalWidth();

    return QSize(w, h);
}

int TabButton::normalWidth() const {
    QFontMetrics metrics = qApp->fontMetrics();

    return LABELWIDTH*2+metrics.width(text())+margin*3;
}

int TabButton::normalHeight() const {
#if QT_VERSION >= 0x040600
    return (LABELWIDTH+contentsMargins().top()+contentsMargins().bottom()+margin);
#else
    return (LABELWIDTH+2+margin);
#endif
}

void TabButton::setWidgetIcon(const QPixmap &px){
    px_label->setPixmap(px.scaled(PXWIDTH, PXWIDTH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void TabButton::updateStyles() {
    label->setStyleSheet(QString("QLabel { margin-left: %1; }").arg(margin));
    px_label->setStyleSheet(QString("QLabel { margin-right: %1; }").arg(margin));
    setStyleSheet(QString("QPushButton { padding-right: %1; padding-left: %1;}").arg(LABELWIDTH));
}

void TabButton::updateGeometry() {
    label->setGeometry(width()-LABELWIDTH-margin*2, (height()-LABELWIDTH)/2, LABELWIDTH, LABELWIDTH);
    px_label->setGeometry(margin*2, (height()-LABELWIDTH)/2, LABELWIDTH, LABELWIDTH);

    updateStyles();
}

