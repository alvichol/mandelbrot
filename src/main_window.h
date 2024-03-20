#pragma once

#include "worker.h"
#include <QImage>
#include <QKeyEvent>
#include <QMainWindow>
#include <QWheelEvent>
#include <memory>

class main_window : public QMainWindow {

    Q_OBJECT

public:
    explicit main_window(QWidget *parent = nullptr);
    ~main_window() override;

    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void update_output();

private:
    void work_start(job_type job);
    void shift_color(uint32_t val);
    void move_fractal(QPointF point);

private:
    std::array<worker, THREAD_COUNTER> workers;
    QImage image;
    uint64_t threads_done = 0;
    uint64_t precision = 0;
    double cur_scale = (1 / 400.);
    std::complex<double> cur_offset = 0;
    std::array<uint32_t, 3> cur_color = {255, 0, 0};
    bool mouse_pressed = false;
    QPointF press_pos;
};