#ifndef _CCCOUNTER5_H
#define _CCCOUNTER5_H

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
class cuckoocounter5
{
private:
	struct entry5 {int C;string str;} top[CC_d][MAX_MEM + 10];

	struct node { int C, FP; } HK[CC_d][MAX_MEM + 10][BN];
	BOBHash64 * bobhash;
	int K, M2;
    int threshold1; //热流判定阈值
    int threshold2; //entry5规模阈值
    double threshold3;   //比例系数
	
public:
	cuckoocounter5(int M2, int K,int value) :M2(M2), K(K), threshold1(value)
	{
        threshold2 = 2*threshold1;
        threshold3 = 0.2;
        bobhash = new BOBHash64(1005);      
    }
	void clear()
	{
		for (int i = 0; i < CC_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				for(int r=0;r<BN;r++)HK[i][j][r].C = HK[i][j][r].FP = 0;
        
        for(int i=0;i<CC_d;i++){
            for(int j=0;j<M2+5;j++){
                top[i][j].C = 0;
                top[i][j].str = "\0";
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
                    if(maxv>threshold1){    //计数器大于阈值1，可能为热流
                        if(top[i][hash[i]].C<threshold2){	//情形1：entry5较小，此时比例系数为1，直接比较是否替换
                            if(maxv>top[i][hash[i]].C){	//情形1.1：计数器大于entry5,替换。反之不替换
                                unsigned long long temp_H1 = Hash(top[i][hash[i]].str); 
                                int temp_FP = (temp_H1 >> 56);
                                HK[i][hash[i]][j].FP = temp_FP;
                                top[i][hash[i]].str = x;
                                HK[i][hash[i]][j].C = top[i][hash[i]].C;
                                top[i][hash[i]].C = maxv;
					        }	
				        }else{	//情形2：entry5较大，此时计数器较难超过entry5,更改比例系数来提升entry5使用率
                            if(maxv>top[i][hash[i]].C*threshold3){  //计数器认为是热流，寻找替换桶的entry5并执行替换策略
                                if(maxv>top[1-i][hash[1-i]].C){ //计数器大于替换桶的entry5
                                    unsigned long long temp_H1 = Hash(top[1-i][hash[1-i]].str); 
                                    int temp_FP = (temp_H1 >> 56);
                                    int temp_C = top[1-i][hash[1-i]].C;
                                    top[1-i][hash[1-i]].str = x;
                                    top[1-i][hash[1-i]].C = maxv;
                                    HK[1-i][hash[1-i]][BN-1].C = temp_C;
                                    HK[1-i][hash[1-i]][BN-1].FP = temp_FP;
                                }
                            }
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

    struct Node { string x; int y;} q[MAX_MEM + 10];
    static int cmp(Node i, Node j) { return i.y > j.y; }
    
    void work()
	{
        int cnt = 0;
		for (int i = 0; i<CC_d; i++){
            for(int j=0;j<M2+5;j++){
                cnt++;
                q[i*(M2+5)+j].y = top[i][j].C;
                q[i*(M2+5)+j].x = top[i][j].str;     
            }
        }
        sort(q,q+cnt,cmp);
	}
	
	pair<string, int> Query(int k)
	{
		if(k<2*M2){
            return make_pair(q[k].x,q[k].y);
        }else{
            return make_pair(q[2*M2-1].x,q[2*M2-1].y);
        }
	}
};
#endif
