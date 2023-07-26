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

void x11::dontShowInTaskbar(QWidget *widget)
{
    // Needed for some WMs to avoid to display the window in the taskbar
    // and task switcher and display it in all desktop (FIXME)
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
