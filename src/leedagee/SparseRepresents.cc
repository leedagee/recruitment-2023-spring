#include "leedagee/SparseRepresents.hh"
#include "leedagee/Transpose.hh"

namespace leedagee {

void COO::fromSparseMatrix(const float *data, int m, int n) {
  this->m = m;
  this->n = n;
  this->d.reserve(this->m * this->n / 8);
  for (int _i = 0; _i < n; _i += BLOCK_SIZE)
    for (int _j = 0; _j < m; _j += BLOCK_SIZE)
      for (int i = _i; i < _i + BLOCK_SIZE; i++)
        for (int j = _j; j < _j + BLOCK_SIZE; j++) {
          const float &v = data[i * n + j];
          if (v != 0.0f)
            this->d.push_back(COOEntry{i, j, v});
        }
}

void BCSR::fromSparseMatrix(const float *data, int m, int n) {
  this->m = m;
  this->n = n;
  this->d.reserve(this->m * this->n / 8);
  for (int _j = 0, c = 0, b = 0; _j < n; _j += COLUMN_BLOCK, b++) {
    auto &ix = this->idx[b];
    auto &rix = this->ridx[b];
    ix.resize(this->n);
    for (int i = 0; i < n; i++) {
      ix[i] = c;
      rix[c] = i;
      for (int j = _j; j < _j + COLUMN_BLOCK; j++) {
        const float &v = data[i * n + j];
        if (v != 0.0f)
#ifdef DEBUGCSR
          d.emplace_back(CSREntry{i, j, v}), c++;
#else
          d.emplace_back(CSREntry{j, v}), c++;
#endif
      }
    }
    ix[n] = c;
    rix[c] = n;
  }
}

} // namespace leedagee
