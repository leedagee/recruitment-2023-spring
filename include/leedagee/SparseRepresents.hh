#include "Matrices.hh"
#include "leedagee/Transpose.hh"
#include <vector>
#include <array>

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
        int y;
        float v;
    } CSREntry;

    struct BCSR {
        std::array<std::vector<int>, MAX_SIZE / COLUMN_BLOCK> idx;
        std::vector<CSREntry> d;
        int m, n;
        void fromSparseMatrix(const float *data, int m, int n);
    };
}
