#ifndef __DEFINES_HPP__
#define __DEFINES_HPP__

#include <immintrin.h>
#include <algorithm>

#define FORCE_INLINE __attribute__((always_inline))

#define LOAD_VECTOR(arr) _mm512_loadu_si512(arr)
#define STORE_VECTOR(arr, vec) _mm512_storeu_si512(arr, vec)

#define INTS_IN_VECTOR 16
#define MAX_VECTORS_IN_NETWORK 64
#define SORTING_NETWORK_THRESHOLD (16 * 40)
#define SCALAR_NETWORK_THRESHOLD 16
#define _MM_SHUFFLEMY(z,y,x,w) (_MM_PERM_ENUM)(((z) << 6) | ((y) << 4) | ((x) << 2) | (w))

#define COEX_INT(a, b) if (arr[a] > arr[b]) std::swap(arr[a], arr[b]);

#define COEX(a, b)           \
{                             \
auto vec_tmp = a;              \
a = _mm256_min_epi32(a, b);     \
b = _mm256_max_epi32(vec_tmp, b);\
}

#define COEX16(a, b)                                  \
{                                                      \
auto vec_tmp = _mm512_min_epi32(a, b);                  \
b = _mm512_ternarylogic_epi32(a, b, vec_tmp, 0b10010110);\
a = vec_tmp;                                             \
}

/* shuffle 2 vectors, instruction for int is missing,
* therefore shuffle with float */
#define SHUFFLE_2_VECS(a, b, mask)         \
reinterpret_cast<__m256i>(_mm256_shuffle_ps(\
reinterpret_cast<__m256>(a), reinterpret_cast<__m256>(b), mask));

#define SHUFFLE_2_VECS2(a, b, mask)        \
reinterpret_cast<__m512i>(_mm512_shuffle_ps(\
reinterpret_cast<__m512>(a), reinterpret_cast<__m512>(b), mask));

#define REVERSE_HALF16(vec)                             \
{                                                        \
vec = _mm512_permutexvar_epi32(                           \
      _mm512_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0,            \
                        15, 14, 13, 12, 11, 10, 9, 8), vec);\
}

#define ASC(a, b, c, d, e, f, g, h)                                    \
    (((h < 7) << 7) | ((g < 6) << 6) | ((f < 5) << 5) | ((e < 4) << 4) |\
     ((d < 3) << 3) | ((c < 2) << 2) | ((b < 1) << 1) | (a < 0))

#define COEX_PERMUTE(vec, a, b, c, d, e, f, g, h, MASK)            \
{                                                                   \
    __m256i permute_mask = _mm256_setr_epi32(a, b, c, d, e, f, g, h);\
    __m256i permuted = _mm256_permutevar8x32_epi32(vec, permute_mask);\
    __m256i min = _mm256_min_epi32(permuted, vec);                    \
    __m256i max = _mm256_max_epi32(permuted, vec);                    \
    constexpr int blend_mask = MASK(a, b, c, d, e, f, g, h);          \
    vec = _mm256_blend_epi32(min, max, blend_mask);                   \
}

#define COEX_SHUFFLE(vec, a, b, c, d, e, f, g, h, MASK)     \
{                                                            \
    constexpr int shuffle_mask = _MM_SHUFFLEMY(d, c, b, a);   \
    __m256i shuffled = _mm256_shuffle_epi32(vec, shuffle_mask);\
    __m256i min = _mm256_min_epi32(shuffled, vec);             \
    __m256i max = _mm256_max_epi32(shuffled, vec);             \
    constexpr int blend_mask = MASK(a, b, c, d, e, f, g, h);   \
    vec = _mm256_blend_epi32(min, max, blend_mask);            \
}

/* sorting network for 8 int with compare-exchange macros
* (used for pivot selection in median of the medians) */
#define SORT_8(vec)                              \
{                                                 \
    COEX_SHUFFLE(vec, 1, 0, 3, 2, 5, 4, 7, 6, ASC);\
    COEX_SHUFFLE(vec, 2, 3, 0, 1, 6, 7, 4, 5, ASC);\
    COEX_SHUFFLE(vec, 0, 2, 1, 3, 4, 6, 5, 7, ASC);\
    COEX_PERMUTE(vec, 7, 6, 5, 4, 3, 2, 1, 0, ASC);\
    COEX_SHUFFLE(vec, 2, 3, 0, 1, 6, 7, 4, 5, ASC);\
    COEX_SHUFFLE(vec, 1, 0, 3, 2, 5, 4, 7, 6, ASC);\
}

/* vectorized random number generator xoroshiro128+ */
#define VROTL(x, k) /* rotate each uint64_t value in vector */ \
    _mm256_or_si256(_mm256_slli_epi64((x), (k)), _mm256_srli_epi64((x), 64 - (k)))

#endif /* __DEFINES_HPP__ */