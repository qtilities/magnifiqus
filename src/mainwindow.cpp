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
#include "ui_mainwindow.h"
#include "magnifiqus.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QSystemTrayIcon>
#include <QTimer>

MainWindow::MainWindow(QSystemTrayIcon *icon, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , actionGroup(new QActionGroup(this))
    , timer_(new QTimer(this))
    , trayMenu(new QMenu(this))
    , trayIcon(icon)
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->layout);

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
    trayMenu->addAction(ui->actTop);
    trayMenu->addAction(ui->actAutoStart);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->actAbout);
    trayMenu->addSeparator();
    trayMenu->addAction(ui->actQuit);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(ui->magnifiqus, &Magnifiqus::sigRatioChanged, this, &MainWindow::onRatioChanged);
    connect(ui->actTop,   &QAction::triggered, this, &MainWindow::onTopChecked);
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::onAboutClicked);
    connect(ui->actQuit,  &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::onAboutToQuit);

    connect(trayIcon, &QSystemTrayIcon::activated,
    [=](QSystemTrayIcon::ActivationReason reason)
    {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
            setVisible(!isVisible());
    });
    // FIXME: setMouseTracking doesn't make the thing work
    setMouseTracking(true);
    timer_->setSingleShot(true);
    connect(timer_, &QTimer::timeout, this, &MainWindow::setEndDragging);

    setWindowIcon(QIcon(":/appicon"));
    setWindowTitle("x" + QString::number(ui->magnifiqus->ratio()));
    loadSettings();
}
MainWindow::~MainWindow()
{
}
void MainWindow::moveEvent(QMoveEvent *)
{
    timer_->start(50);
    ui->magnifiqus->blockEvents(true);
}
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}
void MainWindow::onAboutToQuit()
{
    ui->actAutoStart->isChecked() ? createAutostartFile() : deleteAutostartFile();
    saveSettings();

    delete ui;
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
    ui->magnifiqus->blockSignals(true);
    actZoom[value - 2]->setChecked(true);
    ui->magnifiqus->blockSignals(false);
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

    ui->magnifiqus->blockSignals(true);
    ui->magnifiqus->setRatio(newValue);
    ui->magnifiqus->blockSignals(false);

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

    Qt::WindowFlags flags = windowFlags();
    bool            onTop = settings.value("AlwaysOnTop", true).toBool();
    if (onTop)
    {
        flags |= Qt::WindowStaysOnTopHint;
        ui->actTop->setChecked(true);
    }
    else
    {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);

    ui->actAutoStart->setChecked(settings.value("AutoStart", false).toBool());

    int ratio = settings.value("Zoom", 2).toUInt();
    if      (ratio < Magnifiqus::ratio_min) ratio = Magnifiqus::ratio_min;
    else if (ratio > Magnifiqus::ratio_max) ratio = Magnifiqus::ratio_max;
    ui->magnifiqus->setRatio(ratio);

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
    settings.setValue("Position", pos());
    settings.setValue("AutoStart",   ui->actAutoStart->isChecked());
    settings.setValue("AlwaysOnTop", ui->actTop->isChecked());
    settings.setValue("Zoom", zoom);
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
    ui->magnifiqus->blockEvents(false);
}
