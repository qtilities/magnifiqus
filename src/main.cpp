/*
    Copyright (C) 2020 Andrea Zanellato <redtid3@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    For a full copy of the GNU General Public License see the LICENSE file
*/
#include "mainwindow.h"

#include <QApplication>
#include <QSystemTrayIcon>

int main(int argc, char* argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);

    app.setOrganizationName("AZDrums");
    app.setOrganizationDomain("azdrums.org");
    app.setApplicationName("magnifiqus");
    app.setApplicationDisplayName("Magnifiqus");

    QIcon icon = QIcon(":/trayicon");
    QSystemTrayIcon* trayIcon = new QSystemTrayIcon(icon);
    trayIcon->setVisible(true);

    MainWindow w(trayIcon);

    return app.exec();
}
