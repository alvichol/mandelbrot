#include "worker.h"

worker::worker() = default;

void worker::initialize(uint64_t idx) {
    input_version = INPUT_VERSION_QUIT + 1;
    thread_idx = idx;
    worker_thread = std::thread([&]() { thread_proc(); });
}

worker::~worker() {
    input_version = INPUT_VERSION_QUIT;
    input_changed.notify_all();
    worker_thread.join();
}

void worker::set_input(coordinates coords) {
    {
        std::lock_guard lg(m);
        input = std::move(coords);
        ++input_version;
    }
    input_changed.notify_all();
}

void worker::thread_proc() {
    uint64_t last_version = input_version;
    for (;;) {
        coordinates input_copy;
        {
            std::unique_lock lg(m);
            input_changed.wait(lg, [&]() {
                return input_version != last_version;
            });
            last_version = input_version;
            if (last_version == INPUT_VERSION_QUIT) {
                break;
            }
            input_copy = input;
        }
        fill_image(last_version, input_copy);
    }
}

void worker::fill_image(uint64_t last_version, const coordinates &coords) {
    uchar *img_start = coords.img->bits();
    auto height = coords.img->height(), width = coords.img->width();
    auto line_size = coords.img->bytesPerLine();
    uint64_t cnt = 0;
    for (int j = 0; j != height; ++j) {
        uchar *p = img_start + j * line_size;
        for (int i = 0; i != width; i++) {
            if (last_version != input_version)
                return;
            if (cnt % THREAD_COUNTER != thread_idx) {
                p++;
                p++;
                p++;
            } else {
                double color = (coords.job == job_type::PAINT ? recolor(p) : pixel_color(last_version, coords, i, j));
                if (color == -1)
                    return;
                *p++ = static_cast<uchar>(color * (coords.color[0] / 255.) * 0xff);
                *p++ = static_cast<uchar>(color * (coords.color[1] / 255.) * 0xff);
                *p++ = static_cast<uchar>(color * (coords.color[2] / 255.) * 0xff);
            }
            cnt++;
        }
    }
    store_output({coords.img});
}

double worker::recolor(uchar *pix) {
    uchar max_col = std::max(*pix, std::max(*(pix + 1), *(pix + 2)));
    double orig = max_col / 255.;
    return orig;
}

double worker::pixel_color(uint64_t last_version, const coordinates &coords, int x, int y) {
    std::complex<double> c(x - coords.img->width() / 2., y - coords.img->height() / 2.);
    c *= coords.scale;
    c += coords.offset;
    std::complex<double> z = 0;
    size_t const MAX_STEPS = PRECISIONS[coords.precision];
    size_t step = 0;
    for (;;) {
        if (last_version != input_version)
            return -1;
        if (z.real() * z.real() + z.imag() * z.imag() >= 4) {
            return static_cast<double>(step % 51) / 50.;
        }
        if (step == MAX_STEPS) {
            return 0;
        }
        z = z * z + c;
        step++;
    }
}

painting_result worker::get_output() const {
    std::lock_guard lg(m);
    return output;
}

void worker::store_output(const painting_result &result) {
    std::lock_guard lg(m);
    output = result;
    if (!notify_output_queued) {
        QMetaObject::invokeMethod(this, "notify_output");
        notify_output_queued = true;
    }
}

void worker::notify_output() {
    {
        std::lock_guard lg(m);
        notify_output_queued = false;
    }
    emit output_changed();
}
