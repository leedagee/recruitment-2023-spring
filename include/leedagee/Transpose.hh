#pragma once

#include <variant>
#include "Matrices.hh"
#define COLUMN_BLOCK (n)

namespace leedagee {
    constexpr int BLOCK_SIZE = 64;
    constexpr int MAX_SIZE = 2048;
    void transpose(float *aT, const float *a, int n, int m);
}
