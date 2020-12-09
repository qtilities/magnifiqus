#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QActionGroup;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void moveEvent(QMoveEvent *);
    void closeEvent(QCloseEvent *);

    void onAboutClicked();
    void onRatioChanged(int);
    void onRatioSelected();
    void onTopChecked(bool);

    void loadSettings();
    void saveSettings();

    void setEndDragging();

    Ui::MainWindow *ui;
    QAction        *actZoom[4];
    QActionGroup   *actionGroup;
    QTimer         *timer_;
};
#endif // MAINWINDOW_H
