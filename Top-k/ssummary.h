#ifndef _ssummary_H
#define _ssummary_H

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "BOBHASH32.h"
#include "params.h"
#define len2 9973
#define rep(i,a,n) for(int i=a;i<=n;i++)
using namespace std;
class ssummary
{
    public:
        int tot;
        int sum[M+10],K,last[M+10],Next[M+10],ID[M+10],tmp[M+10];
        int head[N+10],Left[N+10],Right[N+10],num;
        string str[M+10];
        int head2[len2+10],Next2[M+10]; //head2表示哈希表，元素的str哈希后得到位置存放id。next2则是为了解决哈希冲突。
        BOBHash32 * bobhash;

        //K：最多可统计的元素种类数，受限于内存空间
        ssummary(int K):K(K) {bobhash=new BOBHash32(1000);}
        void clear()
        {
            memset(sum,0,sizeof(sum));
            memset(last,0,sizeof(Next));
            memset(Next2,0,sizeof(Next2));
	    memset(tmp,0,sizeof(tmp));
            rep(i,0,N)head[i]=Left[i]=Right[i]=0;
            rep(i,0,len2-1)head2[0]=0;
            tot=0;
            rep(i,1,M+2)ID[i]=i;
            num=M+2;
            Right[0]=N;
            Left[N]=0;
        }
        
        int getid()
        {
            //从ID数组中取出一个可用的元素编号，并返回它
            int i=ID[num--];    //将ID数组的最后一个可用元素弹出，并将num减1表示可用ID减少
            last[i]=Next[i]=sum[i]=Next2[i]=0;
            return i;
        }
        int location(string ST)
        {
            //利用bobhash将字符串映射到哈希表的位置并返回
            return (bobhash->run(ST.c_str(),ST.size()))%len2;
        }
        void add2(int x,int y)
        {
            //x表示哈希表中对应的位置，y是元素编号。
            Next2[y]=head2[x];  //为避免冲突，先记录原来的元素。Next2[y]指向哈希表原来的元素，其中y是要插入的元素编号
            head2[x]=y; //y插入哈希表，如果冲突则取代原来的元素
        }
        int find(string s)
        {
            //在哈希表中查找字符串s对应的元素编号
            for(int i=head2[location(s)];i;i=Next2[i])  
              if(str[i]==s)return i;
            return 0;
        }
        void linkhead(int i,int j)
        {
            //将出现次数为i的元素组插入到出现次数为j的元素组之后，形成一个新的链表节点
            Left[i]=j;
            Right[i]=Right[j];
            Right[j]=i;
            Left[Right[i]]=i;
        }
        void cuthead(int i)
        {
            //将出现次数为i的元素组从链表中删除
            int t1=Left[i],t2=Right[i];
            Right[t1]=t2;
            Left[t2]=t1;
        }
        int getmin()
        {
            //返回当前最小的出现次数
            if (tot<K) return 0;    //当前统计的元素个数小于元素种类数，则一定有元素从未出现，返回0
            if(Right[0]==N)return 1;
            return Right[0];
        }
        void link(int i,int ww)
        {
            //将编号为i的元素插入到出现次数为sum[i]的元素组中
            ++tot;  //统计的元素加1
            bool flag=(head[sum[i]]==0);    //flag标记该元素组是否为新创建的
            Next[i]=head[sum[i]];   //为避免冲突，先记录原来的元素。Next2[i]指向链表原来的元素，其中i是要插入的元素编号
            if(Next[i])last[Next[i]]=i; //双向链表，原来的元素也要指向插入的元素
            last[i]=0;
            head[sum[i]]=i;
            if(flag)
            {
                for(int j=sum[i]-1;j>0 && j>sum[i]-10;j--)
                if(head[j]){linkhead(sum[i],j);return;}
                linkhead(sum[i],ww);
            }
        }
        void cut(int i)
        {
            //将编号为i的元素从出现次数为sum[i]的元素组中删除
            --tot;
            if(head[sum[i]]==i)head[sum[i]]=Next[i];
            if(head[sum[i]]==0)cuthead(sum[i]);
            int t1=last[i],t2=Next[i];
            if(t1)Next[t1]=t2;
            if(t2)last[t2]=t1;
        }
        void recycling(int i)
        {
            //回收编号为i的元素，将其从哈希表中删除，并将编号放回id数组中
            int w=location(str[i]);
            if (head2[w]==i)
              head2[w]=Next2[i];
              else
              {
                  for(int j=head2[w];j;j=Next2[j])
                  if(Next2[j]==i)
                  {
                        Next2[j]=Next2[i];
                        break;
                  }
              }
            ID[++num]=i;
        }
};
#endif
