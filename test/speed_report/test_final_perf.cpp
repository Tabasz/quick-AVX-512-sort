#include <sys/time.h>
#include "test_lib.hpp"
#include "perf_lib.hpp"

int main(int argc, char **argv)
{
    int *T = new int[NO_OF_INTEGERS];

    int siz = atoi(argv[1]);
    int test_id = atoi(argv[2]);

    for (int reps = 0; reps < 5; reps++)
    {
        initRNG();
        long long times = NO_OF_INTEGERS / siz;
        long long time = 0;

        if (siz < 512)
        {
            test[test_id](T, siz);

            PerfEvent e;
            e.startCounters();
            time -= get_time();
            for (int iter = 0; iter < times; iter++)
            {
                sort(T + iter * siz, siz);
            }
            time += get_time();
            e.stopCounters();
            e.printReport(std::cout, 1); // use n as scale factor
        }
        else
        {
            for (int iter = 0; iter < times; iter++)
            {
                test[test_id](T, siz);

                PerfEvent e;
                e.startCounters();
                time -= get_time();
                sort(T, siz);
                time += get_time();
                e.stopCounters();
                if (iter >= times - 10)
                    e.printReport(std::cout, 1); // use n as scale factor
            }
        }
        fprintf(stderr, "                                                      size %d test %d iters %lld\ttime: %.5lfs\n", siz, test_id, times, (double)time / 1e6);
    }
}