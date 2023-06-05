#include "Matrices.hh"
#include <vector>

namespace leedagee {
    typedef struct {
        int x, y;
        float v;
    } CSREntry;

    struct CSR {
        std::vector<CSREntry> d;
        int m, n;
    };

    struct CSR fromSparseMatrix(const SparseMatrix &orig);
}
