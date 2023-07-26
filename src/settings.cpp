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
#include "settings.hpp"
#include "qtilities.hpp"

#include <QApplication>
#include <QSettings>

#include <algorithm>

Qtilities::Settings::Settings()
    : alwaysOnTop_(Default::alwaysOnTop)
    , autoStart_(Default::autoStart)
    , cursorFilled_(Default::cursorFilled)
    , showCursor_(Default::showCursor)
    , cursorSize_(Default::cursorSize)
    , zoom_(Default::zoom)
    , bgColor_(Default::backgroundColor)
    , bdColor_(Default::borderColor)
    , position_(Default::position)
    , size_(Default::size)
{
}

void Qtilities::Settings::load()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(),
                       QApplication::applicationDisplayName());

    settings.beginGroup("General");
    alwaysOnTop_ = settings.value(QSL("AlwaysOnTop"), Default::alwaysOnTop).toBool();
    autoStart_ = settings.value(QSL("AutoStart"), Default::autoStart).toBool();
    cursorFilled_ = settings.value(QSL("CursorFilled"), Default::cursorFilled).toBool();
    showCursor_ = settings.value(QSL("ShowCursor"), Default::showCursor).toBool();

    int cursorSize = settings.value(QSL("CursorSize"), Default::cursorSize).toInt();
    int zoom = settings.value(QSL("Zoom"), Default::zoom).toInt();

    cursorSize_ = std::clamp(cursorSize, Limits::cursorSizeMin, Limits::cursorSizeMax);
    zoom_ = std::clamp(zoom, Limits::zoomMin, Limits::zoomMax);
    bgColor_ = settings.value(QSL("BackgroundColor"), Default::backgroundColor).value<QColor>();
    bdColor_ = settings.value(QSL("BorderColor"), Default::borderColor).value<QColor>();
    position_ = settings.value(QSL("Position"), Default::position).toPoint();
    size_ = settings.value(QSL("Size"), Default::size).toSize().expandedTo(Limits::sizeMin);
    settings.endGroup();
}

void Qtilities::Settings::save()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, QApplication::organizationName(),
                       QApplication::applicationDisplayName());

    settings.beginGroup("General");
    settings.setValue(QSL("AlwaysOnTop"), alwaysOnTop_);
    settings.setValue(QSL("AutoStart"), autoStart_);
    settings.setValue(QSL("CursorFilled"), cursorFilled_);
    settings.setValue(QSL("ShowCursor"), showCursor_);
    settings.setValue(QSL("CursorSize"), cursorSize_);
    settings.setValue(QSL("Zoom"), zoom_);
    settings.setValue(QSL("BackgroundColor"), bgColor_);
    settings.setValue(QSL("BorderColor"), bdColor_);
    settings.setValue(QSL("Position"), position_);
    settings.setValue(QSL("Size"), size_);
    settings.endGroup();
}
