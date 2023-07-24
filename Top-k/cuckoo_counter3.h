#ifndef _CCCOUNTER3_H
#define _CCCOUNTER3_H

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include "BOBHASH64.h"
#include "params.h"

using namespace std;
class CCCounter3
{
private:

	struct bucket_t {	//one bucket
		char fingerprint[4];	//four fingerprints  
		char *str;
		uint8_t count12;		// 
		uint8_t count3;			//	four entries' counter
		uint32_t count4;		//
		uint32_t count5;
	
	};
	uint bucket_num, maxloop, h1, h2;	//bucket_num indicates the number of buckets in each array
    uint32_t threshold;		//阈值一：热流阈值
	double threshold2;		//阈值二：比例系数
	uint32_t threshold3;	//阈值三：评估entry5的大小
	bucket_t *bucket[2];		//two arrays
	BOBHash64 * bobhash[2];		//Bob hash function

public:
	CCCounter3(uint _bucket,int value) {
		bucket_num = _bucket;
		threshold = value;
		threshold2 = 0.2;
		threshold3 = 1000;
		for (int i = 0; i < 2; i++) {
			bobhash[i] = new BOBHash64(i + 1000);
		}
		for (int i = 0; i < 2; i++) {	//initialize two arrays 
			bucket[i] = new bucket_t[bucket_num];
			memset(bucket[i], 0, sizeof(bucket_t) * bucket_num);
			for(int j=0;j<bucket_num;j++){
				bucket[i][j].str = new char[100];
				bucket[i][j].str[0] = '\0';
			}
		}
	
	}
	
	bool overflow(bucket_t* b, int j,const char *key) {	//overflow occur in entry_j in the bucket
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
			else if (b->count5 < 0xf) {
				h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
				char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
				b->fingerprint[0] = fp;
				strcpy(b->str,key);
				b->count12 = b->count12&0xf0 | (uint8_t)b->count5;
				b->count5 = 0xf;
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
			else if (b->count5 < 0xf) {
				h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
				char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
				b->fingerprint[1] = fp;
				strcpy(b->str,key);
				b->count12 = b->count12&0xf | (uint8_t)(b->count5<<4);
				b->count5 = 0xf;
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
			else if (b->count5 < 0xff) {
				h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
				char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
				b->fingerprint[2] = fp;
				strcpy(b->str,key);
				b->count3 = (uint8_t)b->count5;
				b->count5 = 0xff;
			}
			else return false;
		}
		else if (j == 4) {				//let entry4 try to swap with entry5
			if (b->count5 < 0xffff) {
				h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
				char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
				b->fingerprint[3] = fp;
				strcpy(b->str,key);
				b->count4 = (uint16_t)b->count5;
				b->count5 = 0xffff;
			}
			else return false;
		}
		return true;					//when entry5 overflows, we just ignore it
	}
	
	
	bool plus(bucket_t* b, int j,const char *key) {		//try to plus entry_j in the bucket
										//return true if no overflow happens
		if (j == 1){
            b->count12++;
            if((b->count12&0xf)==0xf){
                bool res = overflow(b, j,key);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
        }
		else if (j == 2){
            b->count12 += 0x10;
            if((b->count12&0xf0)==0xf0){
                bool res = overflow(b, j,key);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
        }
		else if (j == 3){
            b->count3++;
            if(b->count3==0xff){
                bool res = overflow(b, j,key);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
        }
		else if (j == 4){
            b->count4++;
			if(b->count4==0xffff){
                bool res = overflow(b, j,key);	//solve the overflow
			    if (!res) return false;		//return false when we can't solve
            }
			if(b->count4>threshold){	//entry4大于阈值1,判定为热流
				if(b->count5<threshold3){	//情形1：entry5较小，此时比例系数为1，直接比较是否替换
					if(b->count4>b->count5){	//情形1.1：entry4大于entry5,替换。反之不替换
						h1 = (bobhash[0]->run(b->str, strlen(b->str))); //% bucket_num;
						char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
						b->fingerprint[3] = fp;
						strcpy(b->str,key);
						int tmp = b->count4;
						b->count4 = (uint16_t)b->count5;
						b->count5 = tmp;
					}	
				}else{	//情形2：entry5较大，此时entry4较难超过entry5,更改比例系数来提升entry5使用率
					if(b->count4>b->count5*threshold2){
						h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
						char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
						h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))); //% bucket_num;		
						uint hash[2] = {h1%bucket_num, h2%bucket_num};
						uint hashh[2] = {h1, h2};
						for(int i=0;i<2;i++){
							if(bucket[i][hash[i]].fingerprint[3]==fp){	//找到原桶·
								if(b->count4>bucket[1-i][hash[1-i]].count5){
									int h = (bobhash[0]->run(bucket[1-i][hash[1-i]].str, strlen(bucket[1-i][hash[1-i]].str))); //% bucket_num;
									char fp = (char)(h ^ (h >> 8) ^ (h >> 16) ^ (h >> 24));
									uint32_t tmp = bucket[1-i][hash[1-i]].count5;
									strcpy(bucket[1-i][hash[1-i]].str,key);
									bucket[1-i][hash[1-i]].count5 = b->count4;
									b->fingerprint[3] = NULL;
									b->count4 = 0;
									bucket[1-i][hash[1-i]].count4 = (uint16_t)tmp;
									bucket[1-i][hash[1-i]].fingerprint[3] = fp;
								}
							}
						}
					}
				}
			}	
        }
		else if (j == 5){
            b->count5++;
        }
		return true;
	}
	
	void kickout(int loop, int i, bucket_t* b, uint8_t count, uint hash, int j,const char *key) {
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
		else if (bucket[1-i][rehash].str == NULL && count < 0xffffffff) {
			strcpy(bucket[1-i][rehash].str,key);
			bucket[1-i][rehash].count5 = (uint32_t)count;
		}
		else if (loop > 0) {	//if we can't find empty entry
			loop--;
			if (count < 0xf) {
				uint8_t tmpcount = bucket[1-i][rehash].count12 & 0xf;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 1,key);	//do kickout
				bucket[1-i][rehash].fingerprint[0] = b->fingerprint[j-1];
				bucket[1-i][rehash].count12 = (bucket[1-i][rehash].count12&0xf0) | count;					
			}
			else if (count < 0xff) {
				uint8_t tmpcount = bucket[1-i][rehash].count3;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 3,key);
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = count;
			}
			else if (count < 0xffff){
				uint16_t tmpcount = bucket[1-i][rehash].count4;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 4,key);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = (uint16_t)count;
			}
			else{
				uint32_t tmpcount = bucket[1-i][rehash].count5;
				kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 5,key);
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = (uint32_t)count;
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
			else if (count < 0xffff) {	//replace entry4
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				if (count < bucket[1-i][rehash].count4)
					bucket[1-i][rehash].count4 = count;
			}
		}
	}
	
	void kickoverflow(int i, bucket_t* b, uint hash, int j,const char *key) {
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
			else if (bucket[1-i][rehash].str == NULL) {
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = 0xf;
			}
			else if (bucket[1-i][rehash].count3 < 0xf) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count3, rehashh, 3,key);
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xf;
			}
			else if (bucket[1-i][rehash].count4 < 0xf) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count4, rehashh, 4,key);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xf;
			}
			else if (bucket[1-i][rehash].count5 < 0xf) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count5, rehashh, 5,key);
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = 0xf;
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
			else if (bucket[1-i][rehash].str == NULL) {
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = 0xff;
			}
			else if (bucket[1-i][rehash].count4 < 0xff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count4, rehashh, 4,key);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xff;
			}
			else if (bucket[1-i][rehash].count5 < 0xff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count5, rehashh, 5,key);
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = 0xff;
			}
			else return;
			b->count3 = 0;
		}
		else if (j == 4) {			//entry4 overflow
			if (bucket[1-i][rehash].str == NULL) {
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = 0xffff;
			}
			else if (bucket[1-i][rehash].count5 < 0xffff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count5, rehashh, 5,key);
				strcpy(bucket[1-i][rehash].str,key);
				bucket[1-i][rehash].count5 = 0xffff;
			}
			else return;
			b->count4 = 0;
		}
		b->fingerprint[j-1] = NULL;
	}

	
	void Insert(string str) {
		// for(int i=0;i<2;i++){
		// 	for(int j=0;j<bucket_num;j++){
		// 		if(strcmp(bucket[i][j].str,key)==0){
		// 			bucket[i][j].count5++;
		// 			return;
		// 		}
		// 	}
		// }
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
				if (bucket[i][hash[i]].fingerprint[j-1] == fp ) {
					bool res = plus(&bucket[i][hash[i]], j,key);
					if (!res) kickoverflow(i, &bucket[i][hash[i]], hashh[i], j,key);
					return;
				}
				else if (!flag && bucket[i][hash[i]].fingerprint[j-1] == NULL) {
					ii = i;
					jj = j;
					flag = 1;
				}
			}
		}	
		for (int i=0; i<2; i++) {
			if (strcmp(bucket[i][hash[i]].str,key)==0 ) {
				bool res = plus(&bucket[i][hash[i]], 5,key);
				if (!res) kickoverflow(i, &bucket[i][hash[i]], hashh[i], 5,key);
				return;
			}
			else if (!flag && bucket[i][hash[i]].str == NULL) {
				ii = i;
				jj = 5;
				flag = 1;
			}
		}	
		
		if (flag) {
			if(jj!=5){
				bucket[ii][hash[ii]].fingerprint[jj-1] = fp;
				plus(&bucket[ii][hash[ii]], jj,key);
				return;
			}else{
				strcpy(bucket[ii][hash[ii]].str,key);
				plus(&bucket[ii][hash[ii]], jj,key);
				return;
			}
			
		}
		int i = fp&0x1;
		uint8_t count = bucket[i][hash[i]].count12 & 0xf;
	//	kickout(--maxloop, i, &bucket[i][hash[i]], count, hashh[i], 1);
		bucket[i][hash[i]].fingerprint[0] = fp;
		bucket[i][hash[i]].count12 = (bucket[i][hash[i]].count12&0xf0) | 0x1;
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

    struct Node { string x; int y;} q[MAX_MEM + 10];
    static int cmp(Node i, Node j) { return i.y > j.y; }
    
    void work()
	{
        int cnt = 0;
		for (int i = 0; i<2; i++){
            for(int j=0;j<bucket_num;j++){
                cnt++;
                q[i*bucket_num+j].y = bucket[i][j].count5;
                q[i*bucket_num+j].x = bucket[i][j].str;     
            }
        }
        sort(q,q+cnt,cmp);
	}

	
    pair<string, int> Query(int k)
	{
        if(k<2*bucket_num){
            return make_pair(q[k].x,q[k].y);
        }else{
            return make_pair(q[2*bucket_num-1].x,q[2*bucket_num-1].y);
        }
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
		for(int i=0;i<2;i++){
			if (bucket[i][hash[i]].str == key)
					return 1;
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
				if (bucket[i][j].count5 != 0) used_num++;
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
		else if (j == 5) {
			b->count5--;
			if (b->count5 == 0)
				b->str = NULL;
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
		for(int i=0;i<2;i++){
			if (bucket[i][hash[i]].str == key) {
				minus(&bucket[i][hash[i]], 5);
				return;					
			}
		}
	}

	~CCCounter3() {
		for (int i = 0; i < 2; i++) {
			delete[]bucket[i];
		}
		for (int i = 0; i < 1; i++) {
			delete bobhash[i];
		}
	}
};
#endif//_CCCOUNTER3_H