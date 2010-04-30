#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>
#include <QLabel>
#include <QResizeEvent>
#include <QEvent>
#include <QPixmap>

class LineEdit : public QLineEdit
{
Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = 0);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual bool eventFilter(QObject *, QEvent *);
    virtual QSize sizeHint() const;

signals:
    void clearEdit();

private:
    void updateStyles();
    void updateGeometry();

    QLabel *label;
    QPixmap pxm;
};

#endif // LINEEDIT_H
