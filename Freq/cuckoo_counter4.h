#ifndef _CCCOUNTER4_H
#define _CCCOUNTER4_H

#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include "BOBHash.h"

using namespace std;
class CCCounter4
{
private:
	struct bucket_t {	//one bucket
		int flag; //flag for checking if merge happened
		char fingerprint[4];	//four fingerprints  
		uint8_t count12;		// 
		uint32_t count3;			//	four entries' counter
		uint32_t count4;		//
	};
	uint bucket_num, maxloop, h1, h2;	//bucket_num indicates the number of buckets in each array
	bucket_t *bucket[2];		//two arrays
	BOBHash * bobhash[2];		//Bob hash function

public:
	CCCounter4(uint _bucket) {
		bucket_num = _bucket;
		for (int i = 0; i < 2; i++) {
			bobhash[i] = new BOBHash(i + 1000);
		}
		for (int i = 0; i < 2; i++) {	//initialize two arrays 
			bucket[i] = new bucket_t[bucket_num];
			memset(bucket[i], 0, sizeof(bucket_t) * bucket_num);
		}
		for(int j=0;j<bucket_num;j++){
			bucket[0][j].flag = 1;
		}
	}
	
	bool overflow_flag0(bucket_t* b, int j) {	//overflow occur in entry_j in the bucket
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

	bool overflow_flag1(bucket_t* b, int j) {	//overflow occur in entry_j in the bucket
		if (j == 2) {				
			if (b->count3 < 0xff) {
				char tmp = b->fingerprint[1];
				b->fingerprint[1] = b->fingerprint[2];
				b->fingerprint[2] = tmp;
				b->count12 = b->count3;
				b->count3 = 0xff;
			}
			else if (b->count4 < 0xff) {
				char tmp = b->fingerprint[1];
				b->fingerprint[1] = b->fingerprint[3];
				b->fingerprint[3] = tmp;
				b->count12 = b->count4;
				b->count4 = 0xff;
			}
			else return false;
		}
		return true;					//when entry3,4 overflows, we just ignore it	
	}
	
	bool plus_flag0(bucket_t* b, int j) {		//try to plus entry_j in the bucket
										//return true if no overflow happens
		if (j == 1) b->count12++;
		else if (j == 2) b->count12 += 0x10;
		else if (j == 3) b->count3++;
		else if (j == 4) b->count4++;
		if (((b->count12&0xf)==0xf)&&(j==1) || ((b->count12&0xf0)==0xf0)&&(j==2) || (b->count3==0xff)&&(j==3)) {	//if overflow happens
			bool res = overflow_flag0(b, j);	//solve the overflow
			if (!res) return false;		//return false when we can't solve
		}
		return true;	
	}

	bool plus_flag1(bucket_t* b, int j) {		//try to plus entry_j in the bucket
										//return true if no overflow happens
		if (j == 2) b->count12++;
        else if (j == 3) b->count3++;
        else if (j == 4) b->count4++;
        if ((b->count12==0xff)&&(j==2)) {    //if overflow happens
            bool res = overflow_flag1(b, j);    //solve the overflow
            if (!res) return false;        //return false when we can't solve
        }
        return true;				
	}

	bool plus(bucket_t* b, int j){
		if(b->flag == 0){
			return plus_flag0(b,j);
		}else{
			return plus_flag1(b,j);
		}
	}
	
	void kickout2flag0(int loop, int i, bucket_t* b, uint8_t count, uint hash, int j,uint rehashh,uint rehash) {
		//loop: the rest time of kickout
		//i && b && j: kickout happens in array[i], bucket_b, entry_j
		//count: the count's number in entry_j
		//hash: the position of the bucket in the array (h_1(e) or h_2(e))
		
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
	

	void kickout2flag1(int loop, int i, bucket_t* b, uint8_t count, uint hash, int j,uint rehashh,uint rehash) {
		//loop: the rest time of kickout
		//i && b && j: kickout happens in array[i], bucket_b, entry_j
		//count: the count's number in entry_j
		//hash: the position of the bucket in the array (h_1(e) or h_2(e))
		
			if (bucket[1-i][rehash].fingerprint[1] == NULL && count < 0xff) {
				bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];
				bucket[1-i][rehash].count12 = count;
			}
			else if (bucket[1-i][rehash].fingerprint[2] == NULL && count < 0xffff) {
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = count;
			}
			else if (bucket[1-i][rehash].fingerprint[3] == NULL && count < 0xffff) {
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = (uint16_t)count;
			}
			else if (loop > 0) {	//if we can't find empty entry
				loop--;
				if (count < 0xff) {
					uint8_t tmpcount = bucket[1-i][rehash].count12;
					kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 2);
					bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];
					bucket[1-i][rehash].count12 = count;
				}
				else {
					uint16_t tmpcount = bucket[1-i][rehash].count4;
					kickout(loop, 1-i, &bucket[1-i][rehash], tmpcount, rehashh, 4);
					bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
					bucket[1-i][rehash].count4 = (uint16_t)count;
				}
			}
			else {	//the last round of kickout
				if (count < 0xff) {
					bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];	//replace entry2
					if (count < bucket[1-i][rehash].count12)
						bucket[1-i][rehash].count12 = count;
				}
				else if (count < 0xffff) {	//replace entry3
					bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
					if (count < bucket[1-i][rehash].count3)
						bucket[1-i][rehash].count3 = count;
				}
			}
		
	}

	void kickout(int loop, int i, bucket_t* b, uint8_t count, uint hash, int j) {
		uint rehashh = (hash ^ (bobhash[0]->run((const char*)&b->fingerprint[j-1], 1)));
		uint rehash = rehashh % bucket_num;	//calculate the other hash number
		if(bucket[1-i][rehash].flag == 0){
			kickout2flag0(loop,i,b,count,hash,j,rehashh,rehash);
		}else{
			kickout2flag1(loop,i,b,count,hash,j,rehashh,rehash);
		}
	}

	void kickoverflow_flag020(int i, bucket_t* b, uint hash, int j,uint rehashh,uint rehash) {	
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

	void kickoverflow_flag121(int i, bucket_t* b, uint hash, int j,uint rehashh,uint rehash) {
		if (j == 2) {		//entry2 overflow
			if (bucket[1-i][rehash].fingerprint[2] == NULL) {
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xff;
			}
			else if (bucket[1-i][rehash].fingerprint[3] == NULL) {
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xff;
			}
			else if (bucket[1-i][rehash].count3 < 0xff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count3, rehashh, 3);
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xff;
			}
			else if (bucket[1-i][rehash].count4 < 0xff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count4, rehashh, 4);
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xff;
			}
			else return;
			b->count12 = 0;
		}
		b->fingerprint[j-1] = NULL;
	}

	void kickoverflow_flag021(int i, bucket_t* b, uint hash, int j,uint rehashh,uint rehash) {	
		if (j == 1 || j == 2) {		//entry1 overflow
			if (bucket[1-i][rehash].fingerprint[1] == NULL) {
				bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];
				bucket[1-i][rehash].count12 = 0xf;
			}
			else if (bucket[1-i][rehash].fingerprint[2] == NULL) {
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xf;
			}
			else if (bucket[1-i][rehash].fingerprint[3] == NULL) {
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xf;
			}
			else if (bucket[1-i][rehash].count12 < 0xf) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count12, rehashh, 2);
				bucket[1-i][rehash].fingerprint[1] = b->fingerprint[j-1];
				bucket[1-i][rehash].count12 = 0xf;
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
			if (bucket[1-i][rehash].fingerprint[2] == NULL) {
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xff;
			}
			else if (bucket[1-i][rehash].fingerprint[3] == NULL) {
				bucket[1-i][rehash].fingerprint[3] = b->fingerprint[j-1];
				bucket[1-i][rehash].count4 = 0xff;
			}
			else if (bucket[1-i][rehash].count3 < 0xff) {
				kickout(maxloop, 1-i, &bucket[1-i][rehash], bucket[1-i][rehash].count3, rehashh, 3);
				bucket[1-i][rehash].fingerprint[2] = b->fingerprint[j-1];
				bucket[1-i][rehash].count3 = 0xff;
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

	void kickoverflow_flag120(int i, bucket_t* b, uint hash, int j,uint rehashh,uint rehash) {
		if (j == 2) {		//entry2 overflow
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
			b->count12 = 0;
		}
		b->fingerprint[j-1] = NULL;
	}	

	void kickoverflow(int i, bucket_t* b, uint hash, int j) {
		//i && b && j: overflow happens in {array[i], bucket_b, entry_j}, and need kickout
		//hash: the position of the bucket in the array (h_1(e) or h_2(e))
		//when overflow can't be solved within the scope of its bucket
		uint rehashh = (hash ^ (bobhash[0]->run((const char*)&b->fingerprint[j-1], 1)));
		uint rehash = rehashh % bucket_num;

		if(b->flag == 0 && bucket[1-i][rehash].flag == 0){
			kickoverflow_flag020(i, b, hash, j, rehashh, rehash);
		}else if(b->flag == 1 && bucket[1-i][rehash].flag == 1){
			kickoverflow_flag121(i, b, hash, j, rehashh, rehash);
		}else if(b->flag == 1 && bucket[1-i][rehash].flag == 0){
			kickoverflow_flag120(i, b, hash, j, rehashh, rehash);
		}else{
			kickoverflow_flag021(i, b, hash, j, rehashh, rehash);
		}
	}

	
	void Insert(const char *key) {
		maxloop = 1;		
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))); //% bucket_num;		
		uint hash[2] = {h1%bucket_num, h2%bucket_num};
		uint hashh[2] = {h1, h2};
		
		int ii, jj, flag = 0;
		for (int i=0; i<2; i++) {
			if(bucket[i][hash[i]].flag == 0){
				for(int j=1; j<=4; j++) {
					if (bucket[i][hash[i]].fingerprint[j-1] == fp) {
						bool res = plus(&bucket[i][hash[i]], j);
						if (!res){
							if(j==1 || j==2){	
								bucket[i][hash[i]].fingerprint[1] = fp;
								bucket[i][hash[i]].count12 = 0xf0;
								bucket[i][hash[i]].fingerprint[0] = NULL;
								bucket[i][hash[i]].flag = 1;
							}else{	
								kickoverflow(i, &bucket[i][hash[i]], hashh[i], j);
							}	
						} 
						return;
					}
					else if (!flag && bucket[i][hash[i]].fingerprint[j-1] == NULL) {
						ii = i;
						jj = j;
						flag = 1;
					}
				}
			}else{
				for(int j=2; j<=4; j++) {
					if (bucket[i][hash[i]].fingerprint[j-1] == fp) {
						bool res = plus(&bucket[i][hash[i]], j);
						if (!res){
							kickoverflow(i, &bucket[i][hash[i]], hashh[i], j);
						} 
						return;
					}
					else if (!flag && bucket[i][hash[i]].fingerprint[j-1] == NULL) {
						ii = i;
						jj = j;
						flag = 1;
					}
				}
			}
		}	
		if (flag) {
			bucket[ii][hash[ii]].fingerprint[jj-1] = fp;
			plus(&bucket[ii][hash[ii]], jj);
			return;
		}
		int i = fp&0x1;
		if(bucket[i][hash[i]].flag == 0){
			//uint8_t count = bucket[i][hash[i]].count12 & 0xf;
			// kickout(--maxloop, i, &bucket[i][hash[i]], count, hashh[i], 1);
			bucket[i][hash[i]].fingerprint[0] = fp;
			bucket[i][hash[i]].count12 = (bucket[i][hash[i]].count12&0xf0) | 0x1;	
		}else{
			bucket[i][hash[i]].fingerprint[1] = fp;
			bucket[i][hash[i]].count12 = 1;	
		}	
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
	double query_flag0(bucket_t* b, int j) {
		if (j == 1) return (b->count12&0xf);
		else if (j == 2) return ((b->count12&0xf0) >> 4);
		else if (j == 3) return b->count3;
		else if (j == 4) return b->count4;
	}

	double query_flag1(bucket_t* b, int j) {
		if (j == 1) return (b->count12&0xf);
		else if (j == 2) return b->count12;
		else if (j == 3) return b->count3;
		else if (j == 4) return b->count4;
	}
	
	double query(bucket_t* b, int j) {
		if(b->flag == 0){
			return query_flag0(b,j);
		}else{
			return query_flag1(b,j);
		}
	}

	double Query(const char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24))
		h1 = (bobhash[0]->run(key, strlen(key))); //% bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1))); //% bucket_num;
		uint hash[2] = {h1%bucket_num, h2%bucket_num};
		bool flag=0;
		for (int i = 0; i < 2; i++) {
			if(bucket[i][hash[i]].flag == 0){
				for (int j = 1; j <= 4; j++) {
					if (bucket[i][hash[i]].fingerprint[j-1] == fp)
						return query(&bucket[i][hash[i]], j);
					if (!flag && bucket[i][hash[i]].fingerprint[j-1] == NULL)
						flag=1;
				}
			}else{
				for (int j = 2; j <= 4; j++) {
					if (bucket[i][hash[i]].fingerprint[j-1] == fp)
						return query(&bucket[i][hash[i]], j);
					if (!flag && bucket[i][hash[i]].fingerprint[j-1] == NULL)
						flag=1;
				}
			}
			
		}
		if (flag) return 0;
		int min1;
		int min2;
		if(bucket[0][hash[0]].flag == 0){
			min1 = min(bucket[0][hash[0]].count12&0xf, (bucket[0][hash[0]].count12&0xf0)>>4);
		}else{
			min1 = bucket[0][hash[0]].count12;
		}
		if(bucket[1][hash[1]].flag == 0){
            min2 = min(bucket[1][hash[1]].count12&0xf, (bucket[1][hash[1]].count12&0xf0)>>4);
        }else{
            min2 = bucket[1][hash[1]].count12;
        }
		return min(min1, min2);
	}
	
	//memeory access
	int Mem(const char *key) {
		//char fp = (char)((int)*key ^ ((int)*key >> 8) ^ ((int)*key >> 16) ^ ((int)*key >> 24));
		h1 = (bobhash[0]->run(key, strlen(key)));// % bucket_num;
		char fp = (char)(h1 ^ (h1 >> 8) ^ (h1 >> 16) ^ (h1 >> 24));
		h2 = (h1 ^ (bobhash[0]->run((const char*)&fp, 1)));// % bucket_num;
		int hash[2] = {h1%bucket_num, h2%bucket_num};
		for (int i = 0; i < 2; i++) {
			if(bucket[i][hash[i]].flag == 0){
				for (int j = 0; j < 4; j++) {
					if (bucket[i][hash[i]].fingerprint[j] == fp)
						return 1;
				}
			}else{
				for (int j = 1; j < 4; j++) {
					if (bucket[i][hash[i]].fingerprint[j] == fp)
						return 1;
				}
			}
		}
		return 2;
	}

	// the use ratio of the bucket
	double Ratio() {
		int used_num = 0;
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < bucket_num; j++) {
				if(bucket[i][j].flag == 0){
					if ((bucket[i][j].count12&0xf) != 0) used_num++;
					if ((bucket[i][j].count12&0xf0 != 0)) used_num++;
					if (bucket[i][j].count3 != 0) used_num++;
					if (bucket[i][j].count4 != 0) used_num++;
				}else{
					if ((bucket[i][j].count12 != 0)) used_num++;
					if (bucket[i][j].count3 != 0) used_num++;
					if (bucket[i][j].count4 != 0) used_num++;
				}
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

	~CCCounter3() {
		for (int i = 0; i < 2; i++) {
			delete[]bucket[i];
		}
		for (int i = 0; i < 1; i++) {
			delete bobhash[i];
		}
	}
};
#endif//_CCCOUNTER4_H
