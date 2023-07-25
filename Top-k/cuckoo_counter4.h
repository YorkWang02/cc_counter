#ifndef _CCCOUNTER4_H
#define _CCCOUNTER4_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <unordered_map>
#include "BOBHASH32.h"
#include "params.h"
#include "ssummary.h"
#include "BOBHASH64.h"
#define CC_d 2
#define BN 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cuckoocounter4
{
private:
	int *count;
	char **items;	//过滤器中的条目名
    int pointer;    //指示过滤器中的空闲位置

	struct node { int C, FP; } HK[CC_d][MAX_MEM + 10][BN];
	BOBHash64 * bobhash;
	int K, M2;
    int threshold;
	
public:
	cuckoocounter4(int M2, int K,int value) :M2(M2), K(K), threshold(value)
	{
        //初始化过滤器
		items = new char *[K];
		for(int i = 0; i < K; i++)
		{
			items[i] = new char[100];
			items[i][0] = '\0';
		}

		count = new int[K];
		memset(count, 0, sizeof(int) * K);

        pointer = 0;

        bobhash = new BOBHash64(1005);      
    }
	void clear()
	{
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				for(int r=0;r<BN;r++)HK[i][j][r].C = HK[i][j][r].FP = 0;
	}
	unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}

	void rehash(node hash_entry, int loop_times, int j, unsigned long long hash) {
		int k=1-j;
		unsigned long long re_hash = hash ^ Hash(std::to_string(hash_entry.FP));
		int Hsh = re_hash % (M2 - (2 * CC_d) + 2 *(1-j) + 3);
		for (int r = 0; r < BN; r++) {
			if (HK[k][Hsh][r].FP == 0) {
				HK[k][Hsh][r].FP = hash_entry.FP;
				HK[k][Hsh][r].C = hash_entry.C;
				return;
			}
		}
		if (loop_times == 0) {//if the loop times bigger than the thresh
			HK[k][Hsh][1].FP = hash_entry.FP;//no matter rehash the fingerprint or not 
			if (hash_entry.C<HK[k][Hsh][1].C)
				HK[k][Hsh][1].C = hash_entry.C;//replace entry2 with min count
				return;	
		}
		node tmp;
		tmp.FP = HK[k][Hsh][1].FP;
		tmp.C = HK[k][Hsh][1].C;
		HK[k][Hsh][1].FP = hash_entry.FP;
		HK[k][Hsh][1].C = hash_entry.C;
		hash_entry.FP = tmp.FP;
		hash_entry.C = tmp.C;
		loop_times--;
		//printf("the loop_times = %6d\n", loop_times);
		rehash(hash_entry, loop_times, k, re_hash);
	}

	// 辅助函数，用于交换数组中两个元素的位置
	template <typename T>
	void swap(T arr[], int i, int j) {
		T temp = arr[i];
		arr[i] = arr[j];
		arr[j] = temp;
	}

	// 快速排序的分区函数，返回分区点的索引
	template <typename T1, typename T2>
	int partition(T1 key[], T2 value[], int low, int high) {
		T2 pivot = value[high]; // 将最后一个元素作为基准值
		int i = low - 1; // i 指向小于基准值的元素

		for (int j = low; j <= high - 1; j++) {
			// 如果当前元素大于等于基准值，则交换元素位置
			if (value[j] >= pivot) {
				i++;
				swap(key, i, j);
				swap(value, i, j);
			}
		}

		// 将基准值放到正确的位置上
		swap(key, i + 1, high);
		swap(value, i + 1, high);

		return (i + 1); // 返回基准值的索引
	}

	// 快速排序的递归函数
	template <typename T1, typename T2>
	void quickSort(T1 key[], T2 value[], int low, int high) {
		if (low < high) {
			// 对数组进行分区，获取基准值的索引
			int pivotIndex = partition(key, value, low, high);

			// 递归地对基准值左边的子数组进行快速排序
			quickSort(key, value, low, pivotIndex - 1);

			// 递归地对基准值右边的子数组进行快速排序
			quickSort(key, value, pivotIndex + 1, high);
		}
	}

	// 快速排序的入口函数
	template <typename T1, typename T2>
	void quickSort(T1 key[], T2 value[], int size) {
		quickSort(key, value, 0, size - 1);
	}


    int insert_heap(string str,int value){
        char *key = new char[str.length()+1];
		strcpy(key,str.c_str());
		for(int i=0;i<pointer;i++){	
			if(strcmp(items[i], key) == 0){	//已经存在
				count[i]+= value;
				quickSort(items,count,K);
				return 0;
			}
		}
		if(pointer<K-1){	//有空位
			strcpy(items[pointer], key);
			count[pointer] = value;
			pointer++;
			quickSort(items,count,K);
		}else{	//没空位
			if(value>count[pointer]){
				// char *old_item = new char[100];
				// strcpy(old_item,items[pointer]);
				// int old_count = count[pointer];

				strcpy(items[pointer], key);
				count[pointer] = value;
				quickSort(items,count,K);

				// for(int i=0;i<old_count;i++){
				// 	Insert(old_item);
				// }

			}else{
				return -1;
			}
			
		}
		return 0;
	}

	void Insert(string x)
	{
		int maxv = 0;
		int max_loop = 1;
		node temp;
		unsigned long long H1 = Hash(x); int FP = (H1 >> 56);
		unsigned long long H2 = H1^FP;//Hash(std::to_string(FP));//XOR CUCKOO HASHING
	//	unsigned long long H2 = H1^Hash(std::to_string(FP)); //XOR CUCKOO HASHING
		int Hsh1 = H1 % (M2 - (2 * CC_d) + 2 * 0 + 3);
		int Hsh2 = H2 % (M2 - (2 * CC_d) + 2 * 1 + 3);
		int count = 0;
		
		int hash[2] = { Hsh1, Hsh2 };
		unsigned long long hashHH[2] = { H1, H2 };
		
		int ii, jj, mi=(1<<25);	
		for(int i = 0; i < CC_d; i++)
			for (int j = 0; j < BN; j++)
			{	
				if (mi > HK[i][hash[i]][j].C){
					ii=i; jj=j; mi=HK[i][hash[i]][j].C;
				}	
				if (HK[i][hash[i]][j].FP == FP) {
					HK[i][hash[i]][j].C++;
					maxv = max(maxv, HK[i][hash[i]][j].C);
                    if(maxv>threshold){
                        if(insert_heap(x,maxv/threshold)!=-1){
                        HK[i][hash[i]][j].FP = 0; 
					    HK[i][hash[i]][j].C = 0;
                        }
                    }
					count = 1;
					break;
				}
				if(HK[i][hash[i]][j].FP == 0)
				{
					HK[i][hash[i]][j].FP=FP;
					HK[i][hash[i]][j].C=1;
					maxv=max(maxv,1);
					count = 1;
					break;
				}
			}

		if (count == 0) {	//mean can not insert normally
			HK[ii][hash[ii]][jj].FP = FP;
			HK[ii][hash[ii]][jj].C = 1;
			maxv=max(maxv, 1);
		//	rehash(temp, max_loop, ii, hashHH[ii]);
		}
	}
	
	pair<string, int> Query(int k)
	{
		string str;
		str.assign(items[k]);
		return make_pair(str,count[k]*threshold);
	}
};
#endif
