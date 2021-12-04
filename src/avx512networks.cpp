#include <immintrin.h>
#include <cstdint>
#include "vectorized_sort.hpp"
#include "defines.hpp"
#include "merge_networks.hpp"
#include "column_networks.hpp"

/* optimized sorting network for two vectors, that is 16 ints */
inline void sort_16(__m256i &v1, __m256i &v2)
{
    COEX(v1, v2); /* step 1 */

    v2 = _mm256_shuffle_epi32(v2, _MM_SHUFFLE(2, 3, 0, 1)); /* step 2 */
    COEX(v1, v2);

    auto tmp = v1; /* step  3 */
    v1 = SHUFFLE_2_VECS(v1, v2, 0b10001000);
    v2 = SHUFFLE_2_VECS(tmp, v2, 0b11011101);
    COEX(v1, v2);

    v2 = _mm256_shuffle_epi32(v2, _MM_SHUFFLE(0, 1, 2, 3)); /* step  4 */
    COEX(v1, v2);

    tmp = v1; /* step  5 */
    v1 = SHUFFLE_2_VECS(v1, v2, 0b01000100);
    v2 = SHUFFLE_2_VECS(tmp, v2, 0b11101110);
    COEX(v1, v2);

    tmp = v1; /* step  6 */
    v1 = SHUFFLE_2_VECS(v1, v2, 0b11011000);
    v2 = SHUFFLE_2_VECS(tmp, v2, 0b10001101);
    COEX(v1, v2);

    v2 = _mm256_permutevar8x32_epi32(v2, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0));
    COEX(v1, v2); /* step  7 */

    tmp = v1; /* step  8 */
    v1 = SHUFFLE_2_VECS(v1, v2, 0b11011000);
    v2 = SHUFFLE_2_VECS(tmp, v2, 0b10001101);
    COEX(v1, v2);

    tmp = v1; /* step  9 */
    v1 = SHUFFLE_2_VECS(v1, v2, 0b11011000);
    v2 = SHUFFLE_2_VECS(tmp, v2, 0b10001101);
    COEX(v1, v2);

    /* permute to make it easier to restore order */
    v1 = _mm256_permutevar8x32_epi32(v1, _mm256_setr_epi32(0, 4, 1, 5, 6, 2, 7, 3));
    v2 = _mm256_permutevar8x32_epi32(v2, _mm256_setr_epi32(0, 4, 1, 5, 6, 2, 7, 3));

    tmp = v1; /* step  10 */
    v1 = SHUFFLE_2_VECS(v1, v2, 0b10001000);
    v2 = SHUFFLE_2_VECS(tmp, v2, 0b11011101);
    COEX(v1, v2);

    /* restore order */
    auto b2 = _mm256_shuffle_epi32(v2, 0b10110001);
    auto b1 = _mm256_shuffle_epi32(v1, 0b10110001);
    v1 = _mm256_blend_epi32(v1, b2, 0b10101010);
    v2 = _mm256_blend_epi32(b1, v2, 0b10101010);
}

void sorting_network512(int *arr, int n)
{
    if (n <= SCALAR_NETWORK_THRESHOLD) // very small array, use brute force or AVX2
    {
        switch (n)
        {
        case 0:
            return;
        case 1:
            return;
        case 2:
            COEX_INT(0, 1);
            return;
        case 3:
            COEX_INT(0, 1);
            COEX_INT(1, 2);
            COEX_INT(0, 1);
            return;
        }
        if (n <= 8)
        {
            __m256i v1 = _mm256_mask_loadu_epi32(_mm256_set1_epi32(INT32_MAX), 0xFF >> (8 - n), arr);
            SORT_8(v1);
            _mm256_mask_storeu_epi32(arr, 0xFF >> (8 - n), v1);
            return;
        }
        __m256i v1 = _mm256_loadu_si256((__m256i *)arr);
        __m256i v2 = _mm256_mask_loadu_epi32(_mm256_set1_epi32(INT32_MAX), 0xFF >> (16 - n), arr + 8);
        sort_16(v1, v2);
        _mm256_storeu_si256((__m256i *)arr, v1);
        _mm256_mask_storeu_epi32(arr + 8, 0xFF >> (16 - n), v2);
        return;
    }

    static __m512i buffer[MAX_VECTORS_IN_NETWORK];
    const int remainder = n % INTS_IN_VECTOR ? n % INTS_IN_VECTOR : INTS_IN_VECTOR;
    const int nbVectors = (n + INTS_IN_VECTOR - 1) / INTS_IN_VECTOR; //nbVectors > 1
    for (int i = 0; i < nbVectors - 1; ++i)
    {
        buffer[i] = LOAD_VECTOR(arr + i * INTS_IN_VECTOR);
    }
    buffer[nbVectors - 1] = _mm512_mask_loadu_epi32(
        _mm512_set1_epi32(INT32_MAX), 0xFFFF >> (INTS_IN_VECTOR - remainder), arr + n - remainder);
    buffer[nbVectors] = _mm512_set1_epi32(INT32_MAX); // sometimes we need one more vector

    columnNetwork(buffer, nbVectors);
    mergeNetwork(buffer, nbVectors);

    for (int i = 0; i < nbVectors - 1; i++)
    {
        STORE_VECTOR(arr + i * INTS_IN_VECTOR, buffer[i]);
    }
    _mm512_mask_storeu_epi32(arr + n - remainder,
                             0xFFFF >> (INTS_IN_VECTOR - remainder), buffer[nbVectors - 1]);
}