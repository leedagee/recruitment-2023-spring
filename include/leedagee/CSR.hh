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
        void fromSparseMatrix(const float *data, int m, int n);
    };

}
