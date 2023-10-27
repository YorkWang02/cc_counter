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
#include "ssummary.h"
#include "BOBHASH64.h"
#define CC_d 2
#define BN 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class cuckoocounter31 : public sketch::BaseSketch
{
private:
	struct topk {int C; string ID; } heap[CC_d][MAX_MEM + 10];
	struct node { int C, FP; } HK[CC_d][MAX_MEM + 10][BN];
	BOBHash64 * bobhash;
	int K, M2;
	int thresh;
	double epsilon;
public:
	cuckoocounter31(int M2, int K, int t, double e) :M2(M2), K(K), thresh(t), epsilon(e)
	{  bobhash = new BOBHash64(1005); }
	void clear()
	{
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				for(int r=0;r<BN;r++)HK[i][j][r].C = HK[i][j][r].FP = 0;
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++){
				heap[i][j].C = 0;
				heap[i][j].ID = '\0';
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

	void Insert(const string &x)
	{
		bool mon = false;
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
		int index;
		for(int i=0;i<CC_d;i++){
			if(heap[i][hash[i]].ID == x){
				mon = true;
				index = i;
				break;
			}
		}
		if(mon){
			heap[index][hash[index]].C++;
			return;
		}
		int ii, jj, mi=(1<<25);	
		int pos_i, pos_j;
		for(int i = 0; i < CC_d; i++)
			for (int j = 0; j < BN; j++)
			{	
				if (mi > HK[i][hash[i]][j].C){
					ii=i; jj=j; mi=HK[i][hash[i]][j].C;
				}	
				if (HK[i][hash[i]][j].FP == FP) {
					pos_i=i;
					pos_j=j;
					int c = HK[i][hash[i]][j].C;
					if (mon || c <= heap[i][hash[i]].C)
						HK[i][hash[i]][j].C++;
					maxv = max(maxv, HK[i][hash[i]][j].C);
					count = 1;
					// testflag = 1;testi = i;testj = hash[i];testr = j;
					break;
				}
				if(HK[i][hash[i]][j].FP == 0)
				{
					pos_i=i;
					pos_j=j;
					HK[i][hash[i]][j].FP=FP;
					HK[i][hash[i]][j].C=1;
					maxv=max(maxv,1);
					count = 1;
					break;
				}
			}

		if (count == 0) {	//mean can not insert normally
			// temp.FP = HK[ii][hash[ii]][jj].FP;
			// temp.C = HK[ii][hash[ii]][jj].C;
			HK[ii][hash[ii]][jj].FP = FP;
			HK[ii][hash[ii]][jj].C = 1;
			pos_i=ii;
			pos_j=jj;
			maxv=max(maxv, 1);
			// rehash(temp, max_loop, ii, hashHH[ii]);
		}

		if(heap[0][hash[0]].ID=="\0"){
			heap[0][hash[0]].ID = x;
			heap[0][hash[0]].C = maxv;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
		}else if(heap[1][hash[1]].ID=="\0"){
			heap[1][hash[1]].ID = x;
			heap[1][hash[1]].C = maxv;
			HK[pos_i][hash[pos_i]][pos_j].C = 0;
			HK[pos_i][hash[pos_i]][pos_j].FP = 0;
		}
		// else if(maxv-heap[0][hash[0]].C==1){	//流交换
		// 	int fp = Hash(heap[0][hash[0]].ID)>>56;
		// 	int c = heap[0][hash[0]].C;
		// 	heap[0][hash[0]].ID = x;
		// 	heap[0][hash[0]].C = maxv;
		// 	HK[pos_i][hash[pos_i]][pos_j].C = c;
		// 	HK[pos_i][hash[pos_i]][pos_j].FP = fp;
		// }else if(maxv-heap[1][hash[1]].C==1){
		// 	int fp = Hash(heap[1][hash[1]].ID)>>56;
		// 	int c = heap[1][hash[1]].C;
		// 	heap[1][hash[1]].ID = x;
		// 	heap[1][hash[1]].C = maxv;
		// 	HK[pos_i][hash[pos_i]][pos_j].C = c;
		// 	HK[pos_i][hash[pos_i]][pos_j].FP = fp;
		// }
		else if(maxv-heap[pos_i][hash[pos_i]].C==1){	//流下放
			int fp = Hash(heap[pos_i][hash[pos_i]].ID)>>56;
			int c = heap[pos_i][hash[pos_i]].C;
			heap[pos_i][hash[pos_i]].ID = x;
			heap[pos_i][hash[pos_i]].C = maxv;
			HK[pos_i][hash[pos_i]][pos_j].C = c;
			HK[pos_i][hash[pos_i]][pos_j].FP = fp;
		}else if(maxv-heap[1-pos_i][hash[1-pos_i]].C==1){
			int fp = Hash(heap[1-pos_i][hash[1-pos_i]].ID)>>56;
			int c = heap[1-pos_i][hash[1-pos_i]].C;
			heap[1-pos_i][hash[1-pos_i]].ID = x;
			heap[1-pos_i][hash[1-pos_i]].C = maxv;
			HK[1-pos_i][hash[1-pos_i]][0].C = c;
			HK[1-pos_i][hash[1-pos_i]][0].FP = fp;
		}
	}
	struct Node { string x; int y; int thre;} q[MAX_MEM + 10];
	static int cmp(Node i, Node j) { return i.y > j.y; }
	void work()
	{
		int CNT = 0;
		for (int i = 0; i < CC_d; i++) {
			for (int j = 0;j < M2+5;j++){
				q[i*(M2+5)+j].x = heap[i][j].ID; 
				q[i*(M2+5)+j].y = heap[i][j].C;
				CNT++;
			}
		}
		sort(q, q + CNT, cmp);
	}
	int jump=0, x_num=0;
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
