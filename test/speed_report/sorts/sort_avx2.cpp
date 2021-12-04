#include "sort.hpp"

#include "../../avx2/avx2.cpp"

void sort(int *T, int siz)
{
    avx2::quicksort(T, siz);
}
