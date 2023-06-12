#include "leedagee/Transpose.hh"

namespace leedagee {

void transpose(float *aT, const float *a, int n, int m) {
  for (int _j = 0; _j < m; _j += BLOCK_SIZE)
    for (int _i = 0; _i < n; _i += BLOCK_SIZE)
      for (int j = _j; j < _j + BLOCK_SIZE; j++) {
        for (int i = _i; i < _i + BLOCK_SIZE; i++)
          aT[j * n + i] = a[i * m + j];
      }
}

} // namespace leedagee
