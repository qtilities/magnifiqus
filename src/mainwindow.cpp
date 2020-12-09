#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "magnifiqus.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , actionGroup(new QActionGroup(this))
    , timer_(new QTimer(this))
{
    ui->setupUi(this);
    ui->centralwidget->setLayout(ui->layout);

    Qt::Key key;
    for (int i = 0; i < 4; ++i)
    {
        actZoom[i] = new QAction("x" + QString::number(i + 2) , this);
        actZoom[i]->setCheckable(true);
        if (i == 0)
        {
            actZoom[i]->setChecked(true);
            key = Qt::Key_2;
        }
        else if (i == 1)
        {
            key = Qt::Key_3;
        }
        else if (i == 2)
        {
            key = Qt::Key_4;
        }
        else
        {
            key = Qt::Key_5;
        }
        actZoom[i]->setShortcut(Qt::CTRL + key);
        actionGroup->addAction(actZoom[i]);
        ui->menu->addAction(actZoom[i]);
        connect(actZoom[i], &QAction::triggered, this, &MainWindow::onRatioSelected);
    }
    ui->menu->addSeparator();
    ui->menu->addAction(ui->actTop);
    ui->menu->addSeparator();
    ui->menu->addAction(ui->actAbout);
    ui->menu->addSeparator();
    ui->menu->addAction(ui->actQuit);

    connect(ui->magnifiqus, &Magnifiqus::sigRatioChanged, this, &MainWindow::onRatioChanged);
    connect(ui->actTop,   &QAction::triggered, this, &MainWindow::onTopChecked);
    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::onAboutClicked);
    connect(ui->actQuit,  &QAction::triggered, QCoreApplication::instance(), &QCoreApplication::quit);

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
    delete ui;
}
void MainWindow::moveEvent(QMoveEvent *)
{
    timer_->start(50);
    ui->magnifiqus->blockEvents(true);
}
void MainWindow::closeEvent(QCloseEvent *)
{
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
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

    settings.beginGroup("Main");
    move(settings.value("Position", QPoint(200, 200)).toPoint());

    Qt::WindowFlags flags = windowFlags();
    bool            onTop = settings.value("AlwaysOnTop", false).toBool();
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
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());
    int zoom;
    QAction *act = actionGroup->checkedAction();
    for (int i = 0; i < 4; ++i)
    {
        if (actZoom[i] == act)
            zoom = i + 2;
    }
    settings.beginGroup("Main");
    settings.setValue("Position", pos());
    settings.setValue("AlwaysOnTop", ui->actTop->isChecked());
    settings.setValue("Zoom", zoom);
}
void MainWindow::setEndDragging()
{
    ui->magnifiqus->blockEvents(false);
}
