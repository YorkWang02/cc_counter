#ifndef _CCCOUNTER2_H
#define _CCCOUNTER2_H

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include "BOBHASH64.h"
#include "params.h"

using namespace std;
class CCCounter2
{
private:
    int *count;
	char **items;	//过滤器中的条目名
    int pointer;    //指示过滤器中的空闲位置

	struct bucket_t {	//one bucket
		char fingerprint[4];	//four fingerprints  
		uint8_t count12;		// 
		uint8_t count3;			//	four entries' counter
		uint32_t count4;		//
	};
	uint bucket_num, maxloop, h1, h2;	//bucket_num indicates the number of buckets in each array
    uint32_t threshold;
	int K;
	bucket_t *bucket[2];		//two arrays
	BOBHash64 * bobhash[2];		//Bob hash function

public:
	CCCounter2(uint _bucket,int heap_size,uint32_t value) {
		bucket_num = _bucket;
        threshold = value;
		K = heap_size;
		for (int i = 0; i < 2; i++) {
			bobhash[i] = new BOBHash64(i + 1000);
		}
		for (int i = 0; i < 2; i++) {	//initialize two arrays 
			bucket[i] = new bucket_t[bucket_num];
			memset(bucket[i], 0, sizeof(bucket_t) * bucket_num);
		}
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
	}
	
	bool overflow(bucket_t* b, int j) {	//overflow occur in entry_j in the bucket
		if (j == 1) {					//let entry1 try to swap with entry3 
			if (b->count3 < 0xf) {
				char tmp = b->fingerprint[0];
				b->fingerprint[0] = b->fingerprint[2];
				b->fingerprint[2] = tmp;
				b->count12 = b->count12&0xf0 | b->count3;
				b->count3 = 0xf;
			}
			else if (b->count4 < 0xf) {
				char tmp = b->fingerprint[0];
				b->fingerprint[0] = b->fingerprint[3];
				b->fingerprint[3] = tmp;
				b->count12 = b->count12&0xf0 | (uint8_t)b->count4;
				b->count4 = 0xf;
			}
			else return false;
		}
		else if (j == 2) {				//same as above
			if (b->count3 < 0xf) {
				char tmp = b->fingerprint[1];
				b->fingerprint[1] = b->fingerprint[2];
				b->fingerprint[2] = tmp;
				b->count12 = b->count12&0xf | (b->count3<<4);
				b->count3 = 0xf;
			}
			else if (b->count4 < 0xf) {
				char tmp = b->fingerprint[1];
				b->fingerprint[1] = b->fingerprint[3];
				b->fingerprint[3] = tmp;
				b->count12 = b->count12&0xf | (uint8_t)(b->count4<<4);
				b->count4 = 0xf;
			}
			else return false;
		}
		else if (j == 3) {				//let entry3 try to swap with entry4
			if (b->count4 < 0xff) {
				char tmp = b->fingerprint[2];
				b->fingerprint[2] = b->fingerprint[3];
				b->fingerprint[3] = tmp;
				b->count3 = (uint8_t)b->count4;
				b->count4 = 0xff;
			}
			else return false;
		}
		return true;					//when entry4 overflows, we just ignore it
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


	int insert_heap(const char *key,int value){
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

	
	bool plus(bucket_t* b, int j,const char *key) {		//try to plus entry_j in the bucket
										//return true if no overflow happens
		if (j == 1){
            b->count12++;
            if((b->count12&0xf)==0xf){
                bool res = overflow(b, j);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
        }
		else if (j == 2){
            b->count12 += 0x10;
            if((b->count12&0xf0)==0xf0){
                bool res = overflow(b, j);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
        }
		else if (j == 3){
            b->count3++;
            if(b->count3==0xff){
                bool res = overflow(b, j);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
        }
		else if (j == 4){
            int tmp = ++b->count4;
            if(tmp>=threshold){
                if(insert_heap(key,tmp/threshold)!=-1){
                    b->fingerprint[j-1] = NULL; 
					b->count4 = 0;
                }
            }
        }
		return true;
	}
	
	void kickout(int loop, int i, bucket_t* b, uint8_t count, uint hash, int j) {
		//loop: the rest time of kickout
		//i && b && j: kickout happens in array[i], bucket_b, entry_j
		//count: the count's number in entry_j
		//hash: the position of the bucket in the array (h_1(e) or h_2(e))
		uint rehashh = (hash ^ (bobhash[0]->run((const char*)&b->fingerprint[j-1], 1)));
		uint rehash = rehashh % bucket_num;	//calculate the other hash number
		
		if (bucket[1-i][rehash].fingerprint[0] == NULL && count < 0xf) {	//search for an empty entry
			bucket[1-i][rehash].fingerprint[0] = b->fingerprint[j-1];
			bucket[1-i][rehash].count12 = (bucket[1-i][rehash].count12 & 0xf0) | count;
		}
		else if (bucket[1-i][rehash].fingerprint[1] == NULL && count < 0xf) {
			bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];
			bucket[1-i][rehash].count12 = (bucket[1-i][rehash].count12 & 0xf) | (count << 4);
		}
		else if (bucket[1-i][rehash].fingerprint[2] == NULL && count < 0xff) {
			bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
			bucket[1-i][rehash].count3 = count;
		}
		else if (bucket[1-i][rehash].fingerprint[3] == NULL && count < 0xffff) {
			bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
			bucket[1-i][rehash].count4 = (uint16_t)count;
		}
		else if (loop > 0) {	//if we can't find empty entry
			loop--;
			if (count < 0xf) {
				uint8_t tmpcount = bucket[1-i][rehash].count12 & 0xf;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 1);	//do kickout
				bucket[1-i][rehash].fingerprint[0] = b->fingerprint[j-1];
				bucket[1-i][rehash].count12 = (bucket[1-i][rehash].count12&0xf0) | count;					
			}
			else if (count < 0xff) {
				uint8_t tmpcount = bucket[1-i][rehash].count3;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 3);
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = count;
			}
			else {
				uint16_t tmpcount = bucket[1-i][rehash].count4;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 4);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = (uint16_t)count;
			}
		}
		else {	//the last round of kickout
			if (count < 0xf) {
				bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];	//replace entry2
				if (count < ((bucket[1-i][rehash].count12&0xf0)>>4))
					bucket[1-i][rehash].count12 = (bucket[1-i][rehash].count12&0xf) | (count<<4);
			}
			else if (count < 0xff) {	//replace entry3
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				if (count < bucket[1-i][rehash].count3)
					bucket[1-i][rehash].count3 = count;
			}
		}
	}
	
	void kickoverflow(int i, bucket_t* b, uint hash, int j) {
		//i && b && j: overflow happens in {array[i], bucket_b, entry_j}, and need kickout
		//hash: the position of the bucket in the array (h_1(e) or h_2(e))
		//when overflow can't be solved within the scope of its bucket
		uint rehashh = (hash ^ (bobhash[0]->run((const char*)&b->fingerprint[j-1], 1)));
		uint rehash = rehashh % bucket_num;
		
		if (j == 1 || j == 2) {		//entry1 overflow
			if (bucket[1-i][rehash].fingerprint[2] == NULL) {
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xf;
			}
			else if (bucket[1-i][rehash].fingerprint[3] == NULL) {
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xf;
			}
			else if (bucket[1-i][rehash].count3 < 0xf) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count3, rehashh, 3);
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xf;
			}
			else if (bucket[1-i][rehash].count4 < 0xf) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count4, rehashh, 4);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xf;
			}
			else return;
			if (j == 1) b->count12 &= 0xf0;
			else b->count12 &= 0xf;
		}
		else if (j == 3) {			//entry3 overflow
			if (bucket[1-i][rehash].fingerprint[3] == NULL) {
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xff;
			}
			else if (bucket[1-i][rehash].count4 < 0xff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count4, rehashh, 4);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xff;
			}
			else return;
			b->count3 = 0;
		}
		b->fingerprint[j-1] = NULL;
	}
	
	void Insert(string str) {
		char *key = new char[str.length()+1];
		strcpy(key,str.c_str());
        maxloop = 1;		
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))); //% bucket_num;		
		uint hash[2] = {h1%bucket_num, h2%bucket_num};
		uint hashh[2] = {h1, h2};
		
		int ii, jj, flag = 0;
		for (int i=0; i<2; i++) {
			for(int j=1; j<=4; j++) {
				if (bucket[i][hash[i]].fingerprint[j-1] == fp) {
					bool res = plus(&bucket[i][hash[i]], j,key);
					if (!res) kickoverflow(i, &bucket[i][hash[i]], hashh[i], j);
					return;
				}
				else if (!flag && bucket[i][hash[i]].fingerprint[j-1] == NULL) {
					ii = i;
					jj = j;
					flag = 1;
				}
			}
		}	
		if (flag) {
			bucket[ii][hash[ii]].fingerprint[jj-1] = fp;
			plus(&bucket[ii][hash[ii]], jj,key);
			return;
		}
		int i = fp&0x1;
		uint8_t count = bucket[i][hash[i]].count12 & 0xf;
	//	kickout(--maxloop, i, &bucket[i][hash[i]], count, hashh[i], 1);
		bucket[i][hash[i]].fingerprint[0] = fp;
		bucket[i][hash[i]].count12 = (bucket[i][hash[i]].count12&0xf0) | 0x1;
		delete [] key;
	}
	double zero(){
		double cnt[2]={0.0, 0.0};
		for (int i=0; i<2; i++){
			for (int j=0; j<bucket_num; j++){
				if (bucket[i][j].fingerprint[0]==NULL) cnt[i]+=1;	
			}
		}
		return (bucket_num-cnt[0])/bucket_num * (bucket_num-cnt[1])/bucket_num;
	}	

	pair<string, int> Query(int k)
	{
		string str;
		str.assign(items[k]);
		return make_pair(str,count[k]*threshold);
	}
	
	
	//memeory access
	int Mem(const char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, strlen(key)));// % bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1)));// % bucket_num;
		int hash[2] = {h1%bucket_num, h2%bucket_num};
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 4; j++) {
				if (bucket[i][hash[i]].fingerprint[j] == fp)
					return 1;
			}
		}
		return 2;
	}

	// the use ratio of the bucket
	double Ratio() {
		int used_num = 0;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < bucket_num; j++) {
				if ((bucket[i][j].count12&0xf) != 0) used_num++;
				if ((bucket[i][j].count12&0xf0 != 0)) used_num++;
				if (bucket[i][j].count3 != 0) used_num++;
				if (bucket[i][j].count4 != 0) used_num++;
			}
		}
		return used_num / (bucket_num * 2.0);
	}

	void minus(bucket_t* b, int j) {
		if (j == 1) {
			b->count12--;
			if (b->count12 & 0xf == 0)
				b->fingerprint[0] = NULL;
		}
		else if (j == 2) {
			b->count12 -= 0x10;
			if (b->count12 & 0xf0 == 0)
				b->fingerprint[1] = NULL;
		}
		else if (j == 3) {
			b->count3--;
			if (b->count3 == 0)
				b->fingerprint[2] = NULL;
		}
		else if (j == 4) {
			b->count4--;
			if (b->count4 == 0)
				b->fingerprint[3] = NULL;
		}
	}

	//delete the bucket
	void Delete(char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1)));// % bucket_num;
		uint hash[2] = {h1%bucket_num, h2%bucket_num};
		for (int i = 0; i < 2; i++) {
			for (int j = 1; j <= 4; j++) {
				if (bucket[i][hash[i]].fingerprint[j-1] == fp) {
					minus(&bucket[i][hash[i]], j);
					return;					
				}
			}
		}
	}

	~CCCounter2() {
		for (int i = 0; i < 2; i++) {
			delete[]bucket[i];
		}
		for (int i = 0; i < 1; i++) {
			delete bobhash[i];
		}
	}
};
#endif//_CCCOUNTER2_H