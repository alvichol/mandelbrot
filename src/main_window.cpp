#include "main_window.h"
#include <QPainter>

void main_window::work_start(job_type job = job_type::DRAW) {
    std::shared_ptr<QImage> new_img;
    if (job == job_type::DRAW) {
        new_img = std::make_shared<QImage>(width(), height(), QImage::Format_RGB888);
    } else {
        new_img = std::make_shared<QImage>(image.copy());
    }
    if (job == job_type::PAINT) precision = (precision + PRECISIONS.size() - 1) % PRECISIONS.size();
    for (size_t i = 0; i < THREAD_COUNTER; i++) {
        workers[i].set_input({new_img, cur_scale, cur_offset, cur_color, precision, job});
    }
}

main_window::main_window(QWidget *parent)
    : QMainWindow(parent),
      image(800, 600, QImage::Format_RGB888) {
    resize(800, 600);
    for (size_t i = 0; i < THREAD_COUNTER; i++) {
        workers[i].initialize(i);
        connect(&workers[i], &worker::output_changed, this, &main_window::update_output);
    }
    work_start();
}

void main_window::update_output() {
    threads_done++;
    if (threads_done == THREAD_COUNTER) {
        threads_done = 0;
        std::shared_ptr<QImage> res = workers[0].get_output().img;
        image = res->copy();
        precision = (precision + 1) % PRECISIONS.size();
        if (precision != 0) work_start();
        update();
    }
}

void main_window::resizeEvent(QResizeEvent *event) {
    precision = 0;
    work_start();
}

void main_window::wheelEvent(QWheelEvent *event) {
    QPoint numDegrees = event->angleDelta();
    double rot_y = numDegrees.y() / 120.;
    if (rot_y == 0)
        return;

    double delta_scale = std::abs(rot_y) * 1.2;
    if (rot_y > 0) {
        cur_scale /= delta_scale;
    } else {
        cur_scale *= delta_scale;
    }
    precision = 0;
    work_start();
}

void main_window::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Up:
            move_fractal(QPointF(0, -height() / 10.));
            break;
        case Qt::Key_Down:
            move_fractal(QPointF(0, height() / 10.));
            break;
        case Qt::Key_Left:
            move_fractal(QPointF(-width() / 10., 0));
            break;
        case Qt::Key_Right:
            move_fractal(QPointF(width() / 10., 0));
            break;
        case Qt::Key_F:
            shift_color(30);
            break;
        default:
            QMainWindow::keyPressEvent(event);
            break;
    }
}

void main_window::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        mouse_pressed = true;
        press_pos = event->position();
    }
}

void main_window::mouseMoveEvent(QMouseEvent *event) {
    if ((event->buttons() & Qt::LeftButton) && mouse_pressed) {
        QPointF offset(event->position() - press_pos);
        press_pos = event->position();
        move_fractal(-offset);
    }
}

void main_window::mouseReleaseEvent(QMouseEvent *event) {
    mouse_pressed = false;
}

void main_window::move_fractal(QPointF point) {
    std::complex<double> offset(point.x(), point.y());
    offset *= cur_scale;
    cur_offset += offset;
    precision = 0;
    work_start();
}

void main_window::shift_color(uint32_t val) {
    size_t idx = (cur_color[0] == 255 && cur_color[1] != 255 ? 0 : (cur_color[1] == 255 && cur_color[2] != 255 ? 1 : 2));
    size_t prev = (idx + 2) % 3;
    size_t next = (idx + 1) % 3;
    if (cur_color[prev] > 0) {
        uint32_t diff = std::min(val, cur_color[prev]);
        cur_color[prev] -= diff;
        cur_color[next] += val - diff;
    } else {
        uint32_t diff = std::min(val, 255 - cur_color[next]);
        cur_color[next] += diff;
        cur_color[idx] -= val - diff;
    }
    work_start(job_type::PAINT);
}

void main_window::paintEvent(QPaintEvent *) {
    QPainter p(this);
    p.drawImage(0, 0, image);
}

main_window::~main_window() {}
