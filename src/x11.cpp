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
#include <QtGlobal>
#include <QWidget>
#include <QApplication>
#if QT_VERSION < 0x060000
#include <QX11Info>
#include <QDesktopWidget>
#else
#include <QMainWindow>
#include <QDialog>
#include <QWidget>
#endif
#include "x11.hpp"
#if 0
#include <X11/Xlib-xcb.h>
#endif
#include <X11/Xatom.h>

Window x11::desktop()
{
#if QT_VERSION < 0x060000
    return QApplication::desktop()->winId();
#else
    return 0;
#endif
}

Display *x11::display()
{
#if QT_VERSION < 0x060000
    return QX11Info::display();
#else
    QNativeInterface::QX11Application *x11App
        = qApp->nativeInterface<QNativeInterface::QX11Application>();
    return x11App->display();
#endif
}

Window x11::rootWindow()
{
#if QT_VERSION < 0x060000
    return QX11Info::appRootWindow();
#else
    for (QWidget *widget : qApp->topLevelWidgets()) {
        if (QMainWindow *mainWindow = qobject_cast<QMainWindow *>(widget))
            return mainWindow->effectiveWinId();
        if (QDialog *dialog = qobject_cast<QDialog *>(widget))
            return dialog->effectiveWinId();
    }
    return 0;
#endif
}

WId x11::activeWindowId()
{
    WId root = WId(x11::rootWindow());
    Atom atom = XInternAtom(x11::display(), "_NET_ACTIVE_WINDOW", false);
    unsigned long type, resultLen, rest;
    int format;
    WId result = 0;
    unsigned char *data = nullptr;
    if (XGetWindowProperty(x11::display(), root, atom, 0, 1, false, XA_WINDOW, &type, &format,
                           &resultLen, &rest, &data)
        == Success) {
        result = *reinterpret_cast<long *>(data);
        XFree(data);
    }
    return result;
}

QRect x11::windowFrame(WId wid)
{
    QRect result;
    XWindowAttributes wa;
    if (XGetWindowAttributes(x11::display(), wid, &wa)) {
        Window child;
        int x, y;
        // translate to root coordinate
        XTranslateCoordinates(x11::display(), wid, wa.root, 0, 0, &x, &y, &child);
#if 0
        qDebug("%d, %d, %d, %d", x, y, wa.width, wa.height);
#endif
        result.setRect(x, y, wa.width, wa.height);

        // get the frame widths added by the window manager
        Atom atom = XInternAtom(x11::display(), "_NET_FRAME_EXTENTS", false);
        unsigned long type, resultLen, rest;
        int format;
        unsigned char *data = nullptr;
        if (XGetWindowProperty(x11::display(), wid, atom, 0, LONG_MAX, false, XA_CARDINAL, &type,
                               &format, &resultLen, &rest, &data)
            == Success) {
        }
        if (data) { // left, right, top, bottom
            long *offsets = reinterpret_cast<long *>(data);
            result.setLeft(result.left() - offsets[0]);
            result.setRight(result.right() + offsets[1]);
            result.setTop(result.top() - offsets[2]);
            result.setBottom(result.bottom() + offsets[3]);
            XFree(data);
        }
    }
    return result;
}

void x11::dontShowInTaskbar(QWidget *widget)
{
    Display *display = x11::display();
    Window window = widget->effectiveWinId();
    Atom state = XInternAtom(display, "_NET_WM_STATE", false);
    Atom sticky = XInternAtom(display, "_NET_WM_STATE_STICKY", false);
    Atom skipPgr = XInternAtom(display, "_NET_WM_STATE_SKIP_PAGER", false);
    Atom skipTbr = XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", false);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeAppend, (unsigned char *)&sticky, 1L);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeAppend, (unsigned char *)&skipPgr, 1L);
    XChangeProperty(display, window, state, XA_ATOM, 32, PropModeAppend, (unsigned char *)&skipTbr, 1L);
}
