#include <bits/stdc++.h>
#include <sys/time.h>
#include "speed_report/sorts/sort.hpp"
const int M = 1024 * 1024 * 128;
int T[M + 1], T2[M + 1];

#define rand xorshf96
static unsigned long x = 123456789, y = 362436069, z = 521288629;
unsigned long xorshf96() //period 2^96-1
{
    unsigned long t;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
}

int R(int a, int b) { return rand() % (b - a + 1) + a; }
int main(int argc, char **argv)
{
    assert(argc > 1);
    int times = atoi(argv[1]);

    for (int i = 0; i < times; i++)
    {
        int siz = R(1, 1000 * 1000);
        for (int j = 0; j < siz; j++)
            T[j] = T2[j] = rand();

        std::sort(T2, T2 + siz);
        sort(T, siz);

        for (int j = 0; j < siz; j++)
        {
            assert(T[j] == T2[j]);
        }
    }
    printf("OK\n");
}