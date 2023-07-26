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

#include <QColor>
#include <QPoint>
#include <QSize>

namespace Qtilities {

namespace Default {
static constexpr bool alwaysOnTop = true;
static constexpr bool autoStart = false;
static constexpr bool cursorFilled = false;
static constexpr bool showCursor = false;
static constexpr int cursorSize = 1;
static constexpr int zoom = 2;
static constexpr QPoint position = QPoint(200, 200);
static constexpr QSize size = QSize(240, 120);
static constexpr QColor backgroundColor = QColor(35, 35, 35);
static constexpr QColor borderColor = QColor(18, 18, 18);
} // namespace Default

namespace Limits {
static constexpr int cursorSizeMin = 1;
static constexpr int cursorSizeMax = 100;
static constexpr QSize sizeMin = QSize(30, 30);
static constexpr int zoomMin = Default::zoom;
static constexpr int zoomMax = 5;
} // namespace Limits

class Settings
{
public:
    Settings();

    void load();
    void save();

    bool alwaysOnTop() const { return alwaysOnTop_; }
    void setAlwaysOnTop(bool alwaysOnTop) { alwaysOnTop_ = alwaysOnTop; }

    bool autoStart() const { return autoStart_; }
    void setAutoStart(bool autoStart) { autoStart_ = autoStart; }

    bool cursorFilled() const { return cursorFilled_; }
    void setCursorFilled(bool filled) { cursorFilled_ = filled; }

    bool showCursor() const { return showCursor_; }
    void setShowCursor(bool showCursor) { showCursor_ = showCursor; }

    int cursorSize() const { return cursorSize_; }
    void setCursorSize(int cursorSize) { cursorSize_ = cursorSize; }

    int zoom() const { return zoom_; }
    void setZoom(int zoom) { zoom_ = zoom; }

    QColor backgroundColor() const { return bgColor_; }
    void setBackgroundColor(const QColor &bgColor) { bgColor_ = bgColor; }

    QColor borderColor() const { return bdColor_; }
    void setBorderColor(const QColor &bdColor) { bdColor_ = bdColor; }

    QPoint position() const { return position_; }
    void setPosition(const QPoint &position) { position_ = position; }

    QSize size() const { return size_; }
    void setSize(const QSize &size) { size_ = size; }

private:
    bool alwaysOnTop_;
    bool autoStart_;
    bool cursorFilled_;
    bool showCursor_;
    int cursorSize_;
    int zoom_;
    QColor bgColor_;
    QColor bdColor_;
    QPoint position_;
    QSize size_;
};
} // namespace Qtilities
