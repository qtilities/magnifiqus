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
#pragma once
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QActionGroup;
class QSystemTrayIcon;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSystemTrayIcon* icon, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void moveEvent(QMoveEvent *);
    void closeEvent(QCloseEvent *);
    void onAboutToQuit();

    void onAboutClicked();
    void onRatioChanged(int);
    void onRatioSelected();
    void onTopChecked(bool);

    void loadSettings();
    void saveSettings();

    void createAutostartFile();
    void deleteAutostartFile();

    void setEndDragging();

    Ui::MainWindow  *ui;
    QAction         *actZoom[4];
    QActionGroup    *actionGroup;
    QTimer          *timer_;
    QMenu           *trayMenu;
    QSystemTrayIcon *trayIcon;
};
