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

#include <QWidget>

class QTimer;
class Magnifiqus : public QWidget
{
    Q_OBJECT

public:
    explicit Magnifiqus(QWidget *parent = nullptr);

    void     setRatio(int value);
    int      ratio() const;

    void     blockEvents(bool);

    static constexpr int ratio_min = 2;
    static constexpr int ratio_max = 5;

signals:
    void sigRatioChanged(int);

private:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void updatePosition();

    QTimer *timer_;
    QPixmap pixmap_;
    int     ratio_;
    bool    blockEvents_;
};
