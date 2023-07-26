/*
    MIT License

    Copyright (c) 2023 Andrea Zanellato <redtid3@gmail.com>

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/
#include "systemtrayicon.hpp"
#include "application.hpp"

#include <QAction>
#include <QIcon>
#include <QMenu>
#include <QSystemTrayIcon>

Qtilities::SystemTrayIcon::SystemTrayIcon(const QIcon &icon, QObject *parent)
    : QObject{parent}
{
    init();
    trayIcon_->setIcon(icon);
}

Qtilities::SystemTrayIcon::SystemTrayIcon(QObject *parent)
    : QObject{parent}
{
    init();
}

void Qtilities::SystemTrayIcon::setIcon(const QIcon &icon) { trayIcon_->setIcon(icon); }

void Qtilities::SystemTrayIcon::setWidget(QWidget *widget)
{
    if (!widget)
        return;

    connect(trayIcon_, &QSystemTrayIcon::activated, this,
            [widget](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                    widget->setVisible(widget->isHidden());
            });
}

void Qtilities::SystemTrayIcon::init()
{
    trayIcon_ = new QSystemTrayIcon(this);
    trayMenu_ = new QMenu;
    actAbout_ = new QAction(QIcon::fromTheme("help-about", QIcon(":/help-about")), tr("&About"), this);
    actPrefs_ = new QAction(QIcon::fromTheme("preferences-system", QIcon(":/preferences-system")),
                            tr("&Preferences"), this);
    actQuit_ = new QAction(QIcon::fromTheme("application-exit", QIcon(":/application-exit")),
                           tr("&Quit"), this);

    trayMenu_->addAction(actAbout_);
    trayMenu_->addAction(actPrefs_);
    trayMenu_->addAction(actQuit_);
    trayIcon_->setContextMenu(trayMenu_);
    trayIcon_->show();

    Application *theApp = static_cast<Application *>(qApp);
    connect(actAbout_, &QAction::triggered, theApp, &Application::about);
    connect(actPrefs_, &QAction::triggered, theApp, &Application::preferences);
    connect(actQuit_, &QAction::triggered, qApp, &QCoreApplication::quit);

    connect(qApp, &QApplication::aboutToQuit, trayIcon_, &QObject::deleteLater);
    connect(qApp, &QApplication::aboutToQuit, trayMenu_, &QObject::deleteLater);
}
