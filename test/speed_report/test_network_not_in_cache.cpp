#include "test_lib.hpp"

#define MAX_NETWORK_SIZE 40 * 16
int main()
{
    int *T = new int[NO_OF_INTEGERS];
    int64_t *random_order = new int64_t[NO_OF_INTEGERS];

    for (int reps = 0; reps < PASSES; reps++)
    {
        initRNG();
        for (int size = 1; size <= MAX_NETWORK_SIZE; size++)
        {
            run_test_not_cache(0, size, T, random_order);
        }
    }
}