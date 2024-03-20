#pragma once
#include <QImage>
#include <QObject>
#include <atomic>
#include <complex>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

inline constexpr uint64_t THREAD_COUNTER = 8;
inline constexpr std::array<uint64_t, 4> PRECISIONS = {50, 500, 2000, 5000};

enum class job_type {
    DRAW,
    PAINT
};

struct coordinates {
    std::shared_ptr<QImage> img;
    double scale;
    std::complex<double> offset;
    std::array<uint32_t, 3> color = {255, 0, 0};
    uint64_t precision;
    job_type job;
};

struct painting_result {
    std::shared_ptr<QImage> img;
};

class worker : public QObject {

    Q_OBJECT

public:
    worker();
    ~worker() override;

    void initialize(uint64_t idx);
    void set_input(coordinates coords);
    painting_result get_output() const;

signals:
    void output_changed();

private:
    void thread_proc();
    void fill_image(uint64_t last_version, const coordinates &coords);
    double pixel_color(uint64_t last_version, const coordinates &coords, int x, int y);
    double recolor(uchar *pix);
    void store_output(const painting_result &result);

private slots:
    void notify_output();

private:
    mutable std::mutex m;
    coordinates input;
    painting_result output;
    std::atomic<uint64_t> input_version;
    std::condition_variable input_changed;
    std::thread worker_thread;
    bool notify_output_queued = false;
    uint64_t thread_idx = 0;

    static uint64_t const INPUT_VERSION_QUIT = 0;
};
