import pandas as pd
import matplotlib.pyplot as plt
import csv
import matplotlib.ticker as ticker
#from brokenaxes import brokenaxes
#自动出k=100-1000，内存从100kb�?1000kb的图
#data = pd.read_csv('result.csv')
#data= pd.read_csv('BS_mem100500_topk1001000.csv')
#plt.figure(figsize=(10, 5.5))
#data = pd.read_csv('CK_caida100_1000topk1000_10000.csv')
#data = pd.read_csv('CK_mem500_topk1000_9000.csv')
#data = pd.read_csv('CK_IMCmem100topk1000_2000.csv')
#data = pd.read_csv('iK_IMCmem500_topk1000_9000.csv')
#data = pd.read_csv('IMC1001000_topk1001000.csv')

#data = pd.read_csv('CS_IMC_mem100_1000_topk_100_1000.csv')
#data = pd.read_csv('CS_CAIDA_mem100_1000_topk_100_1000.csv')

data = pd.read_csv('result.csv')
#data = pd.read_csv('BS_IMC_mem10k100k.csv')

#CS
# keys=["cmsketch","hyperuss","wavingsketch","dasketch","CuckooCounter","CuckooSketch","heavykeeper","spacesaving"]
#keys=["CuckooCounter","CuckooCounter3","CuckooCounter31","CuckooCounter32","CuckooCounter33"]
# keys=["CuckooCounter3","CuckooCounter31","CuckooCounter32","CuckooCounter33"]
keys=["CuckooCounter","CuckooCounter3","cmsketch","ASketch","heavykeeper","mvsketch","spacesaving"]

labels = data.iloc[:, 0:2]
# labels={"CM-heap":"cmsketch","USS":"hyperuss","WS":"wavingsketch","DAS":"dasketch","CC":"CuckooCounter","BS":"CuckooSketch","HK":"heavykeeper","SS":"cmsketch"}
#labels={"CC":"CuckooCounter","CC3":"CuckooCounter3","CC31":"CuckooCounter31","CC32":"CuckooCounter32","CC33":"CuckooCounter33"}
# labels={"CC3":"CuckooCounter3","CC31":"CuckooCounter31","CC32":"CuckooCounter32","CC33":"CuckooCounter33"}
labels={"CC":"CuckooCounter","No-Heap-CC":"CuckooCounter3","CM-heap":"cmsketch","AS":"ASketch","HK":"heavykeeper","MV":"mvsketch","SS":"spacesaving"}

tick_size=9
label_size=10
# colors={"BS":"#30A9DE","WS":"#EFDC05","CMHeap":"#E53A40","USS":"#a5dff9","DAS":"#ef5285","HK":"#FFB353","CC":"#bcbd22","SS":"#17becf"}
#colors={"CC":"#30A9DE","CC3":"#EFDC05","CC31":"#E53A40","CC32":"#a5dff9","CC33":"#ef5285"}
# colors={"CC3":"#EFDC05","CC31":"#E53A40","CC32":"#a5dff9","CC33":"#ef5285"}
colors={"CC":"#30A9DE","No-Heap-CC":"#EFDC05","CM-heap":"#E53A40","AS":"#a5dff9","HK":"#ef5285","MV":"#FFB353","SS":"#bcbd22"}
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
# topk_range = range(100, 1001, 100)
plt.grid(True, linestyle='--', axis='y')
plt.grid(True, linestyle='--', axis='x')
markers = ['<', '>', '^', 'v', 'x', 'd', 's', '*', 'h', 'H', 'D', 'd', 'P']

#markers={"CM":"<","SS":">","LC":"^","USS":"v","BM":"X","HK":"d","WS":"s","CS":"p"}
for mem in mems:
    #plt.figure(figsize=(10, 4.5))
    #plt.subplots_adjust(wspace=0.4,hspace=0.4)
    ax1 = plt.subplot(221)
    topks = list(insertThroughput[mem].keys())
    for index, func in enumerate(funcs):
        throughputs = [insertThroughput[mem][topk][func] for topk in topks]
        ax1.plot(range(len(topks)), throughputs, marker=markers[index],label=func, linestyle='-', alpha=1, linewidth=2, markerfacecolor='none', zorder=105)
        #plt.plot(range(len(topks)), throughputs, marker=markers[index],label=func,markeredgewidth=2, markerfacecolor='none', zorder=105)
    # 设置图表标题和标�?
    plt.xticks(range(len(topks)), topks)
    plt.title(f'Memory({mem}KB)',fontweight='bold', fontsize=label_size)
    plt.xlabel(u'Topk(num)', fontweight='bold', fontsize=label_size)
    plt.grid(True, linestyle='--', axis='y')
    plt.grid(True, linestyle='--', axis='x')
    plt.ylabel(u'Insert throughput(Mps)', fontweight='bold', fontsize=label_size)   
    plt.subplots_adjust(wspace=0.4,hspace=0.4,bottom=0.13,left=0.12,top=0.84)
    plt.legend(labels,bbox_to_anchor=(0.5,1.2),ncol=4, loc='upper center',borderaxespad=0)
   # plt.figure()
    ax2= plt.subplot(222)
   # y=[pow(10,i) for i in range(0,10)]
    for index, func in enumerate(funcs):
        aae = [AAE[mem][topk][func] for topk in topks]
        ax2.plot(range(len(topks)), aae, marker=markers[index], label=func, linestyle='-', alpha=1, linewidth=2, markerfacecolor='none', zorder=105)
    # 设置图表标题和标�?
    plt.xticks(range(len(topks)), topks)
    plt.title(f'AAE ({mem}KB)',fontweight='bold', fontsize=label_size)
    plt.xlabel(u'Topk(num)', fontweight='bold', fontsize=label_size)
    plt.ylabel(u'AAE', fontweight='bold', fontsize=label_size)  
    plt.grid(True, linestyle=':', color='gray')
    #100
    ax2.set_ylim(10**-2, 10**5)
    #500
    # ax2.set_ylim(10**-1, 10**5)
  #imc
    #ax2.set_ylim(10**-2, 10**3)
    ax2.set_yscale('log')


    ax3= plt.subplot(223)
    for index, func in enumerate(funcs):
        are = [ARE[mem][topk][func] for topk in topks]
        ax3.plot(range(len(topks)), are, marker=markers[index], label=func, linestyle='-', alpha=1, linewidth=2, markerfacecolor='none', zorder=105)
    # 设置图表标题和标�?
    plt.xticks(range(len(topks)), topks)
    plt.title(f'ARE ({mem}KB)',fontweight='bold', fontsize=label_size)
    #plt.xlabel('topk')
    plt.xlabel(u'Topk(num)', fontweight='bold', fontsize=label_size)
    plt.ylabel(u'ARE', fontweight='bold', fontsize=label_size)    
    plt.grid(True, linestyle='--', axis='y')
    plt.grid(True, linestyle='--', axis='x')   
    #  ax3.set_ylim(10**-5, 10**2)
   #500
    ax3.set_ylim(10**-5, 10**2)
    ax3.set_yscale('log')
   # plt.legend()

    ax4= plt.subplot(224)
    for index, func in enumerate(funcs):
        precision = [_sum[mem][topk][func] for topk in topks]
        ax4.plot(range(len(topks)), precision, marker=markers[index], label=func, linestyle='-', alpha=1, linewidth=2, markerfacecolor='none', zorder=105)
    # 设置图表标题和标�?
    plt.xticks(range(len(topks)), topks)
    plt.title(f'precision ({mem}KB)',fontweight='bold', fontsize=label_size)
    plt.xlabel(u'Topk(num)', fontweight='bold', fontsize=label_size)
    plt.ylabel(u'precision', fontweight='bold', fontsize=label_size)    
    plt.grid(True, linestyle=':', color='gray')
    #caida
    ax4.set_ylim(0.1, 1.0)
#imc
   # ax4.set_ylim(10**-1, 1.0)
filepath = '100KB.pdf'
plt.subplots_adjust(wspace=0.4,hspace=0.4,bottom=0.13,left=0.12,top=0.84)
plt.savefig(filepath) 
plt.show()   
