#import "typst-slides/slides.typ": *

#show: slides.with(
    authors: "Leedagee",
    title: "Regression, chaos and misfortune",
    subtitle: "A wrong way towards dense-sparse matrix multiplication optimization",
    date: "June 2023",
)

#set text(font: "Inria Sans", size: 25pt)

#slide(theme-variant: "title slide")

#new-section("Overview")

#slide(title: "Environment")[
    #box(height: 65%, columns(2, [
        === Hardware: `hustcpu01` 
        + 2x Xeon Gold 6338 \@ 2.0GHz 
        + 256G mem \@ unknown speed

        === Software: 
        + Ubuntu Focal 20.04 LTS
        + nixpkgs-23.05 (cmake & numactl)
        + IntelLLVM 2022.1.0
        + vtune 2023.1.0

        === Configuration:
        + 16 threads
        + `-O3 -ffast-math`
        + Inter-procedure Optimization
        + IntelLLVM icpx
        + Intel OpenMP `-fiopenmp`
        + `numactl --membind=0 --cpubind=0`
        + `/tmp/dataset` matrices
    ]))
]

#slide[
    #set align(center)
    #table(
        columns: (auto, auto, auto, auto),
        inset: 10pt,
            [Pattern & Size], [Baseline], [Optimized], [Opt Avg of 1024-runs],
            [banded-1024], [11.777s], [18.291ms@643.9x], [2.038ms@5779x],
            [banded-2048], [94.319s], [37.324ms@2527x], [9.344ms@10094x],
            [diagonal-1024], [11.893s], [17.009ms@699.2x], [2.027ms@5867x],
            [diagonal-2048], [94.609s], [37.552ms@2520x], [9.327ms@10144x],
            [general-1024], [11.869s], [21.257ms@558.4x], [3.718ms@3192x],
            [general-2048], [94.636s], [45.591ms@2076x], [16.658ms@5681x],
            [symmetric-1024], [11.717s], [19.800ms@591.8x], [2.913ms@4022x],
            [symmetric-2048], [94.188s], [52.412ms@1797x], [13.180ms@7146x],
            [triangular-1024], [11.861s], [14.692ms@807.3x], [3.777ms@3140x],
            [triangular-2048], [94.364s], [44.413ms@2125x], [16.850ms@5600x],
    )
]

#new-section("Optimization")

#slide(title: "The problem and thoughts")[
    + Matrix B is sparse
    + For each non-zero element in $B _(r,c)$, the computation involves
        reading $A_(*, c)$ and writing $C_(*, r)$
    + For memory contiguous, instead of computing $A B^T$, compute $(B A^T)^T$
    + To avoid cache miss:
        - Each row in $C^T$ will be written by exactly one thread in the parallel region. 
        - Memory Blocking techniques should be used on $A^T$ (but proved to be wrong >\_<)
]

#slide(title: "The initial implementation")[
    commit `9d5381`
    
    - Two transpose operation.
    - Naive iterating through B.
    - SIMD by compiler (it seems)

    Results are premising actually:
    + 39.7ms / 287ms on banded
    + 381ms / 2942ms on general

    #block[
        #set text(size: 15pt)
        Note: 
        + I misnamed COO as CSR, until BCSR was implemented, the naming are wrong all the code.
        + The performance test here are done on my i5-1240P laptop, performance may be inconsistent.
    ]
]

#slide[
    Profiling on the naive impl shows that the `Matrix#at` function and `[]` operator are not optimized out. Given how `Matrix` / `SparseMatrix` are implemented, a mind-explosive `reinterpret_cast` is used.

    Profiling after that:

    #image("naive-profile.png", height: 50%)
]

#slide[
    It is highly memory bound
        - Transpose \~80%, CPI goes 13
        - Calculation \~50%, mainly random write

    Next steps:

    + Memory blocking applied on transposing
    + Scan B in different order
        - Transposed, significant improvement (about 1.5x), calculation is now random reading 
        - Looping as if doing memory blocking, actually not very useful

    As algo changes later, more details on this implementation are omitted.
]

#slide(title: "ALBUS-like partition")[
    - Implement CSR, in this case, BCSR.
    - Column blocking is enabled at this time (128 columns as a block)
    - Switched to icpx & `-fiopenmp` for vtune analyze
    - Paralleled transposing of A and scanning of B
    - Still memory bound (of course for sparse ones)
]

#slide(title: "AVX512 intrinsics")[
    Just write intrinsics (quite exciting, isn't it?), and get optimized out by compiler (confused and disappointed this time)

    #set text(size: 12pt)
    ```cpp
    // calculation
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

    // non-temporal memory store
    for (int j = 0; j < m; j += 16) {
    __m512 cv = _mm512_load_ps(&cLine[j]);
    _mm512_stream_ps(&cT[r * m + j], cv);
    }
    ```
]

#slide(title: "Finding correct blocking size")[
    Just write script to enumerate through possibly sizes.

    #table(columns: (auto, auto, auto, auto, auto), align: center, inset: 8pt,
        [TB / CB], [128], [256], [512], [1024],
        [16], [38.661ms], [30.793ms], [28.092ms], [26.331ms],
        [32], [37.508ms], [34.084ms], [28.355ms], [25.406ms],
        [64], [37.422ms], [30.036ms], [29.278ms], [25.146ms],
        [128], [42.755ms], [30.115ms], [27.183ms], [25.194ms],
        [256], [40.039ms], [31.425ms], [27.270ms], [26.480ms],
    )

    Disabled column blocking, use 64 as transpose blocking size.
]

#slide(title: "NUMA and number of threads")[
    Due to small matrices, the performance stopped increasing after 16 cores.
    Profiling later suggests maybe it's NUMA issue or just thread creation overhead.

    Profiler suggests there's inter-NUMA (and socket) communication. 
    Since 16 cores are enough, with `numactl`, the performance gets almost 2x speedup.
]

#slide(title: "Toolchain")[
    Tons of time spent on CMake, IntelLLVM and OpenMP

    CMake version is critical.
    
    The nix environment pollutes the building, 
        causing glibc version conflict with Intel OpenMP.

    `ld`, `ranlib`, `ar` need to be set to IntelLLVM's to correctly handle the IPO/LTO
        objects (which are actually LLVM IR Bitcode)

    Strange `-lrt`, `-ldl`, `-pthread` missing.
]

#slide(title: "The Final Profiling")[
    + 93.5% Effective time on start_thread / clone\@gilbc (?), filtered out in discussion later
    + 34.5% Memory bound on computation
    + Multi-threading is a joke after filtering out the start_thread calls.

    #image("multi-threading-is-joke.png")
]

#slide(title: "Pushing it further")[
    - Larger matrices
    - Can the transpose be optimized out in actual application?
    - How icpx optimize my code and gives no vfmadd instructions?
]

#slide(title: "创")[
    + Slow cmake generation because of the matrices generation
    + Old cmake without icpx IPO support
    + Old nix / nixpkgs
    + clangd crashes on omp pragmas
    + icpx/clang crashes
    + No sepdk kernel modules on `hustcpu01`, unable to do memory bandwidth analyze
    + VTune cannot give any result under some circumstance
    + Broken typst bibliography support
]

#slide(title: "Chaos")[
    + Local machine performance inconsistency (A 12th-Gen Intel Laptop)
    + Git, but just as snapshots of code, build script is not well managed
    + The baseline gets great speed up after IPO, comparing with baseline is meaningless somehow.
    + Local coding & debug, `hustcpu01` only for testing, 
        not sure about the actual performance speedup when doing optimization
    + Because of all of these, I cannot form a report / presentation quickly (I have to go through all the work I have done to recall and gather all the details and performance metrics)
    + Different vtune profiler report on static / dynamic linked libiomp (same performance) 
]

#slide[
    Conclusion: my workflow is just bullshit

    Question:
    + The right way to characterize / summarize a single optimization?
    + Did I do too much environment tuning (or DevOps, if it is), and too few code optimization?
    + With collaboration on a much more complex project? I cannot image how chaotic it will be.
    + How to form the report / presentation efficiently? What should I collect in development? 
]

#new-section("Reference")

#slide[
    #set text(size: 20pt)
    == References
    + heptagonhust, #link("https://mp.weixin.qq.com/s/AF3uJ7sutM6W1b4OoNRMwA")[spmv的优化], 2022
    + H. Bian, J. Huang et al, #link("https://www.sciencedirect.com/science/article/pii/S0167739X2033020X")[ALBUS: A method for efficiently processing SpMV using SIMD and Load balancing], FGCS 2021 Volume 116, 2020
    + P. Koanantakool et al., #link("https://ieeexplore.ieee.org/abstract/document/7516081")[Communication-Avoiding Parallel Sparse-Dense Matrix-Matrix Multiplication], 2016 IEEE IPDPS
    + G. Ruetsch, P. Micikevicius, #link("https://www.cs.colostate.edu/~cs675/MatrixTranspose.pdf")[Optimizing Matrix Transpose in CUDA], 2009
    + D. Yan, T. Wu, Y. Liu and Y. Gao, #link("https://ieeexplore.ieee.org/abstract/document/8359956")[An efficient sparse-dense matrix multiplication on a multicore system], 2017 IEEE ICCT
    + Intel Crop., #link("https://www.intel.com/content/dam/doc/manual/64-ia-32-architectures-optimization-manual.pdf")[Intel 64 and IA-32 Architectures Optimization Reference Manual], 2023
    + OpenMP Org., #link("https://www.openmp.org/wp-content/uploads/OpenMPRefCard-5-2-web.pdf")[OpenMP API Reference Guide], 2021
]
