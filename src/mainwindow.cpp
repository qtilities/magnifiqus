/*
    Copyright (C) 2020-2021 Andrea Zanellato <redtid3@gmail.com>

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
#include "dialogprefs.hpp"

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
#include <QX11Info>

#include <X11/Xlib-xcb.h>
#include <X11/Xatom.h>

constexpr int MainWindow::ratio_min;
constexpr int MainWindow::ratio_max;
constexpr int MainWindow::size_min;

static QScreen* findScreenAt(const QPoint& pos)
{
    for (QScreen* screen : QGuiApplication::screens()) {
        if (screen->geometry().contains(pos))
            return screen;
    }
    return nullptr;
}
MainWindow::MainWindow(QSystemTrayIcon* icon, QWidget* parent)
    : QMainWindow(parent)
    , actAbout(new QAction(QIcon::fromTheme("help-about"), tr("&About"), this))
    , actAutoStart(new QAction(tr("Auto&start"), this))
    , actCursor(new QAction(tr("Show &cursor"), this))
    , actPrefs(new QAction(QIcon::fromTheme("preferences-system"), tr("&Preferences"), this))
    , actQuit(new QAction(QIcon::fromTheme("application-exit"), tr("&Quit"), this))
    , actTop(new QAction(tr("Always on &Top"), this))
    , actionGroup(new QActionGroup(this))
    , tmrShowRatio_(new QTimer(this))
    , tmrUpdatePos_(new QTimer(this))
    , trayMenu(new QMenu(this))
    , trayIcon(icon)
    , dlgPrefs(new DialogPrefs(this))
    , dragType_(DragNone)
    , ratioChanged_(false)
    , cursorFilled_(false)
    , ratio_(ratio_min)
{
    actAutoStart->setCheckable(true);
    actCursor->setCheckable(true);
    actTop->setCheckable(true);

    for (int i = 0; i < 4; ++i) {
        actZoom[i] = new QAction("x&" + QString::number(i + 2), this);
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
    trayMenu->addAction(actCursor);
    trayMenu->addSeparator();
    trayMenu->addAction(actAbout);
    trayMenu->addSeparator();
    trayMenu->addAction(actPrefs);
    trayMenu->addSeparator();
    trayMenu->addAction(actQuit);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    connect(qApp, &QCoreApplication::aboutToQuit, this, &MainWindow::onAboutToQuit);
    connect(qApp, &QCoreApplication::aboutToQuit, dlgPrefs, &QObject::deleteLater);
    connect(this, &MainWindow::sigRatioChanged, this, &MainWindow::onRatioChanged);
    connect(actAbout, &QAction::triggered, this, &MainWindow::onAboutClicked);
    connect(actPrefs, &QAction::triggered, this, &MainWindow::onPrefsClicked);
    connect(actTop, &QAction::triggered, this, &MainWindow::onTopChecked);
    connect(actQuit, &QAction::triggered, qApp, &QCoreApplication::quit);
    connect(dlgPrefs, &DialogPrefs::sigPrefsChanged, this, &MainWindow::onPreferencesChanged);

    connect(trayIcon, &QSystemTrayIcon::activated,
            [=](QSystemTrayIcon::ActivationReason reason) {
                if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick)
                    setVisible(!isVisible());
            });
    tmrUpdatePos_->setInterval(10);
    connect(tmrUpdatePos_, &QTimer::timeout, this, &MainWindow::updateGrabFollowMouse);

    tmrShowRatio_->setInterval(1000);
    connect(tmrShowRatio_, &QTimer::timeout, this, &MainWindow::notifyRatioComplete);

    setMouseTracking(true);
    setWindowFlag(Qt::Tool);
    setWindowFlag(Qt::FramelessWindowHint);
    setWindowIcon(QIcon(":/appicon"));
    loadSettings();
}
MainWindow::~MainWindow()
{
}
void MainWindow::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint pos = event->pos();
        lastPoint_ = pos;
        dragType_ = isOverResizeCorner(pos) ? DragResize : DragMove;
    }
    QMainWindow::mousePressEvent(event);
}
void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if ((event->buttons() & Qt::LeftButton) && dragType_ != DragNone)
        dragType_ = DragNone;
}
void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();

    if (dragType_ == DragResize || isOverResizeCorner(pos))
        setCursor(Qt::SizeFDiagCursor);
    else
        setCursor(Qt::ArrowCursor);

    if ((event->buttons() & Qt::LeftButton) && dragType_ != DragNone) {
        if (dragType_ == DragResize) {
            int w = qMax(size_min, pos.x());
            int h = qMax(size_min, pos.y());
            resize(w, h);
        } else {
            move(event->globalX() - lastPoint_.x(), event->globalY() - lastPoint_.y());
        }
        update();
    }
    QMainWindow::mouseMoveEvent(event);
}
void MainWindow::paintEvent(QPaintEvent*)
{
    int w = width();
    int h = height();
    int hBound = w / ratio_ / 2;
    int vBound = h / ratio_ / 2;
    QPoint topL = mapToGlobal(rect().topLeft());
    QRect boundRect = QRect(topL.x() - hBound,
                            topL.y() - vBound,
                            w + hBound * 2,
                            h + vBound * 2);
    QColor borderColor("#181818");
    QPen pen(borderColor);
    QPainter painter(this);

    if (boundRect.contains(QCursor::pos())) {
        // Avoid to get a "mirror on mirror" effect
        const QPixmap& p = getWindowOverlayPixmap();
        painter.drawPixmap(0, 0, p);
    } else {
        // Draw the cached scaled pixmap
        painter.scale(ratio_, ratio_);
        painter.drawPixmap(0, 0, pixmap_);
        painter.resetTransform();

        if (actCursor->isChecked()) {
            // Draw pointer coords rect
            float cursorSize = ratio_ * cursorSize_;
            QRectF cursorRect(rect().x() + (w / 2) - cursorSize / 2,
                              rect().y() + (h / 2) - cursorSize / 2,
                              ratio_ * cursorSize_, ratio_ * cursorSize_);
            pen.setWidth(1);
            painter.setPen(Qt::white);
            painter.setCompositionMode(QPainter::RasterOp_SourceXorDestination);
            painter.drawRect(cursorRect);

            if (cursorFilled_)
                painter.fillRect(cursorRect, Qt::white);

            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        }
    }
    // Draw the frame border
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawRect(QRect(rect().x() + 1,
                           rect().y() + 1,
                           rect().width() - 2,
                           rect().height() - 2));
    if (ratioChanged_) {
        // Notify the changed ratio on the top right corner using the mouse wheel
        painter.setPen(Qt::white);
        QFont font(this->font());
        font.setPointSize(24);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(rect().topRight().x() - 42,
                         rect().topRight().y() + 28,
                         "x" + QString::number(ratio_));
    }
}
void MainWindow::showEvent(QShowEvent*)
{
    setX11NetWmState();

    updateGrabFollowMouse();
    tmrUpdatePos_->start();
}
void MainWindow::hideEvent(QHideEvent*)
{
    tmrUpdatePos_->stop();
}
void MainWindow::wheelEvent(QWheelEvent* event)
{
    QPoint degrees = event->angleDelta() / 8;
    if (!degrees.isNull()) {
        int newRatio = degrees.y() / 15 + ratio_;
        setRatio(newRatio);
    }
    event->accept();
}
void MainWindow::onAboutToQuit()
{
    actAutoStart->isChecked() ? createAutostartFile() : deleteAutostartFile();
    saveSettings();
}
void MainWindow::onPreferencesChanged()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationDisplayName());

    settings.beginGroup("Main");
    cursorFilled_ = settings.value("CursorFilled").toBool();
    cursorSize_ = settings.value("CursorSize").toUInt();
    settings.endGroup();
}

void MainWindow::onPrefsClicked()
{
    if (dlgPrefs->isHidden())
        dlgPrefs->show();
}
void MainWindow::onAboutClicked()
{
    QFile f(":/about.html");
    if (!f.open(QFile::ReadOnly | QFile::Text)) {
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
    ratioChanged_ = true;
    tmrShowRatio_->start();
    blockSignals(false);
}
void MainWindow::onRatioSelected()
{
    int newValue;
    QObject* sender = this->sender();
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
    bool onTop = settings.value("AlwaysOnTop", true).toBool();
    if (onTop) {
        flags |= Qt::WindowStaysOnTopHint;
        actTop->setChecked(true);
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);

    actAutoStart->setChecked(settings.value("AutoStart", false).toBool());
    actCursor->setChecked(settings.value("ShowCursor", false).toBool());

    int ratio = settings.value("Zoom", 2).toUInt();
    if (ratio < ratio_min)
        ratio = ratio_min;
    else if (ratio > ratio_max)
        ratio = ratio_max;
    setRatio(ratio);

    cursorSize_ = settings.value("CursorSize", 1).toUInt();
    if (cursorSize_ < 1 || cursorSize_ > 100)
        cursorSize_ = 1;

    cursorFilled_ = settings.value("CursorFilled", false).toBool();
    settings.endGroup();
}
void MainWindow::saveSettings()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope,
                       QApplication::organizationName(),
                       QApplication::applicationDisplayName());
    int zoom;
    QAction* act = actionGroup->checkedAction();
    for (int i = 0; i < 4; ++i) {
        if (actZoom[i] == act)
            zoom = i + 2;
    }
    settings.beginGroup("Main");
    settings.setValue("AlwaysOnTop", actTop->isChecked());
    settings.setValue("AutoStart", actAutoStart->isChecked());
    settings.setValue("Position", pos());
    settings.setValue("ShowCursor", actCursor->isChecked());
    settings.setValue("Size", size());
    settings.setValue("Zoom", zoom);
}
void MainWindow::createAutostartFile()
{
    QDir configDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    QString appName = QApplication::applicationName();
    QString filePath = configDir.filePath("autostart/" + appName + ".desktop");
    QFile file(filePath);

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
    QString filePath(
        configDir.filePath(
            "autostart/" + QApplication::applicationName() + ".desktop"));
    QFile file(filePath);

    if (!file.exists())
        return;

    file.remove();
}
void MainWindow::setRatio(int value)
{
    if (value < ratio_min || value == ratio_ || value > ratio_max)
        return;

    ratio_ = value;
    emit sigRatioChanged(value);
}
void MainWindow::notifyRatioComplete()
{
    ratioChanged_ = false;
}
void MainWindow::updateGrabFollowMouse()
{
    QPoint pos = QCursor::pos();
    QScreen* screen = findScreenAt(pos);
    if (!screen)
        return;

    int w = width();
    int h = height();
    const int ratio = ratio_;
    WId wid = QApplication::desktop()->winId();
    QPoint grabCenter = pos;
    QRect grabLimits
        = screen->geometry().adjusted(w / (2 * ratio), h / (2 * ratio),
                                      -w / (2 * ratio), -h / (2 * ratio));

    grabCenter.setX(qMax(grabLimits.left(), qMin(grabLimits.right(), grabCenter.x())));
    grabCenter.setY(qMax(grabLimits.top(), qMin(grabLimits.bottom(), grabCenter.y())));

    QRect grabArea {
        grabCenter.x() - w / (2 * ratio),
        grabCenter.y() - h / (2 * ratio), w, h
    };
    pixmap_ = screen->grabWindow(wid,
                                 grabArea.x(), grabArea.y(),
                                 grabArea.width(), grabArea.height());
    update();
}
void MainWindow::setX11NetWmState()
{
    // Needed for some WMs to avoid to display the window in the taskbar
    // and task switcher and display it in all desktop (FIXME)
    Display* display = QX11Info::display();
    WId window = this->winId();
    Atom state = XInternAtom(display, "_NET_WM_STATE", false);
    Atom sticky = XInternAtom(display, "_NET_WM_STATE_STICKY", false);
    Atom skipPgr = XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", false);
    Atom skipTbr = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", false);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&sticky, 1L);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&skipPgr, 1L);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeAppend, (unsigned char*)&skipTbr, 1L);
}
const QPixmap& MainWindow::getWindowOverlayPixmap()
{
    int w = width();
    int h = height();
    QPixmap& pixmap = windowOverlayPixmap_;

    if (pixmap.width() != w || pixmap.height() != h) {
        pixmap = QIcon(":/appicon").pixmap(w, h);
    }
    return pixmap;
}
bool MainWindow::isOverResizeCorner(QPoint pos) const
{
    QPoint bottomRight = rect().bottomRight();
    return pos.x() > bottomRight.x() - 20 && pos.y() > bottomRight.y() - 20;
}
