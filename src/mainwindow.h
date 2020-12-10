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
class QMenu;
class QSystemTrayIcon;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QSystemTrayIcon* icon, QWidget *parent = nullptr);
    ~MainWindow();

private:
    void closeEvent(QCloseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
    void mouseMoveEvent(QMouseEvent *) override;
    void paintEvent(QPaintEvent *) override;
    void showEvent(QShowEvent *) override;
    void hideEvent(QHideEvent *) override;
    void wheelEvent(QWheelEvent *) override;
    void onAboutToQuit();

    void onAboutClicked();
    void onRatioChanged(int);
    void onRatioSelected();
    void onTopChecked(bool);

    void loadSettings();
    void saveSettings();

    void createAutostartFile();
    void deleteAutostartFile();

    void setRatio(int);
    void updatePosition();
    void notifyRatioComplete();

    const QPixmap &getWindowOverlayPixmap();
    bool isOverResizeCorner(QPoint pos) const;

    static constexpr int ratio_min = 2;
    static constexpr int ratio_max = 5;

    static constexpr int size_min = 30;

    enum DragType
    {
        DragNone,
        DragMove,
        DragResize,
    };

    QAction         *actAbout,
                    *actAutoStart,
                    *actQuit,
                    *actTop,
                    *actZoom[4];
    QActionGroup    *actionGroup;
    QPixmap         pixmap_;
    QPixmap         windowOverlayPixmap_;
    QPoint          lastPoint_;
    QTimer          *tmrShowRatio_,
                    *tmrUpdatePos_;
    QMenu           *trayMenu;
    QSystemTrayIcon *trayIcon;
    DragType        dragType_;
    bool            ratioChanged_;
    int             ratio_;

signals:
    void sigRatioChanged(int);
};
