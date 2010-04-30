#include "LineEdit.h"
#include "WulforUtil.h"

#include <QMouseEvent>

static const int margin = 3;

LineEdit::LineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    pxm = WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITCLEAR);

    label = new QLabel(this);
    label->setPixmap(pxm);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    label->installEventFilter(this);

    connect(this, SIGNAL(clearEdit()), this, SLOT(clear()));

    //setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    updateGeometry();
    updateStyles();
}

void LineEdit::resizeEvent(QResizeEvent *e){
    e->accept();

    updateGeometry();
    updateStyles();
}

bool LineEdit::eventFilter(QObject *obj, QEvent *e){
    if (qobject_cast<QLabel*>(obj) != label)
        return QLineEdit::eventFilter(obj, e);

    switch (e->type()){
    case QEvent::MouseButtonPress:
        {
            emit clearEdit();

            break;
        }
    default:
        break;
    }

    return QLineEdit::eventFilter(obj, e);
}

QSize LineEdit::sizeHint() const{
    return QSize(100, 22);
}

void LineEdit::updateGeometry(){
    label->setGeometry(width()-pxm.width()-margin*2, -1+(height()-pxm.height())/2, pxm.width()+margin, height());
}

void LineEdit::updateStyles(){
    pxm = WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITCLEAR);

    label->setStyleSheet(QString("QLabel { margin-left: %1; }").arg(margin));
    setStyleSheet(QString("QLineEdit{ padding-right: %1; }").arg(label->width()+margin));

    label->setPixmap(pxm);
}
