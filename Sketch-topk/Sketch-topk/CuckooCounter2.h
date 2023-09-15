#ifndef _cuckoocounter2_H
#define _cuckoocounter2_H
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
class cuckoocounter2 : public sketch::BaseSketch{
private:
	int K, M2;
	int T; // 热流阈值
	double epsilon;
    struct bucket { string ID;int C; } TK[MAX_MEM+10]; //上层
	struct node { int C, FP; } HK[CC_d][MAX_MEM + 10][BN]; // 下层
	BOBHash64 * bobhash;
public:
	cuckoocounter2(int M2, int K, int t, double e) :M2(M2), K(K), T(t), epsilon(e)
	{ bobhash = new BOBHash64(1005); }
	void clear()
	{
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				for(int r=0;r<BN;r++)HK[i][j][r].C = HK[i][j][r].FP = 0;
        for (int i = 0; i < K; i++)
            TK[i].ID = TK[i].C = 0;
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
		rehash(hash_entry, loop_times, k, re_hash);
	}
	void Insert(const string &x)
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
                    int c = HK[i][hash[i]][j].C;
                    HK[i][hash[i]][j].C++;
                    maxv = max(maxv, HK[i][hash[i]][j].C);
                    count = 1;
                    if (maxv >= T) { // 计数器达到热流阈值
                        if(insert_topk(x, maxv)){   // 尝试插入上层
                            HK[i][hash[i]][j].FP = 0;
                        } 
                    }
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
    bool insert_topk(const string &x, int c) {
        string ID = x; 
        bool found = false;  
        for (int i = 0; i < K; i++) { //判断是否能匹配上
            if (TK[i].ID == ID) { 
                TK[i].C += c; 
                found = true; 
                break; 
            }
        }
        if (!found) {
            bool empty = false;
            for (int i = 0; i < K; i++) { //判断是否有空位
                if (TK[i].ID == "") { 
                    TK[i].ID = ID; 
                    TK[i].C = c; 
                    empty = true; 
                    break; 
                } 
            }
            if(!empty){
                if(c > TK[K-1].C){
                    TK[K-1].ID = ID;
                    TK[K-1].C = c;
                }else{
                    return false;
                }
            }
        }
        for(int i = 0;i < K-1;i++){ //冒泡排序
            for(int j = 0;j < K-1;j++){
                if(TK[j].C < TK[j+1].C){
                    string tmp_ID = TK[j].ID;
                    int tmp_c = TK[j].C;
                    TK[j].ID = TK[j+1].ID;
                    TK[j].C = TK[j+1].C; 
                    TK[j+1].ID = tmp_ID;
                    TK[j+1].C = tmp_c;  
                }
            }
        }
        return true;
    }

	struct Node { string x; int y;} q[MAX_MEM+10]; 
	static int cmp(Node i, Node j) { return i.y > j.y; } 
	void work()
	{
		for (int i = 0; i < K; i++) {
			q[i].x = TK[i].ID; 
			q[i].y = TK[i].C;
		}
		sort(q, q + K, cmp);
	}
	pair<string, int> Query(int k)
	{
		return make_pair(q[k].x, q[k].y);
	}
	std::string get_name() override {
		return "CuckooCounter2";
	}
};
#endif
