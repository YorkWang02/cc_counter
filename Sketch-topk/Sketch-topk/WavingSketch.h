#ifndef _wavingsketch_H
#define _wavingsketch_H
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
#define WS_d 4 // waving sketch的heavy part的宽度
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class wavingsketch : public sketch::BaseSketch {
private:
    ssummary *ss; // 上层ssummary
    struct item { int FP, freq, flag; }; // 下层waving sketch中的元素
    struct bucket { int count; item heavy[WS_d]; }; // 下层waving sketch中的桶
    bucket WS[MAX_MEM+10]; // 下层waving sketch
    BOBHash64 * bobhash; // 哈希函数对象
    int K, M2; // topk参数，桶的个数
public:
    wavingsketch(int M2, int K): K(K), M2(M2) {
        ss = new ssummary(K);
        bobhash = new BOBHash64(1005);
        ss->clear();
    }
    void clear() {
        memset(WS, 0, sizeof(WS));
    }
    unsigned long long Hash(string ST)
	{
		return (bobhash->run(ST.c_str(), ST.size()));
	}
    void Insert(const string &x) {
        bool mon = false;
        int p = ss->find(x);
        if (p) mon = true;
        int minv = 0x7fffffff;
        int hash = Hash(x) % M2;int FP = (hash >> 32);
        int sign = (Hash(x + std::to_string(1)) & 1) ? 1 : -1;
        bool found = false;
        rep(i, 0, WS_d-1) {
            if (WS[hash].heavy[i].FP == FP) {
                found = true;
                WS[hash].heavy[i].freq++;
                minv = WS[hash].heavy[i].freq;
                if (!WS[hash].heavy[i].flag) WS[hash].count += sign;
                break;
            }
        }
        if (!found) {
            bool empty = false;
            rep(i, 0, WS_d-1) {
                if (WS[hash].heavy[i].FP == 0) {
                    empty = true;
                    WS[hash].heavy[i].FP = FP;
                    WS[hash].heavy[i].freq = 1;
                    WS[hash].heavy[i].flag = true;
                    minv = 1;
                    break;
                }
            }
            if (!empty) {
                minv = 0;
                int min_freq = 0x7fffffff, min_pos = -1;
                rep(i, 0, WS_d-1) {
                    if (WS[hash].heavy[i].freq < min_freq) {
                        min_freq = WS[hash].heavy[i].freq;
                        min_pos = i;
                    }
                }
                if (WS[hash].count * sign >= min_freq) {
                    if (WS[hash].heavy[min_pos].flag) WS[hash].count += min_freq * ((Hash(to_string(WS[hash].heavy[min_pos].FP)) & 1) ? 1 : -1);
                    WS[hash].heavy[min_pos].FP = FP;
                    WS[hash].heavy[min_pos].freq = min_freq + 1;
                    minv = WS[hash].heavy[min_pos].freq;
                    WS[hash].heavy[min_pos].flag = false;
                }
                WS[hash].count += sign;
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
	static int cmp(Node i, Node j) { return i.y > j.y; }
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
		return "wavingsketch";
	}
};
#endif
