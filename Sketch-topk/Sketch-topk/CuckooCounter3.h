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

	void insert_top(int pos_i, int pos_j, const string &x, int FP, int count, int hash[2]){
		if(HK[pos_i][hash[pos_i]][BN-1].ID == "\0"){	//本桶top为空,则占用本桶top,原条目置空
			HK[pos_i][hash[pos_i]][BN-1].ID = x;
			HK[pos_i][hash[pos_i]][BN-1].C = count;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
		}else if(HK[1-pos_i][hash[1-pos_i]][BN-1].ID=="\0"){	//备用桶top为空,则占用备用桶top,原条目置空
			HK[1-pos_i][hash[1-pos_i]][BN-1].ID = x;
			HK[1-pos_i][hash[1-pos_i]][BN-1].C = count;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
		}else if(count-HK[pos_i][hash[pos_i]][BN-1].C == 1){	//计数器大于本桶top,则占用本桶top,交换两条目位置
			int fp = Hash(HK[pos_i][hash[pos_i]][BN-1].ID)>>56;
			int c = HK[pos_i][hash[pos_i]][BN-1].C;
			HK[pos_i][hash[pos_i]][BN-1].ID = x;
			HK[pos_i][hash[pos_i]][BN-1].C = count;
			HK[pos_i][hash[pos_i]][pos_j].C = c;
			HK[pos_i][hash[pos_i]][pos_j].FP = fp;
		}else if(count-HK[1-pos_i][hash[1-pos_i]][BN-1].C == 1){	//计数器大于候选桶top,则占用候选桶top,原条目置空,原候选桶top下放
			int fp = Hash(HK[1-pos_i][hash[1-pos_i]][BN-1].ID)>>56;
			int c = HK[1-pos_i][hash[1-pos_i]][BN-1].C;
			HK[1-pos_i][hash[1-pos_i]][BN-1].ID = x;
			HK[1-pos_i][hash[1-pos_i]][BN-1].C = count;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
			if(c <= 0xf){	//寻找一个可以刚好容纳的条目下放
				HK[1-pos_i][hash[1-pos_i]][0].C = c;
				HK[1-pos_i][hash[1-pos_i]][0].FP = fp;
			}else if(c <= 0xff){
				HK[1-pos_i][hash[1-pos_i]][2].C = c;
				HK[1-pos_i][hash[1-pos_i]][2].FP = fp;
			}else{
				HK[1-pos_i][hash[1-pos_i]][3].C = c;
				HK[1-pos_i][hash[1-pos_i]][3].FP = fp;
			}
		}
	}

	void Insert(const string &x){
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

		for(int i = 0; i < CC_d; i++){	//先遍历两个候选桶的entry5
			if(HK[i][hash[i]][BN-1].ID == x){
				HK[i][hash[i]][BN-1].C++;
				return;
			}
		}

		for(int i = 0; i < CC_d; i++)
			for (int j = 0; j < BN-1; j++) //寻找空闲位置或者计数器最小的位置
			{
				if (mi > HK[i][hash[i]][j].C){
					ii=i; jj=j; mi=HK[i][hash[i]][j].C;
				}
				if (HK[i][hash[i]][j].FP == FP) {
					pos_i = i;
					pos_j = j;
					HK[i][hash[i]][j].C++; 
					// if((j == 0 || j == 1) && HK[i][hash[i]][j].C > 0xf){	//4位计数器位数限制
					// 	HK[i][hash[i]][j].C = 0xf; 
					// }
					// if(j == 2 && HK[i][hash[i]][j].C > 0xff){	//8位计数器位数限制
					// 	HK[i][hash[i]][j].C = 0xff; 
					// }
					// if(j == 0 && HK[i][hash[i]][j].C > 0xff){	//8位计数器位数限制,课题二实验1
					// 	HK[i][hash[i]][j].C = 0xff; 
					// }
					maxv = max(maxv, HK[i][hash[i]][j].C);
					count = 1;
					break;
				}
				if(HK[i][hash[i]][j].FP == 0){	//如果找到空闲的位置，就插入指纹和计数器为1
					pos_i = i;
					pos_j = j;
					HK[i][hash[i]][j].FP = FP; 
					HK[i][hash[i]][j].C = 1;
					maxv = max(maxv,1);
					count = 1;
					break;
				}
			}
		if (count == 0) { //如果没有找到空闲的位置或者相同的指纹，就替换掉计数器最小的位置，并重哈希
			temp.FP = HK[ii][hash[ii]][jj].FP;
			temp.C = HK[ii][hash[ii]][jj].C;
			HK[ii][hash[ii]][jj].FP = FP;
			HK[ii][hash[ii]][jj].C = 1;
			pos_i = ii;
			pos_j = jj;
			maxv = max(maxv, 1);
			// rehash(temp, max_loop, ii, hashHH[ii]);
		}
		insert_top(pos_i, pos_j, x, FP, maxv, hash);	//和top比较
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
		return "CuckooCounter3";
	}
};
#endif