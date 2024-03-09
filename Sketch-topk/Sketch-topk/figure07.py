import pandas as pd
import matplotlib.pyplot as plt
import csv

data = pd.read_csv('result.csv')

# keys = ["CuckooCounter", "CuckooCounter3", "cmsketch", "ASketch", "heavykeeper", "mvsketch"]
# labels = {"CuckooCounter": "CuckooCounter", "CuckooCounter3": "HeavyFinder", "cmsketch": "CM-heap",
#           "ASketch": "AS", "heavykeeper": "HK", "mvsketch": "MV"}
# colors = {"CuckooCounter": "black", "CuckooCounter3": "red", "cmsketch": "blue", "ASketch": "green",
#           "heavykeeper": "purple", "mvsketch": "yellow"}

keys = ["CuckooCounter", "CuckooCounter31", "cmsketch", "ASketch", "heavykeeper", "mvsketch"]
labels = {"CuckooCounter": "CC", "CuckooCounter31": "MC", "cmsketch": "CM",
          "ASketch": "AS", "heavykeeper": "HK", "mvsketch": "MV"}
colors = {"CuckooCounter": "black", "CuckooCounter31": "red", "cmsketch": "blue", "ASketch": "green",
          "heavykeeper": "purple", "mvsketch": "yellow"}

insertThroughput = {}
AAE = {}
ARE = {}
_sum = {}

with open('result.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='|')
    next(reader)
    for row in reader:
        mem = int(row[0])
        func = str(row[1])
        topk = int(row[2])
        aae = float(row[3])
        are = float(row[4])
        mismatch = int(row[5].split('/')[0]) / int(row[5].split('/')[1])
        throughput = float(row[6])
        if aae == 0.0:
            aae = 10**-3
        if are == 0.0:
            are = 10**-6

        if topk not in insertThroughput:
            insertThroughput[topk] = {}
            AAE[topk] = {}
            ARE[topk] = {}
            _sum[topk] = {}
    
        if mem not in insertThroughput[topk]:
            insertThroughput[topk][mem] = {}
            AAE[topk][mem] = {}
            ARE[topk][mem] = {}
            _sum[topk][mem] = {}
    
        if func not in insertThroughput[topk][mem]:
            insertThroughput[topk][mem][func] = []
            AAE[topk][mem][func] = []
            ARE[topk][mem][func] = []
            _sum[topk][mem][func] = []
     
        insertThroughput[topk][mem][func].append(throughput)
        AAE[topk][mem][func].append(aae)
        ARE[topk][mem][func].append(are)
        _sum[topk][mem][func].append(mismatch)

topks = list(insertThroughput.keys())
funcs = list(insertThroughput[topks[0]][100].keys())

markers = ['<', '>', '^', 'v', 'x', 'd', 's', '*', 'h', 'H', 'D', 'd', 'P']

plt.grid(True, linestyle='--', axis='y')
plt.grid(True, linestyle='--', axis='x')

for topk in topks:
    ax1 = plt.subplot(111)
    mems = list(insertThroughput[topk].keys())
    for index, func in enumerate(funcs):
        are = [ARE[topk][mem][func] for mem in mems]
        print(are)
        ax1.plot(range(len(mems)), are, marker=markers[index], label=labels[func], linestyle='-',
                 alpha=1, linewidth=2, markerfacecolor='none', zorder=105, color=colors[func])

    plt.xticks(range(len(mems)), mems)
    #plt.title(f'k={topk}', fontweight='bold', fontsize=10)
    plt.xlabel(u'Memory(KB)', fontweight='bold', fontsize=10)
    plt.grid(True, linestyle='--', axis='y')
    plt.grid(True, linestyle='--', axis='x')
    plt.ylabel(u'ARE', fontweight='bold', fontsize=10)
    plt.subplots_adjust(wspace=0.4, hspace=0.4, bottom=0.13, left=0.12, top=0.84)
    plt.legend(bbox_to_anchor=(0.5, 1.2), ncol=3, loc='upper center', borderaxespad=0)
    ax1.set_ylim(10**-4, 10**3)
    ax1.set_yscale('log')

filepath = 'result/MergeCounter/800/are.pdf'
plt.subplots_adjust(wspace=0.4, hspace=0.4, bottom=0.13, left=0.12, top=0.84)
plt.savefig(filepath)
plt.show()
