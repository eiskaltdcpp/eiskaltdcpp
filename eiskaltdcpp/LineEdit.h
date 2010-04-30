#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QLabel>
#include <QResizeEvent>
#include <QEvent>
#include <QPixmap>
#include <QFocusEvent>

class LineEdit : public QLineEdit
{
Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = 0);
    virtual ~LineEdit() { label->deleteLater(); }

    virtual QSize sizeHint() const;

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

signals:
    void clearEdit();

private:
    void updateStyles();
    void updateGeometry();

    QLabel *label;
    QPixmap pxm;
};

#endif // LINEEDIT_H
