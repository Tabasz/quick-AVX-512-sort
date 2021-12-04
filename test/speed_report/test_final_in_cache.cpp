#include "test_lib.hpp"

int main()
{
    int *T = new int[NO_OF_INTEGERS];

    for (int reps = 0; reps < PASSES; reps++)
    {
        initRNG();
        for (int size = 10; size <= 1000 * 1000 * 1000; size *= 10)
        {
            printf("length of array: %d\n", size);
            for (int test_id = 0; test_id < NUM_TESTS; test_id++)
            {
                run_test_cache(test_id, size, T);
            }
        }
    }
}