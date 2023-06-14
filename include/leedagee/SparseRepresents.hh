#include "Matrices.hh"
#include "leedagee/Transpose.hh"
#include <array>
#include <map>
#include <vector>

namespace leedagee {

typedef struct {
  int x, y;
  float v;
} COOEntry;

struct COO {
  std::vector<COOEntry> d;
  int m, n;
  void fromSparseMatrix(const float *data, int m, int n);
};

typedef struct {
#ifdef DEBUGCSR
  int x;
#endif
  int y;
  float v;
} CSREntry;

class BCSR {
public:
  BCSR() = default;

  std::array<std::vector<int>, MAX_SIZE / COLUMN_BLOCK> idx;
  std::array<std::map<int, int>, MAX_SIZE / COLUMN_BLOCK> ridx;
  std::vector<CSREntry> d;
  int m, n;
  void fromSparseMatrix(const float *data, int m, int n);
};

} // namespace leedagee
