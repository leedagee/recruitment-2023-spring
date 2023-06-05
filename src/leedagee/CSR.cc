#include "leedagee/CSR.hh"

namespace leedagee {

struct CSR fromSparseMatrix(const SparseMatrix &orig) {
  struct CSR r;
  std::tie(r.m, r.n) = orig.size();
  for (int i = 0, c = 0; i < r.m; i++) {
    for (int j = 0; j < r.n; j++, c++) {
      const float v = orig.at(i, j);
      if (v != 0.0f)
        r.d.push_back(CSREntry{i, j, v});
    }
  }
  return r;
}

} // namespace leedagee
