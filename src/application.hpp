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

#include "settings.hpp"

#include <QApplication>
#include <QIcon>
#include <QTranslator>

class QMenu;

namespace Qtilities {

class MainWindow;
class SystemTrayIcon;
class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int argc, char *argv[]);
    void about();
    void preferences();
    QIcon icon() const { return appIcon_; }
    Settings &settings() { return settings_; }
    QMenu *getMenu() const;

private:
    void initLocale();
    void initUi();

    QTranslator qtTranslator_, translator_;
    Settings settings_;
    SystemTrayIcon *trayIcon_;
    MainWindow *mainWindow_;
    QIcon appIcon_;
};
} // namespace Qtilities
