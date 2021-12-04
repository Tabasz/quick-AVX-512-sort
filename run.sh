ID=999

make test1_stlsort && time ./bin/test1_stlsort > log/run_log_test1_stlsort_${ID}
make test2_stlsort && time ./bin/test2_stlsort > log/run_log_test2_stlsort_${ID}

make test1_avx2 && time ./bin/test1_avx2 > log/run_log_test1_avx2_${ID}
make test2_avx2 && time ./bin/test2_avx2 > log/run_log_test2_avx2_${ID}

make test1_avx512bramas && time ./bin/test1_avx512bramas > log/run_log_test1_avx512bramas_${ID}
make test2_avx512bramas && time ./bin/test2_avx512bramas > log/run_log_test2_avx512bramas_${ID}

make test1_512 && time ./bin/test1_512 > log/run_log_test1_512_${ID}
make test2_512 && time ./bin/test2_512 > log/run_log_test2_512_${ID}

# networks
make test3_512 && time ./bin/test3_512 > log/run_log_test3_512_${ID}
make test4_512 && time ./bin/test4_512 > log/run_log_test4_512_${ID}

make test3_avx2 && time ./bin/test3_avx2 > log/run_log_test3_avx2_${ID}
make test4_avx2 && time ./bin/test4_avx2 > log/run_log_test4_avx2_${ID}