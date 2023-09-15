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
	double epsilon; //比例系数
	int hot_thresh; //热流阈值
	int entry5_thresh; //entry5大小阈值
public:
	cuckoocounter3(int M2, int K, double e, int h, int e5) :M2(M2), K(K), epsilon(e), hot_thresh(h), entry5_thresh(e5)
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
		for (int r = 0; r < BN-1; r++) { //只在前4个entry中寻找空闲位置
			if (HK[k][Hsh][r].FP == 0) {
				HK[k][Hsh][r].FP = hash_entry.FP;
				HK[k][Hsh][r].C = hash_entry.C;
				return;
			}
		}
		if (loop_times == 0) {
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
		unsigned long long H2 = H1^FP;//XOR CUCKOO HASHING	
		int Hsh1 = H1 % (M2 - (2 * CC_d) + 2 * 0 + 3);
		int Hsh2 = H2 % (M2 - (2 * CC_d) + 2 * 1 + 3);				
		int hash[2] = { Hsh1, Hsh2 };
		unsigned long long hashHH[2] = { H1, H2 };
		int count = 0;
		int ii, jj, mi=(1<<25);
		for(int i = 0; i < CC_d; i++)
			for (int j = 0; j < BN-1; j++) //只在前4个entry中寻找空闲位置或者计数器最小的位置
			{
				if (mi > HK[i][hash[i]][j].C){
					ii=i; jj=j; mi=HK[i][hash[i]][j].C;
				}
				if (HK[i][hash[i]][j].FP == FP) {
					HK[i][hash[i]][j].C++; //如果找到相同的指纹，就增加计数器
					maxv = max(maxv, HK[i][hash[i]][j].C);
					count = 1;
					if(maxv > hot_thresh){	//如果计数器超过了热流阈值，就认为可能是top-k流
						if(HK[i][hash[i]][4].C < entry5_thresh){ 	//如果第5个entry的计数器小于entry5大小阈值，就直接比较
							if(maxv > HK[i][hash[i]][4].C){ //如果比第5个entry的计数器大，就替换掉，并交换位置
								temp.FP = HK[i][hash[i]][4].FP;
								temp.C = HK[i][hash[i]][4].C;
								temp.ID = HK[i][hash[i]][4].ID; //同时更新流id的值
								HK[i][hash[i]][4].FP = FP;
								HK[i][hash[i]][4].C = maxv;
								HK[i][hash[i]][4].ID = x; //同时更新流id的值
								HK[i][hash[i]][j].FP = temp.FP;
								HK[i][hash[i]][j].C = temp.C;
								HK[i][hash[i]][j].ID = temp.ID; //同时更新流id的值
							}
						}else{ 	//如果第5个entry的计数器大于等于entry5大小阈值，就乘以一个比例系数后比较
							if(maxv > HK[i][hash[i]][4].C * epsilon){ //如果乘以比例系数后比第5个entry的计数器大，就替换掉，并移动到另一个桶中的第4个位置
								temp.FP = HK[i][hash[i]][4].FP;
								temp.C = HK[i][hash[i]][4].C;
								temp.ID = HK[i][hash[i]][4].ID; //同时更新流id的值
								HK[i][hash[i]][4].FP = FP;
								HK[i][hash[i]][4].C = maxv;
								HK[i][hash[i]][4].ID = x; //同时更新流id的值
								HK[i][hash[i]][j].FP = 0; //将原来的位置置空
								HK[i][hash[i]][j].C = 0;
								HK[i][hash[i]][j].ID = ""; //同时更新流id的值
								int k=1-i; //另一个桶的编号
								unsigned long long re_hash = hashHH[k] ^ Hash(std::to_string(temp.FP)); //另一个桶的哈希值
								int Hsh = re_hash % (M2 - (2 * CC_d) + 2 *(1-k) + 3); //另一个桶的索引
								if(HK[k][Hsh][3].FP != 0) //如果另一个桶中的第4个位置已经有元素了，就丢弃该元素，不再移动
									return;
								HK[k][Hsh][3].FP = temp.FP; //将被替换掉的entry移动到另一个桶中的第4个位置
								HK[k][Hsh][3].C = temp.C;
								HK[k][Hsh][3].ID = temp.ID; //同时更新流id的值
							}
						}
					}
					break;
				}
				if(HK[i][hash[i]][j].FP == 0)
				{
					HK[i][hash[i]][j].ID=x;
					HK[i][hash[i]][j].FP=FP; //如果找到空闲的位置，就插入指纹和计数器为1
					HK[i][hash[i]][j].C=1;
					maxv=max(maxv,1);
					count = 1;
					break;
				}
			}
		if (count == 0) { //如果没有找到空闲的位置或者相同的指纹，就替换掉计数器最小的位置，并重哈希
			HK[ii][hash[ii]][jj].ID = x;
			HK[ii][hash[ii]][jj].FP = FP;
			HK[ii][hash[ii]][jj].C = 1;
			maxv=max(maxv, 1);
			//rehash(temp, max_loop, ii, hashHH[ii]);
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
				q[i].x = HK[i][j][4].ID; 
				q[i].y = HK[i][j][4].C;
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
		return "CuckooCounter";
	}
};
#endif