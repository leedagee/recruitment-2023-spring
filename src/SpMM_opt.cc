#include "SpMM.hh"
#include "leedagee/CSR.hh"
#include "leedagee/Transpose.hh"
#ifdef VTUNE_ENABLE
#include <ittnotify.h>
#endif
#include <iostream>

using namespace leedagee;

Matrix SpMM_opt(const Matrix &A, const SparseMatrix &B) {
  auto res = Matrix(0, 0);
#ifdef VTUNE_ENABLE
  __itt_resume();
#endif
  for (int __count = 0; __count < VTUNE_REPR; __count++) {
    // A is m*k, B is n*k, C is m*n
    auto [m, k] = A.size();
    auto [n, _k] = B.size();
    auto *aT = new (std::align_val_t(64)) float[m * k];
    CSR bS;
    // #pragma omp parallel num_threads(2)
    {
      // #pragma omp single nowait
      // #pragma omp task
      transpose(aT, reinterpret_cast<const vector<float> *>(&A)->data(), m, k);
      // #pragma omp task
      bS.fromSparseMatrix(reinterpret_cast<const vector<float> *>(&B)->data(),
                          n, k);
    }
    auto *cT = new (std::align_val_t(64)) float[m * n];
    int nelems = bS.d.size();
    for (int idx = 0; idx < nelems; idx++) {
      const auto &elem = bS.d[idx];
      const auto &r = elem.x, &c = elem.y;
#pragma omp simd
      for (int i = 0; i < m; i++) {
        cT[r * m + i] += elem.v * aT[c * m + i];
      }
    }
    std::vector<float> _c(m * n);
    auto c = _c.data();
    transpose(_c.data(), cT, m, n);
    delete cT;
    delete aT;
    res = std::move(Matrix(_c, m, n));
  }
#ifdef VTUNE_ENABLE
  __itt_pause();
#endif
  return res;
}
