#include "SpMM.hh"
#include "leedagee/CSR.hh"
#ifdef VTUNE_ENABLE
#include <ittnotify.h>
#endif
#include <iostream>

using namespace leedagee;
const static int BLOCK_SIZE = 32;

Matrix SpMM_opt(const Matrix &A, const SparseMatrix &B) {
  auto res = Matrix(0, 0);
#ifdef VTUNE_ENABLE
  __itt_resume();
#endif
  for (int __count = 0; __count < VTUNE_REPR; __count++) {
    // A is m*k, B is n*k, C is m*n
    auto [m, k] = A.size();
    auto [n, _k] = B.size();
    std::vector<float> _aT(m * k);
    auto *a = reinterpret_cast<const vector<float> *>(&A)->data();
    auto aT = _aT.data();
    for (int _j = 0; _j < k; _j += BLOCK_SIZE)
      for (int _i = 0; _i < m; _i += BLOCK_SIZE)
        for (int j = _j; j < _j + BLOCK_SIZE; j++) {
          for (int i = _i; i < _i + BLOCK_SIZE; i++)
            aT[j * m + i] = a[i * k + j];
          }
    std::vector<float> _cT(m * n);
    auto cT = _cT.data();
    auto bS = fromSparseMatrix(B);
    int nelems = bS.d.size();
    #ifdef STAT_FLADDOPS
    long long numaddops = 0;
    #endif
    for (int idx = 0; idx < nelems; idx++) {
      const auto &elem = bS.d[idx];
      const auto &r = elem.x, &c = elem.y;
      #pragma omp simd
      for (int i = 0; i < m; i++) {
        cT[r * m + i] += elem.v * aT[c * m + i];
        #ifdef STAT_FLADDOPS
        numaddops++;
        #endif
      }
    }
    #ifdef STAT_FLADDOPS
    std::cerr << "optimized numaddops: " << numaddops << std::endl;
    #endif
    std::vector<float> _c(m * n);
    auto c = _c.data();
    for (int _i = 0; _i < m; _i += BLOCK_SIZE)
      for (int _j = 0; _j < n; _j += BLOCK_SIZE)
        for (int i = _i; i < _i + BLOCK_SIZE; i++)
          for (int j = _j; j < _j + BLOCK_SIZE; j++) {
            c[i * n + j] = cT[j * m + i];
          }
    res = std::move(Matrix(_c, m, n));
  }
#ifdef VTUNE_ENABLE
  __itt_pause();
#endif
  return res;
}
