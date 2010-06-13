#ifndef TABBUTTON_H
#define TABBUTTON_H

#include <QPushButton>

class QResizeEvent;
class QLabel;
class QEvent;

class TabButton : public QPushButton
{
Q_OBJECT
public:
    explicit TabButton(QWidget *parent = 0);

    QSize sizeHint() const;
    void setWidgetIcon(const QPixmap &px);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual bool eventFilter(QObject *, QEvent *);

signals:
    void closeRequest();

private:
    void updateStyles();
    void updateGeometry();

    QLabel *label;
    QLabel *px_label;
    int parentHeight;
};

#endif // TABBUTTON_H
