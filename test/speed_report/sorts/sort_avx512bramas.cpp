#include "sort.hpp"

#include "../../avx512bramas/avx512bramas.cpp"

void sort(int *T, int siz)
{
    Sort512::Sort(T, siz);
}
