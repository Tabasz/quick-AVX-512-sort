# c:\Users\m00627654\Downloads\WPy64-3950\scripts\cmd.bat

import math

SIZ = 9
def read_file(file, stl):
    result = [[1e9] * SIZ, [1e9] * SIZ, [1e9] * SIZ, [1e9] * SIZ]
    for line in open(file):
        if 'size' not in line: continue
        tokens = line.split()
        # print(tokens)
        size = int(tokens[1])
        test = int(tokens[3])
        iters = int(tokens[5])
        time = float(tokens[7][:-1])
        time = time / iters
        idx = int(math.log10(size) - 1)
        result[test][idx] = min(result[test][idx], time)

    if stl is not None:
        for i in range(4):
            # assert(len(result[i]) == SIZ)
            for j in range(len(result[0])):
                result[i][j] = stl[i][j] / result[i][j]

    return result

stl_sort = read_file('log0_stl.txt', None)
results512 = read_file('log0_avx512.txt', stl_sort)
results2 = read_file('log0_avx2.txt', stl_sort)
for i in range(4):
    print(results512[i])
    for j in range(SIZ):
        print(results512[i][j] / results2[i][j], end = ' ')
    print('')
# print('')
# for i in range(4):
#     print(results2[i])



import matplotlib.pyplot as plt
import numpy as np

# evenly sampled time at 200ms intervals
t = np.arange(0., 5., 0.2)
plt.ylabel('speedup over std::sort')

# red dashes, blue squares and green triangles
# plt.plot(t, t, 'r--', t, t**2, 'bs', t, t**3, 'g^')

idx = np.array([10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000])
# idx = np.array([0, 1, 2, 3, 4, 5, 6, 7, 8])
#https://matplotlib.org/stable/api/_as_gen/matplotlib.pyplot.plot.html
plt.plot(idx, results512[0], 'g^-', label = 'nasze', linewidth=1, markersize=6)
plt.plot(idx, results2[0], 'bs-', label = 'ich', linewidth=1, markersize=4)
# plt.plot(idx, results512[0], 'green', label = 'nasze')

# plt.plot(idx, results2[0], 'bs')
# plt.plot(idx, results2[0], 'blue')

# Rewrite the y labels
ax = plt.gca()


# ax.set_xticklabels(idx)
# plt.rcParams.update({'font.size': 5})
# plt.rc('axes', labelsize=5)
#https://matplotlib.org/stable/tutorials/text/text_intro.html
ax.set_xscale('log')
# ax.xaxis.set_ticks(idx)
ax.set_xticks(idx)

ax.set_yticks([0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30])

plt.grid(True, linewidth=0.1, color='#aaaaaa', linestyle='-')
plt.legend(loc="upper left")
plt.show()
