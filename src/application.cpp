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
#include "application.hpp"
#include "mainwindow.hpp"
#include "dialogabout.hpp"
#include "dialogprefs.hpp"
#include "systemtrayicon.hpp"

#include <QLibraryInfo>
#include <QLocale>

Qtilities::Application::Application(int argc, char *argv[])
    : QApplication(argc, argv)
    , trayIcon_(new SystemTrayIcon(this))
{
    setApplicationName(PROJECT_ID);
    setApplicationDisplayName(APPLICATION_NAME);
    setOrganizationName(ORGANIZATION_NAME);
    setOrganizationDomain(ORGANIZATION_DOMAIN);

    setQuitOnLastWindowClosed(false);

    initLocale();
    initUi();
}

void Qtilities::Application::initLocale()
{
#if 0
    QLocale locale = QLocale::system();
#else
    QLocale locale(QLocale("it"));
    QLocale::setDefault(locale);
#endif
    // Qt translations (buttons and the like)
    QString translationsPath
#if QT_VERSION < 0x060000
        = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#else
        = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#endif
    QString translationsFileName = QStringLiteral("qt_") + locale.name();

    if (qtTranslator_.load(translationsFileName, translationsPath))
        installTranslator(&qtTranslator_);

    // E.g. "<appname>_en"
    translationsFileName = QCoreApplication::applicationName().toLower() + '_' + locale.name();

    // Try first in the same binary directory, in case we are building,
    // otherwise read from system data
    translationsPath = QCoreApplication::applicationDirPath();

    bool isLoaded = translator_.load(translationsFileName, translationsPath);
    if (!isLoaded) {
        // "/usr/share/<appname>/translations
        isLoaded = translator_.load(translationsFileName,
                                    QStringLiteral(PROJECT_DATA_DIR) + QStringLiteral("/translations"));
    }
    if (isLoaded)
        installTranslator(&translator_);
}

void Qtilities::Application::initUi()
{
    // UseHighDpiPixmaps is default from Qt6
#if QT_VERSION < 0x060000
    setAttribute(Qt::AA_UseHighDpiPixmaps, true);
#endif
    QString icoCurPath
        = QCoreApplication::applicationDirPath() + '/' + QStringLiteral(PROJECT_ICON_NAME);
    QString icoSysPath = QStringLiteral(PROJECT_ICON_SYSTEM_PATH);

    // Try first to find the app icon in the current directory
    appIcon_ = QIcon(icoCurPath);
    if (appIcon_.isNull())
        appIcon_ = QIcon(icoSysPath);

    settings_.load();

    mainWindow_ = new Qtilities::MainWindow;
    mainWindow_->loadSettings();
    mainWindow_->move(settings_.position());
    mainWindow_->resize(settings_.size());
    mainWindow_->setWindowIcon(appIcon_);
    mainWindow_->setWindowTitle(applicationDisplayName());
    mainWindow_->show(); // FIXME: Qt5 crash without this
    mainWindow_->hide();

    trayIcon_->setIcon(appIcon_);
    trayIcon_->setWidget(mainWindow_);

    connect(this, &QApplication::aboutToQuit, trayIcon_, &QObject::deleteLater);
    connect(this, &QApplication::aboutToQuit, mainWindow_, &QObject::deleteLater);
    connect(this, &QApplication::aboutToQuit, this, [this]() {
        mainWindow_->saveSettings();
        settings_.save();
    });
}

void Qtilities::Application::about()
{
    DialogAbout about(mainWindow_);
    about.exec();
}

void Qtilities::Application::preferences()
{
    DialogPrefs prefs(mainWindow_);
    connect(&prefs, &QDialog::accepted, mainWindow_, &MainWindow::loadSettings);
    prefs.loadSettings();
    prefs.exec();
}

QMenu *Qtilities::Application::getMenu() const { return trayIcon_->getMenu(); }

int main(int argc, char *argv[])
{
    Qtilities::Application app(argc, argv);
    return app.exec();
}
