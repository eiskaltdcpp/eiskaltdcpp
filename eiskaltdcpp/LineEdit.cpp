#include "LineEdit.h"
#include "WulforUtil.h"

#include <QMouseEvent>
#include <QApplication>
#include <QStyle>
#include <QtDebug>

static const int margin = 3;

LineEdit::LineEdit(QWidget *parent) :
        QLineEdit(parent), menu(NULL), role(LineEdit::InsertText)
{
    pxm = WulforUtil::getInstance()->getPixmap(WulforUtil::eiEDITCLEAR);

    parentHeight = QLineEdit::sizeHint().height();//save parent height before setting up new stylesheet
                                                  //because we losing top and bottom margins
    label = new QLabel(this);
    label->setPixmap(pxm);
    label->setCursor(Qt::ArrowCursor);
    label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
    label->installEventFilter(this);

    connect(this, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged()));

    updateGeometry();
    updateStyles();

    slotTextChanged();
}

LineEdit::~LineEdit(){
    label->deleteLater();

    if (menu)
        menu->deleteLater();
}

void LineEdit::resizeEvent(QResizeEvent *e){
    e->accept();

    updateGeometry();
}

void LineEdit::focusInEvent(QFocusEvent *e){
    QLineEdit::focusInEvent(e);
}

void LineEdit::focusOutEvent(QFocusEvent *e){
    QLineEdit::focusOutEvent(e);
}

bool LineEdit::eventFilter(QObject *obj, QEvent *e){
    if (qobject_cast<QLabel*>(obj) != label)
        return QLineEdit::eventFilter(obj, e);

    switch (e->type()){
    case QEvent::MouseButtonPress:
        {
            if (!menu)
                clear();
            else{
                QAction *act = menu->exec(QCursor::pos());

                if (act && role == InsertText)
                    setText(act->text());
                else if (act)
                    emit menuAction(act);
            }

            break;
        }
    default:
        break;
    }

    return QLineEdit::eventFilter(obj, e);
}

QSize LineEdit::sizeHint() const{
    ensurePolished();

    int h = parentHeight;
    int w = QLineEdit::sizeHint().width();
    QStyleOptionFrameV2 opt;

    initStyleOption(&opt);
    return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, QSize(w, h).expandedTo(QApplication::globalStrut()), this));
}

QSizePolicy LineEdit::sizePolicy() const{
    return QLineEdit::sizePolicy();
}

void LineEdit::updateGeometry(){
    label->setGeometry(width()-pxm.width()-margin*2, 0, pxm.width()+margin, height());
}

void LineEdit::updateStyles(){
    label->setStyleSheet(QString("QLabel { margin-left: %1; }").arg(margin));
    setStyleSheet(QString("QLineEdit{ padding-right: %1; }").arg(label->width()+margin));
}

void LineEdit::setPixmap(const QPixmap &px){
    pxm = px;

    label->setPixmap(px);

    updateGeometry();
    updateStyles();
}

void LineEdit::setMenu(QMenu *m){
    if (menu)
        menu->deleteLater();

    menu = m;

    if (menu){
        menu->setParent(NULL);

        slotTextChanged();
    }
}

void LineEdit::setMenuRole(LineEdit::MenuRole r){
    role = r;
}

void LineEdit::slotTextChanged(){
    if (menu || !text().isEmpty()){
        label->show();

        updateGeometry();
    }
    else
        label->hide();
}
