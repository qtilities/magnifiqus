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
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
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
    void setRatio(int);
    void updatePosition();

    static constexpr int ratio_min = 2;
    static constexpr int ratio_max = 5;

    QAction         *actAbout,
                    *actAutoStart,
                    *actQuit,
                    *actTop,
                    *actZoom[4];
    QActionGroup    *actionGroup;
    QPixmap         pixmap_;
    QTimer          *tmrDrag_;
    QTimer          *tmrUpdatePos_;
    QMenu           *trayMenu;
    QSystemTrayIcon *trayIcon;
    bool            blockEvents_;
    int             ratio_;

signals:
    void sigRatioChanged(int);
};
