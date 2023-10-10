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
#define CC_b 1.08 
#define BN 5 
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cuckoocounter31 : public sketch::BaseSketch{
private:
	struct node { int C, FP; string ID; } HK[CC_d][MAX_MEM + 10][BN]; 
	BOBHash64 * bobhash;
	int K, M2;
	int hot_thresh; //热流阈值
public:
	cuckoocounter31(int M2, int K, int h) :M2(M2), K(K), hot_thresh(h)
	{ 
		bobhash = new BOBHash64(1005); 
	}
	void clear()
	{
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				for(int r=0;r<BN;r++)
					HK[i][j][r].C = HK[i][j][r].FP = 0;
	}
	unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
	void rehash(node hash_entry, int loop_times, int j, unsigned long long hash) {
		int k=1-j;
		unsigned long long re_hash = hash ^ Hash(std::to_string(hash_entry.FP)); //XOR CUCKOO HASHING
		int Hsh = re_hash % (M2 - (2 * CC_d) + 2 *(1-j) + 3);
		if(HK[k][Hsh][BN-1].ID == ""){	//在topk部分寻找空闲位置
			HK[k][Hsh][BN-1].ID = hash_entry.ID;
			HK[k][Hsh][BN-1].C = hash_entry.C;
			return;
		}
		if (loop_times == 0) {
			HK[k][Hsh][BN-1].ID = hash_entry.ID;//no matter rehash the fingerprint or not 
			if (hash_entry.C<HK[k][Hsh][BN-1].C)
				HK[k][Hsh][BN-1].C = hash_entry.C;//replace entry with min count
				return;	
		}
		node tmp;
		tmp.ID = HK[k][Hsh][BN-1].ID;
		tmp.C = HK[k][Hsh][BN-1].C;
		HK[k][Hsh][BN-1].ID = hash_entry.ID;
		HK[k][Hsh][BN-1].C = hash_entry.C;
		hash_entry.ID = tmp.ID;
		hash_entry.C = tmp.C;
		loop_times--;
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
		//首先在topk部分判断是否存在
		for(int i = 0;i < CC_d; i++){
			if(HK[i][hash[i]][BN-1].ID == x){
				HK[i][hash[i]][BN-1].C++;
				return;
			}
		}
		//在heavy part部分判断是否存在
		for(int i = 0;i < CC_d; i++){
			for (int j = 0; j < BN-1; j++){
				if(HK[i][hash[i]][j].FP == FP){
					HK[i][hash[i]][j].C++;
					if(HK[i][hash[i]][j].C > hot_thresh){
						rehash(HK[i][hash[i]][BN-1], max_loop, i, hash[i]);
						HK[i][hash[i]][BN-1].ID = x;
						HK[i][hash[i]][BN-1].C = HK[i][hash[i]][j].C;
					}
					return;
				}
			}
		}
		//在heavy part部分判断是否有空位
		for(int i = 0;i < CC_d; i++){
			for (int j = 0; j < BN-1; j++){
				if(HK[i][hash[i]][j].FP == 0){
					HK[i][hash[i]][j].FP = FP;
					HK[i][hash[i]][j].C = 1;
					return;
				}
			}
		}
		//在heavy part部分执行指数衰减再判断是否有空位
		for(int i = 0;i < CC_d; i++){
			for (int j = 0; j < BN-1; j++){
				double prob_decay = pow(CC_b, -HK[i][hash[i]][j].C);
				if (rand() <= RAND_MAX * prob_decay) {
    				HK[i][hash[i]][j].C--;
				}
			}
		}
		for(int i = 0;i < CC_d; i++){
			for (int j = 0; j < BN-1; j++){
				if(HK[i][hash[i]][j].C==0){
					HK[i][hash[i]][j].FP = FP;
					HK[i][hash[i]][j].C = 1;
					return;
				}
			}
		}
		
	}
	struct Node { string x; int y; int thre;} q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }
	struct Node2 {int FP; int tmpFQ; int fnlFQ; } p[MAX_MEM + 10];
	static int cmp2(Node2 i, Node2 j) { 
		return i.FP < j.FP;
	}
	unordered_map<int, string> mp; //用于存储指纹和流id之间的映射关系
	void work()
	{
		int CNT = 0;
		for (int i = 0; i < CC_d; i++) {
			for (int j = 0;j < M2;j++){
				q[i].x = HK[i][j][BN-1].ID; 
				q[i].y = HK[i][j][BN-1].C;
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