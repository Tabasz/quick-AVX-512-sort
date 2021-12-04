#include <random>
#include <climits>
#include <functional>
#include <sys/time.h>
#include <unistd.h>
#include <cassert>
#include <memory.h>
#include <algorithm>

#include "sorts/sort.hpp"
using namespace std;

#define NO_OF_INTEGERS 1000 * 1000 * 1000 * 10LL
#define CACHE_SIZE 100 * 1000
#define PASSES 1

mt19937 gen;
void initRNG() { gen = mt19937(123); }
uniform_int_distribution<int> uni_dist(INT_MIN, INT_MAX);
normal_distribution<float> normal_dist(0, 100);

void test1(int *T, int n) // random ints
{
    for (int i = 0; i < n; i++)
        T[i] = uni_dist(gen);
}

void test2(int *T, int n) // gaussian
{
    for (int i = 0; i < n; ++i)
    {
        T[i] = round(normal_dist(gen));
    }
}

void test3(int *T, int n) // constant
{
    T[0] = uni_dist(gen);
    for (int i = 0; i < n; i++)
        T[i] = T[0];
}

void test4(int *T, int n) // almost sorted
{
    test1(T, n);
    sort(T, n);
    int misplaced = pow(2, log10(n)) / 2;
    uniform_int_distribution<int> uni_dist2(0, n - 1);

    for (int i = 0; i < misplaced; i++)
    {
        int idx = uni_dist2(gen);
        T[idx] = uni_dist(gen);
    }
}
#define NUM_TESTS 4
function<void(int *, int)> test[NUM_TESTS] = {
    test1,
    test2,
    test3,
    test4
};

int64_t get_time()
{
    static struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec * 1e6 + t.tv_usec;
}

inline void fill_array(int test_id, int size, int *T)
{
    int64_t iters = NO_OF_INTEGERS / size;
    for (int64_t iter = 0; iter < iters; iter++)
    {
        test[test_id](T + iter * size, size);
    }
}

inline void run_test_cache(int test_id, int size, int *T)
{
    int64_t iters = NO_OF_INTEGERS / size;
    fill_array(test_id, size, T);

    if (size < CACHE_SIZE)
    { // as resolution of gettimeofday is small and latency is significant we reduce number of calls to this method for small inputs
        int64_t iters_per_one_fill = CACHE_SIZE / size;
        int64_t passes = iters / iters_per_one_fill;

        int64_t time = 0;
        for (int64_t pass = 0; pass < passes; pass++)
        {
            memcpy(T, T + iters_per_one_fill * size * pass, iters_per_one_fill * size * 4LL);

            time -= get_time();
            for (int64_t iter = 0; iter < iters_per_one_fill; iter++)
                sort(T + iter * size, size);
            time += get_time();
        }
        printf("size %d test %d iters %ld\ttime: %.5lfs\n", size, test_id, passes * iters_per_one_fill, (double)time / 1e6);
    }
    else
    {
        int64_t time = 0;
        for (int64_t iter = 0; iter < iters; iter++)
        {
            memcpy(T, T + iter * size, size * 4LL);

            time -= get_time();
            sort(T, size);
            time += get_time();
        }
        printf("size %d test %d iters %ld\ttime: %.5lfs\n", size, test_id, iters, (double)time / 1e6);
    }
}

void run_test_not_cache(int test_id, int size, int *T, int64_t *random_order)
{
    int64_t iters = NO_OF_INTEGERS / size;
    fill_array(test_id, size, T);

    for (int64_t i = 0; i < iters; i++)
        random_order[i] = i;
    random_shuffle(random_order, random_order + iters);

    int64_t time = 0;
    time -= get_time();
    for (int64_t iter = 0; iter < iters; iter++)
    {
        sort(T + random_order[iter] * size, size);
    }
    time += get_time();

    printf("size %d test %d iters %ld\ttime: %.5lfs\n", size, test_id, iters, (double)time / 1e6);
}
