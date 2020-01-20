// Test C++ prelude
#include <iostream>
#include "prelude.hpp"

#define Vec std::make_shared<VecStream<float>>
#define Concat std::make_shared<ConcatStream<float>>
#define Slice std::make_shared<SliceStream<float>>
#define Filter std::make_shared<FilterStream<float>>

bool cb(float t) {
    std::cout << t << ", ";
    return false;
}

int main() {
    std::vector<float> v1{1, 2, 3};
    std::vector<float> v2{4, 5, 6};
    auto l1 = Vec(v1);
    assert(l1->collect() == std::vector<float>({1, 2, 3}));
    auto l2 = Vec(v2);
    assert(l2->collect() == std::vector<float>({4, 5, 6}));
    auto l3 = Concat(l1, l2);
    assert(l3->collect() == std::vector<float>({1, 2, 3, 4, 5, 6}));
    auto l4 = Slice(l3, 1, 4);
    assert(l4->collect() == std::vector<float>({2, 3, 4}));
    assert(l4->get(2) == 4);

    auto pred = [](float x) { return x > 3; };
    auto l5 = Filter(l4, pred);
    assert(l5->collect() == std::vector<float>({ 4 }));
    auto l6 = Concat(Filter(l1, pred), Filter(l2, pred));
    assert(l6->collect() == std::vector<float>({ 4, 5, 6 }));
}