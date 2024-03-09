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
labels = {"CuckooCounter": "CuckooCounter", "CuckooCounter31": "MergeCounter", "cmsketch": "CM-heap",
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

        if mem not in insertThroughput:
            insertThroughput[mem] = {}
            AAE[mem] = {}
            ARE[mem] = {}
            _sum[mem] = {}
    
        if topk not in insertThroughput[mem]:
            insertThroughput[mem][topk] = {}
            AAE[mem][topk] = {}
            ARE[mem][topk] = {}
            _sum[mem][topk] = {}
    
        if func not in insertThroughput[mem][topk]:
            insertThroughput[mem][topk][func] = []
            AAE[mem][topk][func] = []
            ARE[mem][topk][func] = []
            _sum[mem][topk][func] = []
     
        insertThroughput[mem][topk][func].append(throughput)
        AAE[mem][topk][func].append(aae)
        ARE[mem][topk][func].append(are)
        _sum[mem][topk][func].append(mismatch)

mems = list(insertThroughput.keys())
funcs = list(insertThroughput[mems[0]][100].keys())

markers = ['<', '>', '^', 'v', 'x', 'd', 's', '*', 'h', 'H', 'D', 'd', 'P']

plt.grid(True, linestyle='--', axis='y')
plt.grid(True, linestyle='--', axis='x')

for mem in mems:
    ax1 = plt.subplot(111)
    topks = list(insertThroughput[mem].keys())
    for index, func in enumerate(funcs):
        precision = [_sum[mem][topk][func] for topk in topks]
        ax1.plot(range(len(topks)), precision, marker=markers[index], label=labels[func], linestyle='-',
                 alpha=1, linewidth=2, markerfacecolor='none', zorder=105, color=colors[func])

    plt.xticks(range(len(topks)), topks)
    plt.title(f'Memory({mem}KB)', fontweight='bold', fontsize=10)
    plt.xlabel(u'Topk(num)', fontweight='bold', fontsize=10)
    plt.grid(True, linestyle='--', axis='y')
    plt.grid(True, linestyle='--', axis='x')
    plt.ylabel(u'Precision', fontweight='bold', fontsize=10)
    plt.subplots_adjust(wspace=0.4, hspace=0.4, bottom=0.13, left=0.12, top=0.84)
    plt.legend(bbox_to_anchor=(0.5, 1.2), ncol=3, loc='upper center', borderaxespad=0)
    ax1.set_ylim(0.1, 1.0)

# filepath = 'result/HeavyFinder/100KB/precision.pdf'
filepath = 'result/MergeCounter/100KB/precision.pdf'
plt.subplots_adjust(wspace=0.4, hspace=0.4, bottom=0.13, left=0.12, top=0.84)
plt.savefig(filepath)
plt.show()
