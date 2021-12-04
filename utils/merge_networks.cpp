#include <bits/stdc++.h>
using namespace std;

void doCOEX(int a, int b, int siz)
{
    if (a < siz && b < siz)
    {
        printf("        COEX16(vecs[%d], vecs[%d]);\n", a, b);
    }
}

//prints merging code inside column, merges n rows with m rows (n >= m), n is a power of two
void printColumnMerge(int n, int m)
{
    printf("    case %d:\n", m);
    for (int i = 0; i < n; i++)
    {
        doCOEX(i, i + n, n + m);
    }
    for (int d = n / 2; d >= 1; d /= 2)
    {
        for (int i = d; i < n + m - d; i += 2 * d)
        {
            for (int j = 0; j < d; j++)
            {
                doCOEX(i + j, i + j + d, n + m);
            }
        }
    }
    printf("        break;\n");
}

map<int, int> vectorNumber;

void coex(int a, int b, int m)
{
    if (vectorNumber[a] > vectorNumber[b])
    {
        swap(vectorNumber[a], vectorNumber[b]);
    }
    if (vectorNumber[a] >= m || vectorNumber[b] >= m)
    {
        return;
    }
    printf("        COEX16(vecs[%d], vecs[%d]);\n", vectorNumber[a], vectorNumber[b]);
}

// prints column merging code for m vectors, n must be a smallest power of to not less than m
void printOddEvenMerge(int n, int m)
{
    vectorNumber.clear();
    printf("    case %d:\n", m);

    //two vectors
    for (int i = 0; i < n; i += 2)
    {
        if (i < m || i + 1 < m)
        {
            printf("        tmp = vecs[%d];\n", i);
            printf("        vecs[%d] = _mm512_unpacklo_epi32(vecs[%d], vecs[%d]);\n", i, i + 1, i);
            printf("        vecs[%d] = _mm512_unpackhi_epi32(tmp, vecs[%d]);\n", i + 1, i + 1);
            printf("        COEX16(vecs[%d], vecs[%d]);\n", i, i + 1);
        }
        vectorNumber[i] = i;
        vectorNumber[i + n] = i + 1;
    }
    for (int dist = n / 2; dist > 1; dist /= 2)
    {
        for (int i = dist; i < 2 * n - dist; i+= dist * 2)
        {
            for (int j = 0; j < dist; j += 2)
            {
                coex(i + j, i + dist + j, m);
            }
        }
    }
    for (int i = 0; i < m; i += 2)
    {
        printf("        finishMerge1(vecs[%d], vecs[%d]);\n", i, i + 1);
    }

    //four vectors
    vectorNumber.clear();
    for (int i = 0; i < n; i += 2)
    {
        vectorNumber[2 * i] = i;
        vectorNumber[2 * i + 2 * n] = i + 1;
    }
    for (int dist = n; dist > 2; dist /= 2)
    {
        for (int i = dist; i < 4 * n - dist; i+= dist * 2)
        {
            for (int j = 0; j < dist; j += 4)
            {
                coex(i + j, i + dist + j, m);
            }
        }
    }
    for (int i = 0; i < m; i += 2)
    {
        printf("        finishMerge2(vecs[%d], vecs[%d]);\n", i, i + 1);
    }

    //eight vectors
    vectorNumber.clear();
    for (int i = 0; i < n; i += 2)
    {
        vectorNumber[4 * i] = i;
        vectorNumber[4 * i + 4 * n] = i + 1;
    }
    for (int dist = n * 2; dist > 4; dist /= 2)
    {
        for (int i = dist; i < 8 * n - dist; i+= dist * 2)
        {
            for (int j = 0; j < dist; j += 8)
            {
                coex(i + j, i + dist + j, m);
            }
        }
    }
    for (int i = 0; i < m; i += 2)
    {
        printf("        finishMerge3(vecs[%d], vecs[%d]);\n", i, i + 1);
    }
    
    //sixteen vectors
    vectorNumber.clear();
    for (int i = 0; i < n; i += 2)
    {
        vectorNumber[8 * i] = i;
        vectorNumber[8 * i + 8 * n] = i + 1;
    }
    for (int dist = n * 4; dist > 8; dist /= 2)
    {
        for (int i = dist; i < 16 * n - dist; i+= dist * 2)
        {
            for (int j = 0; j < dist; j += 16)
            {
                coex(i + j, i + dist + j, m);
            }
        }
    }
    for (int i = 0; i < m; i += 2)
    {
        printf("        finishMerge4(vecs[%d], vecs[%d]);\n", i, i + 1);
    }
    printf("        break;\n");
}

int main(int argc, char **argv)
{
    unsigned int value;
    if (argc != 2 || (value = stoi(argv[1])) > 1)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\t%s 0 - generates modified Odd-even network for column merging\n", argv[0]);
        fprintf(stderr, "\t%s 1 - generates Odd-even network for merging parts of columns\n", argv[0]);
        exit(1);
    }
    if (value == 0)
    {
        printf("void mergeNetwork(__m512i *vecs, int l)\n{\n");
        printf("    __m512i tmp;\nswitch (l)\n{\n");
        for (int i = 1; i <= 32; i *= 2)
        {
            for (int j = 1; j <= i; j++)
            {
                printOddEvenMerge(i * 2, i + j);
            }
        }
        printf("    default: assert(false && \"not implemented network!\");\n");
        printf("}\n}\n");
    }
    else
    {
        printf("static inline FORCE_INLINE void columnNetworkMerge(__m512i* vecs, int m)\n{\n");
        printf("    switch (m)\n    {\n");

        for (int i = 1; i <= 32; i++)
        {
            printColumnMerge(32, i);
        }

        printf("    default: assert(false && \"not implemented network!\");\n");
        printf("    }\n}\n");
    }
}