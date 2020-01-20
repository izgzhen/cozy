#include <iostream>
#include <chrono>
#include <time.h>
#include "remove.h"

using namespace std::chrono;

float getRand() {
    return ((float) rand() / (float) RAND_MAX) * 10;
}

std::vector<float> randomList(int N) {
    std::vector<float> list;
    for (int i = 0; i < N; ++i) {
        list.push_back(getRand());
    }
    return list;
}

class Timer {
public:
    Timer() : t_(high_resolution_clock::now()) {}
    double GetDurationMicroseconds() {
        high_resolution_clock::time_point t2 = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(t2 - t_).count();
        return float(duration) / 1000.0;
    }
private:
    high_resolution_clock::time_point t_;
};

void cb(float x) {
    std::cout << x << ", ";
}

int main() {
    srand(time(NULL));
    for (int N = 1000; N < 10000; N += 1000) {
        std::vector<float> l = randomList(N);
        auto cl = std::make_shared<VecStream<float>>(l);
        Remove r(cl, false, 0);
        Timer t;
        r.remove(1);
        r.restore();
        std::cout << N << " " << t.GetDurationMicroseconds() << "\n";
    }
}
