#include "timer.hh"
#include "Matrices.hh"
#include "SpMM.hh"

#include <cassert>

int main(int argc, char *argv[]) {
  assert(argc > 1);
// read from file
  int rows, cols;
  string filename(argv[1]);
  SparseMatrix B(filename);
  std::tie(rows, cols) = B.size();
  Matrix A(rows, cols);
  std::shared_ptr<std::chrono::microseconds> baseline(
      new std::chrono::microseconds),
      optimized(new std::chrono::microseconds);
  Matrix C, C_opt;
  {
    ScopeTimer timer("baseline", baseline);
    C = SpMM_base(A, B);
  }
  {
    ScopeTimer timer("optimized", optimized);
    C_opt = SpMM_opt(A, B);
  }
  if (C != C_opt) {
    std::cout << endl << "result not match" << endl;
    return -1;
  }
  std::cout << ">>> average optimized: " << *optimized / 1us / REPR << "us" << endl;
  std::cout << "Speedup: "
            << (*baseline / 1ms) / (float)(*optimized / 1ms) * REPR << 'x'
            << std::endl;
  return 0;
}
