#ifndef _hyperuss_H
#define _hyperuss_H
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BaseSketch.h"
#include "BOBHASH64.h"
#include "params.h"
#include "ssummary.h"
#define HU_d 4
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class hyperuss : public sketch::BaseSketch{
private:
	ssummary *ss;
	struct node { int FP, C; } HK[HU_d][MAX_MEM+10];
	BOBHash64 * bobhash;
	int K, M2;
public:
	hyperuss(int M2, int K) :M2(M2), K(K) { ss = new ssummary(K); ss->clear(); bobhash = new BOBHash64(1005); }
	void clear()	{
		for (int i = 0; i < HU_d; i++)
			for (int j = 0; j <= M2 + 5; j++)
				HK[i][j].C = HK[i][j].FP = 0;
	}
	unsigned long long Hash(string ST)	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
	void Insert(const string &x)	{
		bool mon = false;
		int p = ss->find(x);
		if (p) mon = true;
		int minv = 0x7fffffff;
		unsigned long long H1 = Hash(x); int FP = (H1 >> 32);
		unsigned long long hash[HU_d];
		for (int i = 0; i < HU_d; i++)
			hash[i]=Hash(x+std::to_string(i))%(M2-(2*HU_d)+2*i+3);

        bool flag0 = false,flag1 = false;
        for(int i = 0; i < HU_d; i++)
		{
			if (HK[i][hash[i]].FP == FP) {
				HK[i][hash[i]].C++;
				minv=HK[i][hash[i]].C;
                flag0 = true;
                break;
			}
        }
        if(!flag0){
            for(int i = 0; i < HU_d; i++)
		    {
			    if (HK[i][hash[i]].FP == 0) {
				    HK[i][hash[i]].FP=FP;
				    HK[i][hash[i]].C=1;
				    minv=1;
                    flag1 = true;
                    break;
			    }
            }
        }
        if(!flag0 && !flag1){
            int mini;
            for(int i=0;i<HU_d;i++){
                if(HK[i][hash[i]].C<minv){
                    minv = HK[i][hash[i]].C;
                    mini = i;
                }
            }
            double p = 1.0 / (minv + 1);
			double q = 1.0 - p;
			double r = (double)rand() / RAND_MAX;
			if (r < p) {
				HK[mini][hash[mini]].FP = FP;
				HK[mini][hash[mini]].C = 1 / p;
				minv=HK[mini][hash[mini]].C;
			}
			else {
				HK[mini][hash[mini]].C = minv / q;
				minv=HK[mini][hash[mini]].C;
			}
        }

		if (!mon)
		{
			if (minv - (ss->getmin()) > 0 || ss->tot < K)
			{
				int i = ss->getid();
				ss->add2(ss->location(x), i);
				ss->str[i] = x;
				ss->sum[i] = minv;
				ss->link(i, 0);
				while (ss->tot > K)
				{
					int t = ss->Right[0];
					int tmp = ss->head[t];
					ss->cut(ss->head[t]);
					ss->recycling(tmp);
				}
			}
		}
		else
			if (minv > ss->sum[p])
			{
				int tmp = ss->Left[ss->sum[p]];
				ss->cut(p);
				if (ss->head[ss->sum[p]]) tmp = ss->sum[p];
				ss->sum[p] = minv;
				ss->link(p, tmp);
			}
	}
	struct Node { string x; int y; } q[MAX_MEM + 10]; 
	static bool cmp(Node i, Node j) { return i.y > j.y; } 
    void work()
	{
		int CNT = 0;
		for (int i = N; i; i = ss->Left[i])
			for (int j = ss->head[i]; j; j = ss->Next[j]) { q[CNT].x = ss->str[j]; q[CNT].y = ss->sum[j]; CNT++; }
		sort(q, q + CNT, cmp);
	}
	pair<string, int> Query(int k)
	{
		return make_pair(q[k].x, q[k].y);
	}
	std::string get_name() {
		return "hyperuss";
	}
};
#endif
