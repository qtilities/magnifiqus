/*
    Magnifiqus - Screen magnifiers made in Qt
    Copyright (C) 2020-2023 Andrea Zanellato <redtid3@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "mainwindow.hpp"
#include "application.hpp"
#include "qtilities.hpp"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include "x11.hpp"

namespace Qtilities {
static constexpr int borderOffset = 20;
static constexpr int zoomFontSize = 24;
} // namespace Qtilities

Qtilities::MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , tmrShowZoom_(new QTimer(this))
    , tmrUpdatePos_(new QTimer(this))
    , dragType_(DragNone)
    , zoomChanged_(false)
{
    tmrUpdatePos_->setInterval(10);
    tmrShowZoom_->setInterval(1000);

    setMouseTracking(true);
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::FramelessWindowHint);
    loadSettings();

    connect(tmrUpdatePos_, &QTimer::timeout, this, &MainWindow::updateGrabFollowMouse);
    connect(tmrShowZoom_, &QTimer::timeout, this, [this] { zoomChanged_ = false; });
}

void Qtilities::MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        lastPoint_ = pos;
        dragType_ = isOverResizeCorner(pos) ? DragResize : DragMove;
    } else if (event->button() == Qt::RightButton) {
        Application *theApp = static_cast<Application *>(qApp);
        theApp->getMenu()->exec(this->cursor().pos());
    }
    QMainWindow::mousePressEvent(event);
}

void Qtilities::MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && dragType_ != DragNone)
        dragType_ = DragNone;
}

void Qtilities::MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    if (dragType_ == DragResize || isOverResizeCorner(pos))
        setCursor(Qt::SizeFDiagCursor);
    else
        setCursor(Qt::ArrowCursor);

    if ((event->buttons() & Qt::LeftButton) && dragType_ != DragNone) {
        if (dragType_ == DragResize) {
            resize(qMax(Limits::sizeMin.width(), pos.x()), qMax(Limits::sizeMin.height(), pos.y()));
        } else {
#if QT_VERSION < 0x060000
            move(event->globalX() - lastPoint_.x(), event->globalY() - lastPoint_.y());
#else
            move(event->globalPosition().x() - lastPoint_.x(),
                 event->globalPosition().y() - lastPoint_.y());

#endif
        }
        update();
    }
    QMainWindow::mouseMoveEvent(event);
}

void Qtilities::MainWindow::paintEvent(QPaintEvent *)
{
    Application *theApp = static_cast<Application *>(qApp);
    int w = width();
    int h = height();
    int zoom = theApp->settings().zoom();
    int hBound = w / zoom / 2;
    int vBound = h / zoom / 2;
    QPoint topL = mapToGlobal(rect().topLeft());
    QRect boundRect = QRect(topL.x() - hBound, topL.y() - vBound, w + hBound * 2, h + vBound * 2);
    QPen pen(theApp->settings().borderColor());
    QPainter painter(this);

    if (boundRect.contains(QCursor::pos())) {
        // Avoid to get a "mirror on mirror" effect
        QPixmap &pixmap = windowOverlayPixmap_;

        if (pixmap.width() != w || pixmap.height() != h)
            pixmap = theApp->icon().pixmap(w, h);

        painter.fillRect(rect(), theApp->settings().backgroundColor());
        painter.drawPixmap(0, 0, pixmap);
    } else {
        // Draw the cached scaled pixmap
        painter.scale(zoom, zoom);
        painter.drawPixmap(0, 0, pixmap_);
        painter.resetTransform();

        if (theApp->settings().showCursor()) {
            // Draw pointer coords rect
            int cursorSize = theApp->settings().cursorSize();
            float zoomedCursorSize = zoom * cursorSize;
            QRectF cursorRect(rect().x() + (w / 2) - zoomedCursorSize / 2,
                              rect().y() + (h / 2) - zoomedCursorSize / 2, zoomedCursorSize,
                              zoom * cursorSize);
            pen.setWidth(1);
            painter.setPen(Qt::white);
            painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            painter.drawRect(cursorRect);

            if (theApp->settings().cursorFilled())
                painter.fillRect(cursorRect, Qt::white);

            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }
    // Draw the frame border
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(QRect(rect().x() + 1, rect().y() + 1, rect().width() - 2, rect().height() - 2));
    if (zoomChanged_) {
        // Notify the changed zoom on the top right corner using the mouse wheel
        painter.setPen(Qt::white);
        QFont font(this->font());
        font.setPointSize(zoomFontSize);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect().topRight().x() - 42, rect().topRight().y() + 28,
                         "x" + QString::number(zoom));
    }
}

void Qtilities::MainWindow::showEvent(QShowEvent *)
{
    if (!isWayland())
        x11::dontShowInTaskbar(this);

    updateGrabFollowMouse();
    tmrUpdatePos_->start();
}

void Qtilities::MainWindow::hideEvent(QHideEvent *) { tmrUpdatePos_->stop(); }

void Qtilities::MainWindow::wheelEvent(QWheelEvent *event)
{
    Settings &settings = static_cast<Application *>(qApp)->settings();
    QPoint degrees = event->angleDelta() / 8;
    if (!degrees.isNull()) {
        int oldZoom = settings.zoom();
        int zoom = degrees.y() / 15 + oldZoom;
        zoom = std::clamp(zoom, Limits::zoomMin, Limits::zoomMax);

        if (zoom == oldZoom)
            return;

        settings.setZoom(zoom);
        zoomChanged_ = true;
        tmrShowZoom_->start();
    }
    event->accept();
}

QSize Qtilities::MainWindow::minimumSizeHint() const { return Limits::sizeMin; }

QSize Qtilities::MainWindow::sizeHint() const { return Limits::sizeMin; }

void Qtilities::MainWindow::loadSettings()
{
    Settings &settings = static_cast<Application *>(qApp)->settings();
    Qt::WindowFlags flags = windowFlags();

    if (settings.alwaysOnTop())
        flags |= Qt::WindowStaysOnTopHint;
    else
        flags &= ~Qt::WindowStaysOnTopHint;

    setWindowFlags(flags);
}

void Qtilities::MainWindow::saveSettings()
{
    Settings &settings = static_cast<Application *>(qApp)->settings();
    settings.setPosition(pos());
    settings.setSize(size());
}

void Qtilities::MainWindow::updateGrabFollowMouse()
{
    QPoint pos = QCursor::pos();
    QScreen *screen = findScreenAt(pos);

    if (!screen)
        return;

    Settings &settings = static_cast<Application *>(qApp)->settings();
    int zoom = settings.zoom();
    int w = width();
    int h = height();
    QPoint grabCenter = pos;
    QRect grabLimits
        = screen->geometry().adjusted(w / (2 * zoom), h / (2 * zoom), -w / (2 * zoom), -h / (2 * zoom));

    grabCenter.setX(qMax(grabLimits.left(), qMin(grabLimits.right(), grabCenter.x())));
    grabCenter.setY(qMax(grabLimits.top(), qMin(grabLimits.bottom(), grabCenter.y())));

    QRect grabArea{grabCenter.x() - w / (2 * zoom), grabCenter.y() - h / (2 * zoom), w, h};
    pixmap_ = screen->grabWindow(x11::desktop(), grabArea.x(), grabArea.y(), grabArea.width(),
                                 grabArea.height());
    update();
}

bool Qtilities::MainWindow::isOverResizeCorner(const QPoint &pos) const
{
    QPoint bottomRight = rect().bottomRight();
    return pos.x() > bottomRight.x() - borderOffset && pos.y() > bottomRight.y() - borderOffset;
}
