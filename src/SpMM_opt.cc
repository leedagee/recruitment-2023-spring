#include "SpMM.hh"
#include "leedagee/CSR.hh"
#ifdef VTUNE_ENABLE
#include <ittnotify.h>
#endif

using namespace leedagee;

Matrix SpMM_opt(const Matrix &A, const SparseMatrix &B) {
  auto res = Matrix(0, 0);
#ifdef VTUNE_ENABLE
  __itt_resume();
  for (int __count = 0; __count < 256; __count++) {
#endif
  // A is m*k, B is n*k, C is m*n
  auto [m, k] = A.size();
  auto [n, _k] = B.size();
  std::vector<float> _aT(m * k);
  auto *a = reinterpret_cast<const vector<float>*>(&A)->data();
  auto aT = _aT.data();
  for (int i = 0; i < m; i++)
    for (int j = 0; j < k; j++) {
      aT[j * m + i] = a[i * k + j];
    }
  std::vector<float> _cT(m * n);
  auto cT = _cT.data();
  auto bS = fromSparseMatrix(B);
  for (const auto &elem : bS.d) {
    const auto &r = elem.x, &c = elem.y;
    for (int i = 0; i < m; i++)
      cT[r * m + i] += elem.v * aT[c * m + i];
  }
  std::vector<float> _c(m * n);
  auto c = _c.data();
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      c[i * n + j] = cT[j * m + i];
    }
  }
  res = std::move(Matrix(_c, m, n));
#ifdef VTUNE_ENABLE
  }
  __itt_pause();
#endif
  return res;
}
