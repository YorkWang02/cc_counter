#ifndef _cuckoocounter3_H
#define _cuckoocounter3_H
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <unordered_map>
#include "BaseSketch.h"
#include "BOBHASH32.h"
#include "params.h"
#include "BOBHASH64.h"
#define CC_d 2 
#define BN 5 
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cuckoocounter3 : public sketch::BaseSketch{
private:
	struct node { int C, FP; string ID; } HK[CC_d][MAX_MEM + 10][BN]; 
	BOBHash64 * bobhash;
	int K, M2;
public:
	cuckoocounter3(int M2, int K) :M2(M2), K(K)
	{ 
		bobhash = new BOBHash64(1005); 
	}
	void clear()
	{
		for (int i = 0; i < CC_d; i++){
			for (int j = 0; j <= M2 + 5; j++){
				for(int r=0;r<BN;r++){
					HK[i][j][r].C = HK[i][j][r].FP = 0;
					HK[i][j][r].ID = "\0";
				}
			}
				
		}		
	}
	unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
	void rehash(node hash_entry, int loop_times, int i, unsigned long long hash,int j) {
		int k=1-i;
		unsigned long long re_hash = hash ^ Hash(std::to_string(hash_entry.FP)); //XOR CUCKOO HASHING
		int Hsh = re_hash % (M2 - (2 * CC_d) + 2 *(1-i) + 3);
		if(j == BN-1){	//被踢出的是entry5
			if(HK[k][Hsh][BN-1].FP == 0){
				HK[k][Hsh][BN-1] = hash_entry;
				return;
			}
		}else{
			for (int r = 0; r < BN; r++) { //寻找空闲位置
				if (HK[k][Hsh][r].FP == 0) {
					HK[k][Hsh][r] = hash_entry;
					bucket_sort(r,k,Hsh);
					return;
				}
			}
		}
		if (loop_times == 0) {
			if(j == BN-1){
				HK[k][Hsh][BN-1] = hash_entry;
				return;
			}else{
				HK[k][Hsh][1].FP = hash_entry.FP;//no matter rehash the fingerprint or not 
				HK[k][Hsh][1].ID = hash_entry.ID;
				if (hash_entry.C<HK[k][Hsh][1].C)
					HK[k][Hsh][1].C = hash_entry.C;//replace entry2 with min count
				bucket_sort(1,k,Hsh);
				return;	
			}
		}
		loop_times--;
		if(j == BN-1){
			std::swap(hash_entry,HK[k][Hsh][BN-1]);
			rehash(hash_entry, loop_times, k, re_hash,BN-1);
		}else{
			std::swap(hash_entry,HK[k][Hsh][1]);
			bucket_sort(1,k,Hsh);
			rehash(hash_entry, loop_times, k, re_hash,1);
		}
	}

	void bucket_sort(int entry_index, int i, int j) {
    	while (entry_index < BN-2 && HK[i][j][entry_index].C > HK[i][j][entry_index + 1].C) {
        	std::swap(HK[i][j][entry_index], HK[i][j][entry_index + 1]);
        	entry_index++;
    	}
	}


	void Insert(const string &x)
	{
		int max_loop = 1;
		node temp;
		unsigned long long H1 = Hash(x); int FP = (H1 >> 56);
		unsigned long long H2 = H1^FP;//XOR CUCKOO HASHING	
		int Hsh1 = H1 % (M2 - (2 * CC_d) + 2 * 0 + 3);
		int Hsh2 = H2 % (M2 - (2 * CC_d) + 2 * 1 + 3);				
		int hash[2] = { Hsh1, Hsh2 };
		unsigned long long hashHH[2] = { H1, H2 };
		int count = 0;
		int ii, jj, mi=(1<<25);
		for(int i = 0; i < CC_d; i++)
			for (int j = 0; j < BN; j++) //寻找空闲位置或者计数器最小的位置
			{
				// if (mi > HK[i][hash[i]][j].C){
				// 	ii=i; jj=j; mi=HK[i][hash[i]][j].C;
				// }
				if (HK[i][hash[i]][j].FP == FP) {
					HK[i][hash[i]][j].C++; //如果找到相同的指纹，就增加计数器
					count = 1;
					if(HK[i][hash[i]][j].C > HK[i][hash[i]][BN-1].C*2 && j < BN-1){	//如果计数器超过了阈值，就认为可能是top-k流
						temp =  HK[i][hash[i]][BN-1];
						HK[i][hash[i]][BN-1] = HK[i][hash[i]][j];
						HK[i][hash[i]][j].C = HK[i][hash[i]][j].FP = 0;
						HK[i][hash[i]][j].ID = '\0';
						if(temp.C != 0){
							rehash(temp, max_loop, i, hashHH[i],BN-1);
						}
					}
					bucket_sort(j,i,hash[i]);
					break;
					
				}
				if(HK[i][hash[i]][j].FP == 0)
				{
					HK[i][hash[i]][j].ID=x;
					HK[i][hash[i]][j].FP=FP; //如果找到空闲的位置，就插入指纹和计数器为1
					HK[i][hash[i]][j].C=1;
					count = 1;
					break;
				}
			}
		if (count == 0) { //如果没有找到空闲的位置或者相同的指纹，就替换掉计数器最小的位置，并重哈希
			// HK[ii][hash[ii]][jj].ID = x;
			// HK[ii][hash[ii]][jj].FP = FP;
			// HK[ii][hash[ii]][jj].C = 1;
			// rehash(HK[0][hash[0]][0], max_loop, 0, hashHH[0],0);
			HK[0][hash[0]][0].FP = FP;
			HK[0][hash[0]][0].C = 1;
			HK[0][hash[0]][0].ID = x;
		}
		
	}
	struct Node { string x; int y; int thre;} q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }

	void work()
	{
		int CNT = 0;
		for (int i = 0; i < CC_d; i++) {
			for (int j = 0;j < M2+5;j++){
				q[i*(M2+5)+j].x = HK[i][j][4].ID; 
				q[i*(M2+5)+j].y = HK[i][j][4].C;
				CNT++;
			}
		}
		sort(q, q + CNT, cmp);
	}
	pair<string, int> Query(int k)
	{
		if(k<CC_d*M2){
			return make_pair(q[k].x, q[k].y);
		}else{
			return make_pair(q[CC_d*M2-1].x, q[CC_d*M2-1].y);
		}
	}
	std::string get_name() override {
		return "CuckooCounter3";
	}
};
#endif