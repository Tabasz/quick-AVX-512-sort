OBJS512			= bin/avx512networks.o bin/avx512sort.o bin/merge_networks.o
DEPS			= src/defines.hpp src/vectorized_sort.hpp src/column_networks.hpp src/merge_networks.hpp
MAKEFILE		= Makefile
CXX				= g++
CXXFLAGS		= -O3 -Wall -Wextra -march=native -funroll-loops

clean:
	rm -f *.o *.e bin/*.o bin/*.e

test1_512: $(OBJS512) test/speed_report/test_final_in_cache.cpp test/speed_report/sorts/sort_avx512.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test2_512: $(OBJS512) test/speed_report/test_final_not_in_cache.cpp test/speed_report/sorts/sort_avx512.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test3_512: $(OBJS512) test/speed_report/test_network_in_cache.cpp test/speed_report/sorts/sort_avx512.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test4_512: $(OBJS512) test/speed_report/test_network_not_in_cache.cpp test/speed_report/sorts/sort_avx512.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)


test_perf_512: $(OBJS512) test/speed_report/test_final_perf.cpp test/speed_report/sorts/sort_avx512.cpp 
	$(CXX) -o bin/$@ $^ $(CXXFLAGS) -static-libstdc++

test_perf_avx2: $(OBJS512) test/speed_report/test_final_perf.cpp test/speed_report/sorts/sort_avx2.cpp 
	$(CXX) -o bin/$@ $^ $(CXXFLAGS) -static-libstdc++


test1_avx2: test/speed_report/test_final_in_cache.cpp test/speed_report/sorts/sort_avx2.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test2_avx2: test/speed_report/test_final_not_in_cache.cpp test/speed_report/sorts/sort_avx2.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test3_avx2: test/speed_report/test_network_in_cache.cpp test/speed_report/sorts/sort_avx2.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test4_avx2: test/speed_report/test_network_not_in_cache.cpp test/speed_report/sorts/sort_avx2.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test1_avx512bramas: test/speed_report/test_final_in_cache.cpp test/speed_report/sorts/sort_avx512bramas.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS) -DSORT512BARAMAS

test2_avx512bramas: test/speed_report/test_final_not_in_cache.cpp test/speed_report/sorts/sort_avx512bramas.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS) -DSORT512BARAMAS

test1_stlsort: test/speed_report/test_final_in_cache.cpp test/speed_report/sorts/sort_stl.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

test2_stlsort: test/speed_report/test_final_not_in_cache.cpp test/speed_report/sorts/sort_stl.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)


correctness: $(OBJS512) test/correctness.cpp test/speed_report/sorts/sort_avx512.cpp
	$(CXX) -o bin/$@ $^ $(CXXFLAGS)

bin/%.o: src/%.cpp $(DEPS) $(MAKEFILE)
	$(CXX) -c $(CXXFLAGS) -o $@ $<