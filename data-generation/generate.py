# 每种类型的稀疏矩阵生成 10 个
# 5 个 2048x2048 ，5 个 1024x1024

import create_matrix
import getopt
import sys
import os

args, argl = getopt.getopt(sys.argv[1:], "f")

force_gen = False

for arg, val in args:
  match arg:
    case "-f":
      force_gen = True

if os.path.exists("_generated"):
  if force_gen:
    os.remove("_generated")
  else:
    sys.exit(0)


for t in create_matrix.SPARSE_MATRIX_TYPES:
  for i in range(5):
    mat = create_matrix.create_sparse_matrix(t, 2048, 2048)
    create_matrix.dump_to_file(mat, "2048x2048{}_{}.bin".format(t, i))
    mat = create_matrix.create_sparse_matrix(t, 1024, 1024)
    create_matrix.dump_to_file(mat, "1024x1024{}_{}.bin".format(t, i))

open("_generated", "wb").close()
