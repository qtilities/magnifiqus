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
