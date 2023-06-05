#include "SpMM.hh"
#include "leedagee/CSR.hh"

using namespace leedagee;

Matrix SpMM_opt(const Matrix &A, const SparseMatrix &B) {
  // A is m*k, B is n*k, C is m*n
  auto [m, k] = A.size();
  auto [n, _k] = B.size();
  std::vector<float> aT(m * k);
  for (int i = 0; i < m; i++)
    for (int j = 0; j < k; j++) {
      aT[j * m + i] = A.at(i, j);
    }
  std::vector<float> cT(m * n);
  auto bS = fromSparseMatrix(B);
  for (const auto &elem : bS.d) {
    const auto &r = elem.x, &c = elem.y;
    for (int i = 0; i < m; i++)
      cT[r * m + i] += elem.v * aT[c * m + i];
  }
  std::vector<float> c;
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      c.push_back(cT[j * m + i]);
    }
  }
  return Matrix(c, m, n);
}
