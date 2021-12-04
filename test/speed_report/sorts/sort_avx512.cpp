#include "sort.hpp"

#include "../../../src/vectorized_sort.hpp"

void sort(int *T, int siz)
{
    quicksort512(T, siz);
}
