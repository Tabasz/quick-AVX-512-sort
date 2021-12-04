#include <immintrin.h>
#include <cstdint>
#include "vectorized_sort.hpp"
#include "defines.hpp"

inline int calc_min(__m512i vec2)
{
    auto perm_mask2 = _mm512_setr_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    vec2 = _mm512_min_epi32(vec2, _mm512_permutexvar_epi32(perm_mask2, vec2));
    __m256i vec = _mm512_extracti64x4_epi64(vec2, 0);

    auto perm_mask = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
    vec = _mm256_min_epi32(vec, _mm256_permutevar8x32_epi32(vec, perm_mask));
    vec = _mm256_min_epi32(vec, _mm256_shuffle_epi32(vec, 0b10110001));
    vec = _mm256_min_epi32(vec, _mm256_shuffle_epi32(vec, 0b01001110));
    return _mm256_extract_epi32(vec, 0);
}

inline int calc_max(__m512i vec2)
{
    auto perm_mask2 = _mm512_setr_epi32(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    vec2 = _mm512_max_epi32(vec2, _mm512_permutexvar_epi32(perm_mask2, vec2));
    __m256i vec = _mm512_extracti64x4_epi64(vec2, 0);

    auto perm_mask = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
    vec = _mm256_max_epi32(vec, _mm256_permutevar8x32_epi32(vec, perm_mask));
    vec = _mm256_max_epi32(vec, _mm256_shuffle_epi32(vec, 0b10110001));
    vec = _mm256_max_epi32(vec, _mm256_shuffle_epi32(vec, 0b01001110));
    return _mm256_extract_epi32(vec, 0);
}

void partition_vec(__m512i &curr_vec, const __m512i &pivot_vec,
                   __m512i &smallest_vec, __m512i &biggest_vec, int *arr, int &l_store, int &r_store)
{
    /* which elements are smaller or equal than the pivot */
    int less_equal_mask = _mm512_cmple_epi32_mask(curr_vec, pivot_vec);
    /* update the smallest and largest values of the array */
    smallest_vec = _mm512_min_epi32(curr_vec, smallest_vec);
    biggest_vec = _mm512_max_epi32(curr_vec, biggest_vec);
    int amount_le_pivot = _mm_popcnt_u32(less_equal_mask);
    /* store smaller/equal elements at the l_store and bigger at the r_store */
    _mm512_mask_compressstoreu_epi32(arr + l_store, less_equal_mask, curr_vec);
    r_store -= 16 - amount_le_pivot;
    l_store += (amount_le_pivot);
    _mm512_mask_compressstoreu_epi32(arr + r_store, ~less_equal_mask, curr_vec);
}

void partition_vec_mask(__m512i &curr_vec, int cnt, const __m512i &pivot_vec,
                        __m512i &smallest_vec, __m512i &biggest_vec, int *arr, int &l_store, int &r_store)
{
    int mask = (1 << cnt) - 1;
    /* which elements are smaller or equal than the pivot */
    int less_equal_mask = _mm512_cmple_epi32_mask(curr_vec, pivot_vec);
    /* update the smallest and largest values of the array */
    smallest_vec = _mm512_min_epi32(curr_vec, smallest_vec);
    biggest_vec = _mm512_max_epi32(curr_vec, biggest_vec);
    int amount_le_pivot = _mm_popcnt_u32(less_equal_mask & mask);
    r_store -= cnt - amount_le_pivot;
    /* store smaller/equal elements at the l_store and bigger at the r_store */
    _mm512_mask_compressstoreu_epi32(arr + l_store, less_equal_mask & mask, curr_vec);
    l_store += (amount_le_pivot);
    _mm512_mask_compressstoreu_epi32(arr + r_store, (~less_equal_mask) & mask, curr_vec);
}

inline int partition(int *arr, int left, int right, int pivot, int &smallest, int &biggest)
{
    auto pivot_vec = _mm512_set1_epi32(pivot); /* fill vector with pivot */
    auto sv = _mm512_set1_epi32(smallest);     /* vector for smallest elements */
    auto bv = _mm512_set1_epi32(biggest);      /* vector for biggest elements */

    int r_store = right; /* right store point */
    int l_store = left;  /* left store point */
    /* buffer 8 vectors from both sides of the array */
    auto vec_left = LOAD_VECTOR(arr + left), vec_left2 = LOAD_VECTOR(arr + left + 16);
    auto vec_left3 = LOAD_VECTOR(arr + left + 32), vec_left4 = LOAD_VECTOR(arr + left + 48);
    auto vec_left5 = LOAD_VECTOR(arr + left + 64), vec_left6 = LOAD_VECTOR(arr + left + 80);
    auto vec_left7 = LOAD_VECTOR(arr + left + 96), vec_left8 = LOAD_VECTOR(arr + left + 112);

    right -= 128;
    auto vec_right = LOAD_VECTOR(arr + right), vec_right2 = LOAD_VECTOR(arr + right + 16);
    auto vec_right3 = LOAD_VECTOR(arr + right + 32), vec_right4 = LOAD_VECTOR(arr + right + 48);
    auto vec_right5 = LOAD_VECTOR(arr + right + 64), vec_right6 = LOAD_VECTOR(arr + right + 80);
    auto vec_right7 = LOAD_VECTOR(arr + right + 96), vec_right8 = LOAD_VECTOR(arr + right + 112);
    left += 128;

    while (right - left >= 128)
    { /* partition 128 elements per iteration */
        __m512i curr_vec, curr_vec2, curr_vec3, curr_vec4, curr_vec5, curr_vec6, curr_vec7, curr_vec8;

        /* load 8 vectors from the side where is less space for storing */
        if (r_store - right < left - l_store)
        {
            right -= 128;
            curr_vec = LOAD_VECTOR(arr + right);
            curr_vec2 = LOAD_VECTOR(arr + right + 16);
            curr_vec3 = LOAD_VECTOR(arr + right + 32);
            curr_vec4 = LOAD_VECTOR(arr + right + 48);
            curr_vec5 = LOAD_VECTOR(arr + right + 64);
            curr_vec6 = LOAD_VECTOR(arr + right + 80);
            curr_vec7 = LOAD_VECTOR(arr + right + 96);
            curr_vec8 = LOAD_VECTOR(arr + right + 112);
        }
        else
        {
            curr_vec = LOAD_VECTOR(arr + left);
            curr_vec2 = LOAD_VECTOR(arr + left + 16);
            curr_vec3 = LOAD_VECTOR(arr + left + 32);
            curr_vec4 = LOAD_VECTOR(arr + left + 48);
            curr_vec5 = LOAD_VECTOR(arr + left + 64);
            curr_vec6 = LOAD_VECTOR(arr + left + 80);
            curr_vec7 = LOAD_VECTOR(arr + left + 96);
            curr_vec8 = LOAD_VECTOR(arr + left + 112);
            left += 128;
        }

        /* partition 8 vectors and store them on both sides of the array */
        partition_vec(curr_vec, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec3, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec4, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec5, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec6, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec7, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec8, pivot_vec, sv, bv, arr, l_store, r_store);
    }

    int extra = (right - left);
    __m512i curr_vec1, curr_vec2, curr_vec3, curr_vec4, curr_vec5, curr_vec6, curr_vec7, curr_vec8;
    switch ((extra + 15) >> 4) // process rest of n % 128 elements
    {
    case 0:
        break;
    case 1:
        curr_vec1 = LOAD_VECTOR(arr + left);
        partition_vec_mask(curr_vec1, extra, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    case 2:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec2, extra - 16, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    case 3:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        curr_vec3 = LOAD_VECTOR(arr + left + 32);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec3, extra - 32, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    case 4:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        curr_vec3 = LOAD_VECTOR(arr + left + 32);
        curr_vec4 = LOAD_VECTOR(arr + left + 48);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec3, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec4, extra - 48, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    case 5:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        curr_vec3 = LOAD_VECTOR(arr + left + 32);
        curr_vec4 = LOAD_VECTOR(arr + left + 48);
        curr_vec5 = LOAD_VECTOR(arr + left + 64);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec3, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec4, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec5, extra - 64, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    case 6:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        curr_vec3 = LOAD_VECTOR(arr + left + 32);
        curr_vec4 = LOAD_VECTOR(arr + left + 48);
        curr_vec5 = LOAD_VECTOR(arr + left + 64);
        curr_vec6 = LOAD_VECTOR(arr + left + 80);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec3, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec4, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec5, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec6, extra - 80, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    case 7:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        curr_vec3 = LOAD_VECTOR(arr + left + 32);
        curr_vec4 = LOAD_VECTOR(arr + left + 48);
        curr_vec5 = LOAD_VECTOR(arr + left + 64);
        curr_vec6 = LOAD_VECTOR(arr + left + 80);
        curr_vec7 = LOAD_VECTOR(arr + left + 96);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec3, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec4, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec5, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec6, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec7, extra - 96, pivot_vec, sv, bv, arr, l_store, r_store);
        break;

    case 8:
        curr_vec1 = LOAD_VECTOR(arr + left);
        curr_vec2 = LOAD_VECTOR(arr + left + 16);
        curr_vec3 = LOAD_VECTOR(arr + left + 32);
        curr_vec4 = LOAD_VECTOR(arr + left + 48);
        curr_vec5 = LOAD_VECTOR(arr + left + 64);
        curr_vec6 = LOAD_VECTOR(arr + left + 80);
        curr_vec7 = LOAD_VECTOR(arr + left + 96);
        curr_vec8 = LOAD_VECTOR(arr + left + 112);
        partition_vec(curr_vec1, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec2, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec3, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec4, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec5, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec6, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec(curr_vec7, pivot_vec, sv, bv, arr, l_store, r_store);
        partition_vec_mask(curr_vec8, extra - 112, pivot_vec, sv, bv, arr, l_store, r_store);
        break;
    }
    /* partition and store 8 vectors coming from the left side of the array */
    partition_vec(vec_left, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left2, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left3, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left4, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left5, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left6, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left7, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_left8, pivot_vec, sv, bv, arr, l_store, r_store);
    /* partition and store 8 vectors coming from the right side of the array */
    partition_vec(vec_right, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right2, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right3, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right4, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right5, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right6, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right7, pivot_vec, sv, bv, arr, l_store, r_store);
    partition_vec(vec_right8, pivot_vec, sv, bv, arr, l_store, r_store);
    smallest = calc_min(sv);
    biggest = calc_max(bv);
    return l_store;
}

/***
 * vectorized pivot selection */

/* vectorized random number generator xoroshiro128+ */
inline __m256i vnext(__m256i &s0, __m256i &s1)
{
    s1 = _mm256_xor_si256(s0, s1); /* modify vectors s1 and s0 */
    s0 = _mm256_xor_si256(_mm256_xor_si256(VROTL(s0, 24), s1),
                          _mm256_slli_epi64(s1, 16));
    s1 = VROTL(s1, 37);
    return _mm256_add_epi64(s0, s1);
} /* return random vector */

/* transform random numbers to the range between 0 and bound - 1 */
inline __m256i rnd_epu32(__m256i rnd_vec, __m256i bound)
{
    __m256i even = _mm256_srli_epi64(_mm256_mul_epu32(rnd_vec, bound), 32);
    __m256i odd = _mm256_mul_epu32(_mm256_srli_epi64(rnd_vec, 32), bound);
    return _mm256_blend_epi32(odd, even, 0b01010101);
}

/* average of two integers without overflow
* http://aggregate.org/MAGIC/#Average%20of%20Integers */
inline int average(int a, int b) { return (a & b) + ((a ^ b) >> 1); }

inline int get_pivot(int *arr, const int left, const int right)
{
    auto bound = _mm256_set1_epi32(right - left + 1);
    auto left_vec = _mm256_set1_epi32(left);

    /* seeds for vectorized random number generator */
    auto s0 = _mm256_setr_epi64x(8265987198341093849, 3762817312854612374,
                                 1324281658759788278, 6214952190349879213);
    auto s1 = _mm256_setr_epi64x(2874178529384792648, 1257248936691237653,
                                 7874578921548791257, 1998265912745817298);
    s0 = _mm256_add_epi64(s0, _mm256_set1_epi64x(left));
    s1 = _mm256_sub_epi64(s1, _mm256_set1_epi64x(right));

    __m256i v[9];
    for (int i = 0; i < 9; ++i)
    {                                                /* fill 9 vectors with random numbers */
        auto result = vnext(s0, s1);                 /* vector with 4 random uint64_t */
        result = rnd_epu32(result, bound);           /* random numbers between 0 and bound - 1 */
        result = _mm256_add_epi32(result, left_vec); /* indices for arr */
        v[i] = _mm256_i32gather_epi32(arr, result, sizeof(uint32_t));
    }
    /* median network for 9 elements */
    COEX(v[0], v[1]);
    COEX(v[2], v[3]); /* step 1 */
    COEX(v[4], v[5]);
    COEX(v[6], v[7]);
    COEX(v[0], v[2]);
    COEX(v[1], v[3]); /* step 2 */
    COEX(v[4], v[6]);
    COEX(v[5], v[7]);
    COEX(v[0], v[4]);
    COEX(v[1], v[2]); /* step 3 */
    COEX(v[5], v[6]);
    COEX(v[3], v[7]);
    COEX(v[1], v[5]);
    COEX(v[2], v[6]); /* step 4 */
    COEX(v[3], v[5]);
    COEX(v[2], v[4]); /* step 5 */
    COEX(v[3], v[4]); /* step 6 */
    COEX(v[3], v[8]); /* step 7 */
    COEX(v[4], v[8]); /* step 8 */

    SORT_8(v[4]);                                 /* sort the eight medians in v[4] */
    return average(_mm256_extract_epi32(v[4], 3), /* compute next pivot */
                   _mm256_extract_epi32(v[4], 4));
}

/* recursion for quicksort */
inline void qs_core512(int *arr, int left, int right, bool choose_avg = false,
                       const int avg = 0, const int upper_bound = INT32_MAX)
{
    if (right - left < SORTING_NETWORK_THRESHOLD)
    {
        /* use sorting networks for small arrays up to SORTING_NETWORK_THRESHOLD */
        sorting_network512(arr + left, right - left + 1);
        return;
    }
    /* avg is average of largest and smallest values in array */
    int pivot = choose_avg ? avg : get_pivot(arr, left, right); // assumes that get pivot is in [smallest, biggest]
    if (pivot == upper_bound)
        pivot--;
    int smallest = INT32_MAX; /* smallest value after partitioning */
    int biggest = INT32_MIN;  /* largest value after partitioning */
    int bound = partition(arr, left, right + 1, pivot, smallest, biggest);

    if (!choose_avg)
    {
        /* the ratio of the length of the smaller partition to the array length */
        double ratio = (std::min(right - (bound - 1), bound - left) / double(right - left + 1));
        /* if unbalanced sub-arrays, change pivot selection strategy */
        if (ratio < 0.2)
            choose_avg = true;
    }

    if (smallest < pivot) /* if values in the left sub-array are not identical */
        qs_core512(arr, left, bound - 1, choose_avg, average(smallest, pivot), pivot);
    if (pivot + 1 < biggest) /* if values in the right sub-array are not identical */
        qs_core512(arr, bound, right, choose_avg, average(biggest, pivot + 1), biggest);
}

/* call this function for sorting */
void quicksort512(int *arr, int n) { qs_core512(arr, 0, n - 1); }