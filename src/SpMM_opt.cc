#include "SpMM.hh"
#include "leedagee/SparseRepresents.hh"
#include "leedagee/Transpose.hh"
#ifdef VTUNE_ENABLE
#include <ittnotify.h>
#endif
#include <cassert>
#include <cstring>
#include <immintrin.h>
#include <iostream>
#include <iterator>
#include <omp.h>

using namespace leedagee;

Matrix SpMM_opt(const Matrix &A, const SparseMatrix &B) {
  auto res = Matrix(0, 0);
#ifdef VTUNE_ENABLE
  __itt_resume();
#endif
  for (int __count = 0; __count < REPR; __count++) {
    // A is m*k, B is n*k, C is m*n
    int m, k, n;
    std::tie(m, k) = A.size();
    std::tie(n, k) = B.size();
    auto *aT = new (std::align_val_t(64)) float[m * k];
    BCSR bS;
#pragma omp sections nowait
    {
#pragma omp section
      transpose(aT, reinterpret_cast<const vector<float> *>(&A)->data(), m, k);
#pragma omp section
      bS.fromSparseMatrix(reinterpret_cast<const vector<float> *>(&B)->data(),
                          n, k);
    }
    auto *cT = new (std::align_val_t(64)) float[m * n]();
    for (int c = 0, b = 0; b < k / COLUMN_BLOCK; b++) {
      auto &ix = bS.idx[b];
      auto &rix = bS.ridx[b];
      const int len = ix[n] - ix[0];
      // partition[] by row
      int partition[NTHREADS + 1], _;
      for (int i = 0; i < NTHREADS; i++)
        partition[i] = rix.lower_bound(ix[0] + i * len / NTHREADS)->second;
      partition[NTHREADS] = n;
#pragma omp parallel num_threads(NTHREADS)
      {
        const int threadid = omp_get_thread_num();
        const int sr = partition[threadid], er = partition[threadid + 1];
        for (int r = sr; r < er; r++) {
          const int sd = ix[r], ed = ix[r + 1];
          alignas(16) float cLine[m];
          memcpy(cLine, &cT[r * m], sizeof(cLine));
          auto nextElem = bS.d.begin();
          std::advance(nextElem, sd);
          for (int d = sd; d < ed; d++) {
            const auto &elem = *nextElem++;
            // it actually slows down
            //_mm_prefetch(&aT[nextElem->y * m], _MM_HINT_T1);
#ifdef DEBUGCSR
            assert(elem.x == r);
#endif
            const int c = elem.y;
            const float v = elem.v;
            __m128 vv = _mm_broadcast_ss(&v);
            __m512 vvv = _mm512_broadcast_f32x2(vv);
            for (int j = 0; j < m; j += 16) {
              __m512 cv = _mm512_load_ps(&cLine[j]);
              __m512 av = _mm512_load_ps(&aT[c * m + j]);
              __m512 tmpv;
              cv = _mm512_fmadd_ps(cv, av, tmpv);
              _mm512_store_ps(&cLine[j], cv);
              // cLine[j] += v * aT[c * m + j];
            }
          }
          for (int j = 0; j < m; j += 16) {
            __m512 cv = _mm512_load_ps(&cLine[j]);
            _mm512_stream_ps(&cT[r * m + j], cv);
          }
        }
      }
    }
    std::vector<float> _c(m * n);
    transpose(_c.data(), cT, m, n);
    delete[] cT;
    delete[] aT;
    res = std::move(Matrix(_c, m, n));
  }
#ifdef VTUNE_ENABLE
  __itt_pause();
#endif
  return res;
}
