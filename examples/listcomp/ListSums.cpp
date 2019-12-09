// listcomp example main driver
#include "util.hpp"

long total = 0;

#define INTERVAL 200


void cb(ListSums::_Type46& x) {
    total += x._0 + x._1;
}

int main(int argc, char const *argv[])
{
    int size = read<int>(argv[1]);
    long ns;
    ListSums l;
    for (int i = 0; i < size; i++) {
        l.insert_r(ListSums::R(1, ""));
        l.insert_s(ListSums::S("a", 2));
    }
    
    while (true) {
        if (size % INTERVAL == 0) {
            ns = now_ns();
        }
        l.insert_r(ListSums::R(3, "a"));
        l.insert_s(ListSums::S("a", 2));
        l.q(cb);
        if (size % INTERVAL == 0) {
            long duration = now_ns() - ns;
            std::cout << size << " " << duration << "\n";
            if (duration > 2000000) {
                break;
            }
        }
        size += 1;
    }
    return 0;
}
