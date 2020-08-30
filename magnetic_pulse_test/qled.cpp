#include "qled.h"

QLed::QLed(QWidget *parent)
    : QLabel(parent)
    , mState(false)
    , mColor(QLed::Color::green)
{
    mOff   = new QPixmap(":/icons/led-off.svg");
    mRed   = new QPixmap(":/icons/led-red.svg");
    mGreen = new QPixmap(":/icons/led-green.svg");

    setMaximumSize(24, 24);
    setAlignment(Qt::AlignCenter);
    setScaledContents(true);

    update();
}

void QLed::update() {
    if (mState) {
        if (mColor == QLed::Color::green) {
            setPixmap(*mGreen);
        } else if (mColor == QLed::Color::red) {
            setPixmap(*mRed);
        } else {
            setPixmap(*mOff);
        }
    } else {
        setPixmap(*mOff);
    }
}
