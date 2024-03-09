import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as ticker
#data = pd.read_csv('CS_CAIDA_topk1000.csv')
#data = pd.read_csv('CS_IMC_topk1000.csv')
#data = pd.read_csv('result.csv')
#data = pd.read_csv('CK_IMC_topk1000.csv')

#data = pd.read_csv('BS_CAIDA_mem100500_topk1001000.csv')
#data = pd.read_csv('BS_CAIDA_topk1000_mem100500.csv')
#data = pd.read_csv('BS_CAIDA_mem10k100k.csv')

data = pd.read_csv('result.csv')

#固定k=1000,内存从100-1000的性能
# x = data.iloc[:,0:1]
# print(set(x))
#CS
#keys=["cmsketch","hyperuss","wavingsketch","dasketch","CuckooCounter","CuckooSketch","heavykeeper","spacesaving"]#CK
#keys=["cmsketch","ASketch","CuckooCounter3","CuckooCounter","ElasticSketch","heavykeeper","LossyCounting","mvsketch","nitrosketch","spacesaving"]
keys=["CuckooCounter","CuckooCounter3","CuckooCounter31","CuckooCounter32","CuckooCounter33"]

labels = data.iloc[:, 0:2]
#CS
#labels={"CM-heap":"cmsketch","USS":"hyperuss","WS":"wavingsketch","DAS":"dasketch","CC":"CuckooCounter","BS":"CuckooSketch","HK":"heavykeeper","SS":"cmsketch"}#CK
#labels={"CM-heap":"cmsketch","AS":"ASketch","CK":"CuckooCounter3","CC":"CuckooCounter","EL":"ElasticSketch","HK":"heavykeeper","LC":"LossyCounting","MV":"mvsketch","NI":"nitrosketch","SS":"spacesaving"}
labels={"CC":"CuckooCounter","CC3":"CuckooCounter3","CC31":"CuckooCounter31","CC32":"CuckooCounter32","CC33":"CuckooCounter33"}


#colors={"CMHeap":"#E53A40","USS":"#a5dff9","WS":"#EFDC05","DAS":"#ef5285","CC":"#bcbd22","BS":"#30A9DE","HK":"#FFB353","SS":"#17becf"}
colors={"CC":"#30A9DE","CC3":"#EFDC05","CC31":"#E53A40","CC32":"#a5dff9","CC33":"#ef5285"}
label_size=8
zipf=0
AAE = {}
ARE = {}
throughput = {}
Precsion = {}
for index, row in data.iterrows():
    key = str(row[0])
    if key not in AAE:
        AAE[key] = []
        ARE[key] = []
        throughput[key] = []
        Precsion[key] = []
    AAE[key].append(row[2])
    ARE[key].append(row[3])
    Precsion[key].append(row[4])
    #Precsion[key].append(int(row[5].split('/')[0])/int(row[5].split('/')[1]))
    throughput[key].append(row[5])
print(AAE)
print(ARE)
print(Precsion)
print(throughput)
offset = 0.001  # 定义标记之间的偏移量
#markers={"CC":"<","cc31":">","CSP":"^","HK":"v","LC":"X","SS":"d","CMHeap":"s","USS":"p","WS":"D"}
markers = ['o', '>', '^', 'v', 'X', 'd', 's', '*', 'h', 'H', 'D', 'd', 'P']
#plt.figure(figsize=(10, 5.5))
#plt.figure()
#一张图的显示
#plt.legend(labels,bbox_to_anchor=(-0.25,2.5),ncol=10, loc='upper center',borderaxespad=0)
#plt.suptitle(labels) #总标题
plt.subplots_adjust(top=0.85) #总标题位置
#plt.subplots_adjust(wspace=0.4,hspace=0.4)

plt.grid(True)
i = 0
xtick=[]
data={}
data["cmsketch"]=[]
data["hyperuss"]=[]
data["wavingsketch"]=[]
data["dasketch"]=[]
data["CuckooCounter"]=[]
data["CuckooSketch"]=[]
data["heavykeeper"]=[]
data["spacesaving"]=[]
ms=5
#mem = ["10","20","30","40","50","60","70","80","90","100"]
#mem = ["100","200","300","400","500","600","700","800","900","1000"]
mem = ["0.1","0.2","0.3","0.4","0.5","0.6","0.7","0.8","0.9","1.0"]
#mem = ["0.1","0.2","0.3","0.4","0.5"]
#mem = ["0.6","0.7","0.8","0.9","1.0"]

#ax1 = plt.subplot(222)
ax1 = plt.subplot()
print(AAE)
for i, (key, value) in enumerate(AAE.items()):
    if key in keys:
        x = np.arange(len(value)) + i * offset  # 计算当前折线的横坐标点
        ax1.plot(x,value,'-o',label=key, marker=markers[i], markersize=ms, linestyle='-', alpha=1, linewidth=2, markerfacecolor='none', zorder=105)
    else:
        continue
#plt.title('', fontweight='bold', fontsize=label_size)
plt.xticks(range(len(mem)), mem)
plt.ylabel('AAE', fontweight='bold', fontsize=label_size)
plt.grid(True, linestyle=':', color='gray')
plt.xlabel(u'Memory(MB)\n ', fontweight='bold', fontsize=label_size)
#ax1.set_ylim(10**-1, 10**4)
ax1.set_yscale('log')
#filepath = '/Users/caolu/Downloads/cspic/IMC-AAE-topk1000.pdf'
#filepath = '/Users/caolu/Downloads/cspic/CAIDA-AAE-topk1000.pdf'
filepath = '4.png'
plt.subplots_adjust(wspace=0.4,hspace=0.4,bottom=0.13,left=0.12,top=0.84)
plt.legend(labels,bbox_to_anchor=(0.5,1.2),ncol=4, loc='upper center',borderaxespad=0)
plt.savefig(filepath)
plt.show()

i = 0
ax2 = plt.subplot()
plt.grid(axis='x',linestyle='-.',linewidth=1,color='black',alpha=0.5)
plt.grid(axis='y',linestyle='-.',linewidth=1,color='black',alpha=0.5)
for i, (key, value) in enumerate(ARE.items()):
    if key in keys:      
        x = np.arange(len(value)) + i * offset  # 计算当前折线的横坐标点
        ax2.plot(x,value, '-o', label=key, marker=markers[i], markersize=ms,linestyle='-', alpha=1,  linewidth=2, markerfacecolor='none')
    else:
        continue
#plt.title('(b)ARE-Top-k=1000',fontweight='bold', fontsize=label_size)
plt.tight_layout()
plt.xlabel(u'Memory(MB)\n ', fontweight='bold', fontsize=label_size)
plt.ylabel('ARE', fontweight='bold', fontsize=label_size)
plt.xticks(range(len(mem)), mem)
plt.grid(True, linestyle=':', color='gray')
#ax2.set_ylim(10**-3, 10**3)
#imc
#ax2.set_ylim(10**-2, 10**3)
ax2.set_yscale('log')
#filepath = '/Users/caolu/Downloads/cspic/IMC-ARE-topk1000.pdf'
#filepath = '/Users/caolu/Downloads/cspic/CAIDA-ARE-topk1000.pdf'
filepath = '5.png'
plt.subplots_adjust(wspace=0.4,hspace=0.4,bottom=0.13,left=0.12,top=0.84)
plt.legend(labels,bbox_to_anchor=(0.5,1.2),ncol=4, loc='upper center',borderaxespad=0)
plt.savefig(filepath)
plt.show()

ax3 = plt.subplot()
plt.grid(axis='x',linestyle='-.',linewidth=1,color='black',alpha=0.5)
plt.grid(axis='y',linestyle='-.',linewidth=1,color='black',alpha=0.5)
i = 0
for i, (key, value) in enumerate(throughput.items()):
    if key in keys:      
        x = np.arange(len(value)) + i * offset   # 计算当前折线的横坐标点
        ax3.plot(x,value, '-o', label=key, marker=markers[i], markersize=ms, linestyle='-', alpha=1, linewidth=2, markerfacecolor='none')
    else:
        continue
plt.xlabel(u'Memory(MB)\n', fontweight='bold', fontsize=label_size)
#plt.xlabel(u'Memory(MB)\n (a) IMC ', fontweight='bold', fontsize=label_size)
plt.ylabel(u'throughput(Mps)', fontweight='bold', fontsize=label_size)   
#plt.title(u'Top-k=1000', fontweight='bold', fontsize=label_size)   
plt.xticks(range(len(mem)), mem)
#ax3.set_ylim(10**-5, 10**3)
plt.grid(True, linestyle=':', color='gray')
#filepath = '/Users/caolu/Downloads/cspic/IMC-throughput-topk1000.pdf'
#filepath = '/Users/caolu/Downloads/cspic/CAIDA-throughput-topk1000.pdf'
filepath = '6.png'
plt.subplots_adjust(wspace=0.4,hspace=0.4,bottom=0.13,left=0.12,top=0.84)
plt.legend(labels,bbox_to_anchor=(0.5,1.2),ncol=4, loc='upper center',borderaxespad=0)
plt.savefig(filepath)
plt.show()

ax4 = plt.subplot()
plt.grid(True, linestyle='--', axis='y')
plt.grid(True, linestyle='--', axis='x')
i = 0
for i, (key, value) in enumerate(Precsion.items()):
    if key in keys:      
        x = np.arange(len(value)) + i * offset   # 计算当前折线的横坐标点
        ax4.plot(x,value, '-o', label=key, marker=markers[i], markersize=ms, linestyle='-', alpha=1, linewidth=2,markerfacecolor='none')
    else:
        continue
plt.xlabel(u'Memory(MB)\n ', fontweight='bold', fontsize=label_size)
#plt.xlabel(u'Memory(MB)\n (d) IMC ', fontweight='bold', fontsize=label_size)
plt.xticks(range(len(mem)), mem)
plt.ylabel(u'precision', fontweight='bold', fontsize=label_size)    
#plt.title(u'Top-k=1000', fontweight='bold', fontsize=label_size)            
plt.grid(True, linestyle=':', color='gray')
#141 坐标
#plt.legend(labels,bbox_to_anchor=(-1.5,1.3),ncol=10, loc='upper center',borderaxespad=0)
#plt.subplots_adjust(wspace=0.375,hspace=0.32,bottom=0.2,left=0.055,top=0.5)
# 211 221 坐标
plt.legend(labels,bbox_to_anchor=(0.5,1.2),ncol=4, loc='upper center',borderaxespad=0)
ax4.set_ylim(0.4, 1.0)
plt.subplots_adjust(wspace=0.4,hspace=0.4,bottom=0.13,left=0.12,top=0.84)
#filepath = '/Users/caolu/Downloads/cspic/IMC-topk1000.pdf'
filepath = '7.png'
#filepath = '/Users/caolu/Downloads/cspic/IMC-precision-topk1000.pdf'
#filepath = '/Users/caolu/Downloads/cspic/CAIDA-topk1000.pdf'
plt.savefig(filepath)
plt.show()


