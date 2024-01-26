#ifndef _cuckoocounter31_H
#define _cuckoocounter31_H
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
#define BN 6 
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cuckoocounter31 : public sketch::BaseSketch{
private:
	struct node { int C, FP; string ID; } HK[CC_d][MAX_MEM + 10][BN]; 
	int flag[CC_d][MAX_MEM + 10];
	BOBHash64 * bobhash;
	int K, M2;
public:
	cuckoocounter31(int M2, int K) :M2(M2), K(K)
	{ 
		bobhash = new BOBHash64(1005); 
	}
	void clear()
	{
		for (int i = 0; i < CC_d; i++){
			for (int j = 0; j <= M2 + 5; j++){
				flag[i][j] = 0;
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
	void rehash(node hash_entry, int loop_times, int j, unsigned long long hash) {
		int k=1-j;
		unsigned long long re_hash = hash ^ Hash(std::to_string(hash_entry.FP));
		int Hsh = re_hash % (M2 - (2 * CC_d) + 2 *(1-j) + 3);
		for (int r = 0; r < BN-1; r++) {
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

	void Insert(const string &x)
	{
		int maxv = 0;
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
		int pos_i,pos_j;

		for(int i=0;i<CC_d;i++){	//先遍历两个备用桶的entry5
			if(HK[i][hash[i]][BN-1].ID == x){
				HK[i][hash[i]][BN-1].C++;
				return;
			}
		}

		for(int i = 0; i < CC_d; i++){
			if(flag[i][hash[i]] == 0){	//该桶没有发生融合
				for (int j = 0; j < BN-1; j++){ //寻找空闲位置或者计数器最小的位置
					if (mi > HK[i][hash[i]][j].C){
						ii=i; jj=j; mi=HK[i][hash[i]][j].C;
					}
					if (HK[i][hash[i]][j].FP == FP) {
						pos_i = i;
						pos_j = j;
						HK[i][hash[i]][j].C++; 
						if((j == 0 || j == 1) && HK[i][hash[i]][j].C > 0xf){	//发生溢出
							// HK[i][hash[i]][j].C = 0xf;	//简单截断
							flag[i][hash[i]] = 1;	//改变标志位
							HK[i][hash[i]][1].FP = FP;	//融合后的数据放到entry2
							HK[i][hash[i]][1].C = 0x10; 
						}
						maxv = max(maxv, HK[i][hash[i]][j].C);
						count = 1;
						break;
					}
					if(HK[i][hash[i]][j].FP == 0){
						pos_i = i;
						pos_j = j;
						HK[i][hash[i]][j].FP = FP; //如果找到空闲的位置，就插入指纹和计数器为1
						HK[i][hash[i]][j].C = 1;
						maxv = max(maxv,1);
						count = 1;
						break;
					}
				}
			}else{	//该桶发生了融合
				for (int j = 1; j < BN-1; j++){ //寻找空闲位置或者计数器最小的位置
					if (mi > HK[i][hash[i]][j].C){
						ii=i; jj=j; mi=HK[i][hash[i]][j].C;
					}
					if (HK[i][hash[i]][j].FP == FP) {
						pos_i = i;
						pos_j = j;
						HK[i][hash[i]][j].C++; 
						maxv = max(maxv, HK[i][hash[i]][j].C);
						count = 1;
						break;
					}
					if(HK[i][hash[i]][j].FP == 0){
						pos_i = i;
						pos_j = j;
						HK[i][hash[i]][j].FP = FP; //如果找到空闲的位置，就插入指纹和计数器为1
						HK[i][hash[i]][j].C = 1;
						maxv = max(maxv,1);
						count = 1;
						break;
					}
				}
			}	
		}
			
		if (count == 0) { //如果没有找到空闲的位置或者相同的指纹，就替换掉计数器最小的位置，并重哈希
			// temp.FP = HK[ii][hash[ii]][jj].FP;
			// temp.C = HK[ii][hash[ii]][jj].C;
			HK[ii][hash[ii]][jj].FP = FP;
			HK[ii][hash[ii]][jj].C = 1;
			pos_i = ii;
			pos_j = jj;
			maxv = max(maxv, 1);
			// rehash(temp, max_loop, ii, hashHH[ii]);
		}

		if(HK[pos_i][hash[pos_i]][BN-1].ID == "\0"){	//本桶top为空
			HK[pos_i][hash[pos_i]][BN-1].ID = x;
			HK[pos_i][hash[pos_i]][BN-1].C = maxv;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
		}else if(HK[1-pos_i][hash[1-pos_i]][BN-1].ID=="\0"){	//备用桶top为空
			HK[1-pos_i][hash[1-pos_i]][BN-1].ID = x;
			HK[1-pos_i][hash[1-pos_i]][BN-1].C = maxv;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
		}else if(maxv-HK[pos_i][hash[pos_i]][BN-1].C == 1){	//替代本桶top
			int fp = Hash(HK[pos_i][hash[pos_i]][BN-1].ID)>>56;
			int c = HK[pos_i][hash[pos_i]][BN-1].C;
			HK[pos_i][hash[pos_i]][BN-1].ID = x;
			HK[pos_i][hash[pos_i]][BN-1].C = maxv;
			HK[pos_i][hash[pos_i]][pos_j].C = c;
			HK[pos_i][hash[pos_i]][pos_j].FP = fp;
		}else if(maxv-HK[1-pos_i][hash[1-pos_i]][BN-1].C == 1){	//替代候选桶top
			int fp = Hash(HK[1-pos_i][hash[1-pos_i]][BN-1].ID)>>56;
			int c = HK[1-pos_i][hash[1-pos_i]][BN-1].C;
			HK[1-pos_i][hash[1-pos_i]][BN-1].ID = x;
			HK[1-pos_i][hash[1-pos_i]][BN-1].C = maxv;
			HK[1-pos_i][hash[1-pos_i]][0].C = c;
			HK[1-pos_i][hash[1-pos_i]][0].FP = fp;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
		}
	}
	struct Node { string x; int y; int thre;} q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }

	void work()
	{
		int CNT = 0;
		for (int i = 0; i < CC_d; i++) {
			for (int j = 0;j < M2+5;j++){
				q[i*(M2+5)+j].x = HK[i][j][BN-1].ID; 
				q[i*(M2+5)+j].y = HK[i][j][BN-1].C;
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
		return "CuckooCounter31";
	}
};
#endif