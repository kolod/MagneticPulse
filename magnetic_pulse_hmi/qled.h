#ifndef QLED_H
#define QLED_H

#include <QLabel>
#include <QWidget>
#include <QPixmap>

class QLed : public QLabel
{
    Q_OBJECT

    Q_ENUMS(Color)
    Q_PROPERTY(Color color READ color WRITE setColor)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit QLed(QWidget *parent=nullptr);
    enum Color {green, red};
    Color color() const {return mColor;}

public slots:

    void setState(bool state)      {
        if (mState != state) {
            mState = state;
            update();
        }
    }

    void setColor(QLed::Color color) {
        mColor = color;
        update();
    }

private:
    bool mState;
    QLed::Color mColor;

    QPixmap *mOff;
    QPixmap *mRed;
    QPixmap *mGreen;

    void update();
};

#endif // QLED_H
