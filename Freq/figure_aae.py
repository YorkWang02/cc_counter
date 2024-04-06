import pandas as pd
import matplotlib.pyplot as plt
import csv

data = pd.read_csv('output.csv')

keys = ["CC", "CC2", "CC3"]
labels = {"CC": "CC", "CC2": "CC2", "CC3": "CC3"}
colors = {"CC": "black", "CC2": "red", "CC3": "yellow"}

insertThroughput = {}
queryThroughput = {}
AAE = {}
ARE = {}

with open('output.csv', newline='') as csvfile:
    reader = csv.reader(csvfile, delimiter=',', quotechar='|')
    next(reader)
    for row in reader:
        mem = float(row[0])
        func = str(row[1])
        throughput_insert = float(row[2])
        throughput_query = float(row[3])
        aae = float(row[4])
        are = float(row[5])
        if aae == 0.0:
            aae = 10**-3
        if are == 0.0:
            are = 10**-3
    
        if (mem, func) not in insertThroughput:
            insertThroughput[(mem, func)] = []
            queryThroughput[(mem, func)] = []
            AAE[(mem, func)] = []
            ARE[(mem, func)] = []
    
        insertThroughput[(mem, func)].append(throughput_insert)
        queryThroughput[(mem, func)].append(throughput_query)
        AAE[(mem, func)].append(aae)
        ARE[(mem, func)].append(are)
       
average_insertThroughput = {}
average_queryThroughput = {}
average_AAE = {}
average_ARE = {}

for (mem, func), values in insertThroughput.items():
    average_insertThroughput[(mem, func)] = sum(values) / len(values)

for (mem, func), values in queryThroughput.items():
    average_queryThroughput[(mem, func)] = sum(values) / len(values)

for (mem, func), values in AAE.items():
    average_AAE[(mem, func)] = sum(values) / len(values)

for (mem, func), values in ARE.items():
    average_ARE[(mem, func)] = sum(values) / len(values)


markers = ['<', '>', '^', 'v', 'x', 'd', 's', '*', 'h', 'H', 'D', 'd', 'P']

plt.grid(True, linestyle='--', axis='y')
plt.grid(True, linestyle='--', axis='x')

plt.figure(figsize=(8, 6))

ax1 = plt.subplot(111)

for index, func in enumerate(keys):
    mems = [mem_func[0] for mem_func in average_AAE.keys() if mem_func[1] == func]
    aae = [average_AAE[(mem, func)] for mem in mems]
    ax1.plot(mems, aae, marker=markers[index], label=labels[func], linestyle='-',
             alpha=1, linewidth=2, markerfacecolor='none', zorder=105, color=colors[func])

plt.title('Average AAE', fontweight='bold', fontsize=12)
plt.xlabel('Memory (KB)', fontweight='bold', fontsize=10)
plt.ylabel('Average AAE', fontweight='bold', fontsize=10)
plt.grid(True, linestyle='--', axis='y')
plt.grid(True, linestyle='--', axis='x')
plt.xlim(min(mems), max(mems))  
plt.ylim(10**0, 10**2)  
plt.legend(bbox_to_anchor=(0.5, 1.2), ncol=3, loc='upper center', borderaxespad=0)
plt.yscale('log')
plt.tight_layout()

filepath = 'figure_aae.pdf'
plt.subplots_adjust(wspace=0.4, hspace=0.4, bottom=0.13, left=0.12, top=0.84)
plt.savefig(filepath)
plt.show()
