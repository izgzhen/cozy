// listcomp example main driver
#include "util.hpp"

#define INTERVAL 2000

int main(int argc, char const *argv[])
{
    int size = read<int>(argv[1]);
    long total = 0;
    long ns;
    std::vector<SumMul::R> rs;
    std::vector<SumMul::S> ss;
    for (int i = 0; i < size; i++) {
        rs.push_back(SumMul::R(1, ""));
        ss.push_back(SumMul::S("a", 2));
    }
    SumMul l(rs, ss);

    while (true) {
        if (size % INTERVAL == 0) {
            ns = now_ns();
        }
        l.insert_r(SumMul::R(1, ""));
        l.insert_s(SumMul::S("a", 2));
        total += l.q();
        if (size % INTERVAL == 0) {
            long duration = now_ns() - ns;
            std::cout << size << " " << duration << "\n";
            if (duration > 1400000) {
                break;
            }
        }
        size += 1;
    }
    return 0;
}
