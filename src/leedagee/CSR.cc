#include "leedagee/CSR.hh"
#include "leedagee/Transpose.hh"

namespace leedagee {

void CSR::fromSparseMatrix(const float *data, int m, int n) {
  this->m = m;
  this->n = n;
  this->d.clear();
  this->d.reserve(this->m * this->n / 8);
  for (int i = 0; i < m; i++)
    for (int j = 0; j < n; j++) {
      const float v = data[i * n + j];
      if (v != 0.0f)
        this->d.push_back(CSREntry{i, j, v});
    }
}

} // namespace leedagee
