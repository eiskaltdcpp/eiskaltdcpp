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
    void resetGeometry() { updateGeometry(); }
    int normalWidth() const;
    int normalHeight() const;

    static void setMaxWidth(int w) { maxWidth = (w >= 0)? w : 0; }

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
    static int maxWidth;
};

#endif // TABBUTTON_H
