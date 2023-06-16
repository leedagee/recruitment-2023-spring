. /opt/intel/oneapi/setvars.sh

export PATH="${CMPLR_ROOT}/linux/bin-llvm:${PATH}"
export PATH="/nix/store/fqfi0m3fw3szj3n99r5n359579808bh6-cmake-3.25.3/bin:${PATH}"

rm -rf build

cmake \
        -B build \
        -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo \
        -DCMAKE_C_COMPILER=icx \
        -DCMAKE_CXX_COMPILER=icpx \
        -DCMAKE_LINKER=icpx \
        -DCMAKE_CXX_LINK_FLAGS="-ipo" \
        -DCMAKE_CXX_LINK_EXECUTABLE="<CMAKE_LINKER> <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>" \
        -DCMAKE_AR=llvm-ar \
        -DCMAKE_RANLIB=llvm-ranlib \
        -DOpenMP_C_FLAGS="-fiopenmp" \
        -DOpenMP_CXX_FLAGS="-fiopenmp" \
        -DOpenMP_C_LIB_NAMES="" \
        -DOpenMP_CXX_LIB_NAMES="" \
        "-DOpenMP_libiomp5_LIBRARY=${CMPLR_ROOT}/linux/compiler/lib/intel64_lin/libiomp5.a" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
        -DVTUNE=ON \
        -DNTHREADS=16 \
        -DREPR=1024

make -C build

