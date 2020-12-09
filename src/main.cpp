#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("AZDrums");
    app.setOrganizationDomain("azdrums.org");
    app.setApplicationName("magnifiqus");
    app.setApplicationDisplayName("Magnifiqus");

    MainWindow w;
    w.show();

    return app.exec();
}
