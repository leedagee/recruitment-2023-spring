#pragma once

#include <variant>
#include "Matrices.hh"

namespace leedagee {
    constexpr int BLOCK_SIZE = 32;
    void transpose(float *aT, const float *a, int n, int m);
}
