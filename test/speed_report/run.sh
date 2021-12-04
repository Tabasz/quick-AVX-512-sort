if [ "$#" -ne 1 ]; then
    echo "Illegal number of parameters"
    exit 1
fi


g++ speed_report/test_final.cpp speed_report/sort_avx512.c -o test_final -O3 -Wall -march=native -funroll-loops
./test_final 2> log$1_avx512

g++ speed_report/test_final.cpp speed_report/sort_avx2.c -o test_final -O3 -Wall -march=native -funroll-loops
./test_final 2> log$1_avx2

g++ speed_report/test_final.cpp speed_report/sort_stl.c -o test_final -O3 -Wall -march=native -funroll-loops
./test_final 2> log$1_sort