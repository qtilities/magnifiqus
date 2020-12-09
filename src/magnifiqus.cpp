#include "magnifiqus.h"

#include <QApplication>
#include <QCursor>
#include <QDesktopWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>
#include <QShowEvent>
#include <QTimer>

static QScreen *findScreenAt(const QPoint &pos)
{
    for (QScreen *screen : QGuiApplication::screens())
    {
        if (screen->geometry().contains(pos))
            return screen;
    }
    return nullptr;
}
Magnifiqus::Magnifiqus(QWidget *parent)
    : QWidget(parent)
    , timer_(new QTimer(this))
    , ratio_(ratio_min)
    , blockEvents_(false)
{
    timer_->setInterval(10);
    connect(timer_, &QTimer::timeout, this, &Magnifiqus::updatePosition);
}
void Magnifiqus::setRatio(int value)
{
    if (value < ratio_min || value == ratio_ || value > ratio_max)
        return;

    ratio_= value;
    emit sigRatioChanged(value);
}
int Magnifiqus::ratio() const
{
    return ratio_;
}
void Magnifiqus::blockEvents(bool block)
{
    blockEvents_= block;
}
void Magnifiqus::showEvent(QShowEvent *)
{
    updatePosition();
    timer_->start();
}
void Magnifiqus::paintEvent(QPaintEvent *)
{
    if (blockEvents_)
        return;

    int    w         = width();
    int    h         = height();
    int    hBound    = w  / ratio_ / 2;
    int    vBound    = h  / ratio_ / 2;
    QPoint topL      = mapToGlobal(rect().topLeft());
    QRect  boundRect = QRect(topL.x() - hBound,
                             topL.y() - vBound * 2,
                             w + hBound * 2,
                             h + vBound * 2);
    if (boundRect.contains(QCursor::pos()))
        return;

    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap_.scaledToHeight(height() * ratio_));
}
void Magnifiqus::updatePosition()
{
    if (blockEvents_)
        return;

    QPoint   pos    = QCursor::pos();
    int      w      = width();
    int      h      = height();
    QScreen *screen = findScreenAt(pos);
    if (!screen)
        return;

    WId wid = QApplication::desktop()->winId();
    pixmap_ = screen->grabWindow(wid,
                                 pos.x() - w / ratio_ / 2,
                                 pos.y() - h / ratio_ / 2, w, h);
    update();
}
