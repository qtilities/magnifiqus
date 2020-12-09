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
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
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
MainWindow::MainWindow(QSystemTrayIcon *icon, QWidget *parent)
    : QMainWindow(parent)
    , actAbout     (new QAction(QIcon::fromTheme("help-about"), tr("&About"), this))
    , actAutoStart (new QAction(tr("Auto&start"), this))
    , actQuit      (new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this))
    , actTop       (new QAction(tr("Always on &Top"), this))
    , actionGroup  (new QActionGroup(this))
    , tmrDrag_     (new QTimer(this))
    , tmrUpdatePos_(new QTimer(this))
    , trayMenu     (new QMenu(this))
    , trayIcon     (icon)
    , blockEvents_ (false)
    , ratio_       (ratio_min)
{
    actAutoStart->setCheckable(true);
    actTop->setCheckable(true);

    for (int i = 0; i < 4; ++i)
    {
        actZoom[i] = new QAction("x&" + QString::number(i + 2) , this);
        actZoom[i]->setCheckable(true);
        if (i == 0)
            actZoom[i]->setChecked(true);

        actionGroup->addAction(actZoom[i]);
        trayMenu->addAction(actZoom[i]);
        connect(actZoom[i], &QAction::triggered, this, &MainWindow::onRatioSelected);
    }
    trayMenu->addSeparator();
    trayMenu->addAction(actTop);
    trayMenu->addAction(actAutoStart);
    trayMenu->addSeparator();
    trayMenu->addAction(actAbout);
    trayMenu->addSeparator();
    trayMenu->addAction(actQuit);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(qApp,     &QCoreApplication::aboutToQuit, this, &MainWindow::onAboutToQuit);
    connect(this,     &MainWindow::sigRatioChanged,   this, &MainWindow::onRatioChanged);
    connect(actAbout, &QAction::triggered,            this, &MainWindow::onAboutClicked);
    connect(actTop,   &QAction::triggered,            this, &MainWindow::onTopChecked);
    connect(actQuit,  &QAction::triggered,            qApp, &QCoreApplication::quit);

    connect(trayIcon, &QSystemTrayIcon::activated,
    [=](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            setVisible(!isVisible());
    });
    // FIXME: setMouseTracking doesn't make the thing work
    setMouseTracking(true);
    tmrDrag_->setSingleShot(true);
    connect(tmrDrag_, &QTimer::timeout, this, &MainWindow::setEndDragging);

    tmrUpdatePos_->setInterval(10);
    connect(tmrUpdatePos_, &QTimer::timeout, this, &MainWindow::updatePosition);

    setWindowIcon(QIcon(":/appicon"));
    setWindowTitle("x" + QString::number(ratio_));
    loadSettings();
}
MainWindow::~MainWindow()
{
}
void MainWindow::moveEvent(QMoveEvent *)
{
    tmrDrag_->start(50);
    blockEvents_= true;
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
void MainWindow::paintEvent(QPaintEvent *)
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
    QPainter painter(this);

    if (boundRect.contains(QCursor::pos()))
    {
        QPixmap p = QIcon(":/appicon").pixmap(w, h);
        painter.drawPixmap(0, 0, p);
        return;
    }
    painter.drawPixmap(0, 0, pixmap_.scaledToHeight(height() * ratio_));
}
void MainWindow::showEvent(QShowEvent *)
{
    updatePosition();
    tmrUpdatePos_->start();
}
void MainWindow::onAboutToQuit()
{
    actAutoStart->isChecked() ? createAutostartFile() : deleteAutostartFile();
    saveSettings();
}
void MainWindow::onAboutClicked()
{
    QFile f(":/about.html");
    if (!f.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << tr("Error loading about file") << Qt::endl;
        return;
    }
    QTextStream in(&f);
    QString text = in.readAll();
    f.close();
    QMessageBox::about(this, tr("About"), text);
}
void MainWindow::onRatioChanged(int value)
{
    blockSignals(true);
    actZoom[value - 2]->setChecked(true);
    blockSignals(false);
}
void MainWindow::onRatioSelected()
{
    int newValue;
    QObject *sender = this->sender();
    if (sender == actZoom[0])
        newValue = 2;
    else if (sender == actZoom[1])
        newValue = 3;
    else if (sender == actZoom[2])
        newValue = 4;
    else
        newValue = 5;

    blockSignals(true);
    setRatio(newValue);
    blockSignals(false);

    setWindowTitle("x" + QString::number(newValue));
}
void MainWindow::onTopChecked(bool checked)
{
    Qt::WindowFlags flags = windowFlags();
    if (checked)
        flags |= Qt::WindowStaysOnTopHint;
    else
        flags &= ~Qt::WindowStaysOnTopHint;
    setWindowFlags(flags);
    show();
}
void MainWindow::loadSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationDisplayName());

    settings.beginGroup("Main");
    move(settings.value("Position", QPoint(200, 200)).toPoint());
    resize(settings.value("Size", QSize(240, 240)).toSize());

    Qt::WindowFlags flags = windowFlags();
    bool            onTop = settings.value("AlwaysOnTop", true).toBool();
    if (onTop)
    {
        flags |= Qt::WindowStaysOnTopHint;
        actTop->setChecked(true);
    }
    else
    {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);

    actAutoStart->setChecked(settings.value("AutoStart", false).toBool());

    int ratio = settings.value("Zoom", 2).toUInt();
    if      (ratio < ratio_min) ratio = ratio_min;
    else if (ratio > ratio_max) ratio = ratio_max;
    setRatio(ratio);

    setWindowTitle("x" + QString::number(ratio));
    settings.endGroup();
}
void MainWindow::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationDisplayName());
    int zoom;
    QAction *act = actionGroup->checkedAction();
    for (int i = 0; i < 4; ++i)
    {
        if (actZoom[i] == act)
            zoom = i + 2;
    }
    settings.beginGroup("Main");
    settings.setValue("Position",    pos());
    settings.setValue("Size",        size());
    settings.setValue("AutoStart",   actAutoStart->isChecked());
    settings.setValue("AlwaysOnTop", actTop->isChecked());
    settings.setValue("Zoom",        zoom);
}
void MainWindow::createAutostartFile()
{
    QDir    configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    QString appName  = QApplication::applicationName();
    QString filePath = configDir.filePath("autostart/" + appName + ".desktop");
    QFile   file(filePath);

    if (file.exists() || !file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << "[Desktop Entry]\n";
    out << "Name=" + QApplication::applicationDisplayName() + "\n";
    out << "Type=Application\n";
    out << "Exec=" + appName + "\n";
    out << "Terminal=false\n";
}
void MainWindow::deleteAutostartFile()
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    QString filePath(configDir.filePath("autostart/" + QApplication::applicationName() + ".desktop"));
    QFile file(filePath);

    if (!file.exists())
        return;

    file.remove();
}
void MainWindow::setEndDragging()
{
    blockEvents_= false;
}
void MainWindow::setRatio(int value)
{
    if (value < ratio_min || value == ratio_ || value > ratio_max)
        return;

    ratio_= value;
    emit sigRatioChanged(value);
}
void MainWindow::updatePosition()
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
