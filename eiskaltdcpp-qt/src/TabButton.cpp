#include "TabButton.h"

#include <QResizeEvent>
#include <QLabel>
#include <QEvent>
#include <QMouseEvent>
#include <QStyleOptionButton>
#include <QApplication>

#include "WulforUtil.h"
#include "WulforSettings.h"

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

    if (e->button() == Qt::LeftButton){
        emit clicked();

        isLeftBtnHold = true;
    }
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

    if (WBGET(WB_APP_TBAR_SHOW_CL_BTNS))
        return LABELWIDTH*2+metrics.width(text())+margin*3;
    else
        return LABELWIDTH+metrics.width(text())+margin*5;
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

    QString styleText_pressed = "QPushButton:checked {\n";
    QString styleText_button = "QPushButton {\n";

    if (WBGET(WB_APP_TBAR_SHOW_CL_BTNS)){
        styleText_pressed += QString("padding-right: %1;\n padding-left: %1;\n").arg(LABELWIDTH);
        styleText_button += QString("padding-right: %1;\n padding-left: %1;\n").arg(LABELWIDTH);
    }
    else{
        styleText_pressed += QString("padding-left: %1;\n padding-left: %1;\n").arg(LABELWIDTH);
        styleText_button += QString("padding-left: %1;\n padding-left: %1;\n").arg(LABELWIDTH);
    }

    styleText_pressed += QString("border-width: 1px; border-color: %1; border-style: outset; border-radius: 5;\n").arg(palette().highlight().color().name());

    styleText_button    += "}\n";
    styleText_pressed   += "}\n";

    setStyleSheet(styleText_button + styleText_pressed);
}

void TabButton::updateGeometry() {
    if (WBGET(WB_APP_TBAR_SHOW_CL_BTNS)){
        if (!label->isVisible())
            label->show();

        label->setGeometry(width()-LABELWIDTH-margin*2, (height()-LABELWIDTH)/2, LABELWIDTH, LABELWIDTH);
    }
    else
        label->hide();

    px_label->setGeometry(margin*2, (height()-LABELWIDTH)/2, LABELWIDTH, LABELWIDTH);

    updateStyles();
}

