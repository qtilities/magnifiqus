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
#pragma once

#include <QMainWindow>

namespace Qtilities {

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum DragType { DragNone, DragMove, DragResize };

public:
    MainWindow(QWidget *parent = nullptr);

    void loadSettings();
    void saveSettings();

private:
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void wheelEvent(QWheelEvent *) override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    bool isOverResizeCorner(const QPoint &) const;
    void updateGrabFollowMouse();

    QPixmap pixmap_;
    QPixmap windowOverlayPixmap_;
    QPoint lastPoint_;
    QTimer *tmrShowZoom_, *tmrUpdatePos_;
    DragType dragType_;

    int cursorSize_;
    int zoom_;
    bool zoomChanged_;
};
} // namespace Qtilities
