#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <map>
#include <fstream>
#include <time.h>
#include <unistd.h>
#include "BOBHASH32.h"
#include "params.h"
#include "ssummary.h"
#include "heavykeeper.h"
#include "spacesaving.h"
#include "LossyCounting.h"
#include "CuckooCounter.h"
#include "ASketch.h"
#include "MVSketch.h"
#include "NitroSketch.h"
#include "ElasticSketch.h"
#include "cuckoo_counter2.h"
#include "cuckoo_counter3.h"

using namespace std;
map <string ,int> B,C;
struct node {string x;int y;} p[32000005];
//ifstream fin("../../real_data/1.dat",ios::in|ios::binary);
string s[32000005];
char tmp[105];
int cmp(node i,node j) {return i.y>j.y;}
int main(int argc, char** argv)
{
    int MEM=400,K=1000;
    int c;
    char dataset[40]={'\0'};
    while((c=getopt(argc, argv, "d:m:k:"))!=-1) {
        switch(c) {
            case 'd':
                strcpy(dataset,optarg);
                break;
            case 'm':
                MEM=atoi(optarg);
                break;
            case 'k':
                K=atoi(optarg);
                break;
        }
    }
    cout<<"MEM="<<MEM<<"KB"<<endl<<"Find top "<<K<<endl<<endl;
    cout<<"preparing all algorithms"<<endl;
    int m=MAX_INSERT;  // the number of flows

    // preparing cuckoocounter
    int cc_M;
    for (cc_M = 1; 32 * cc_M*CC_d + 432 * K <= MEM * 1000 * 8; cc_M++); if (cc_M % 2 == 0) cc_M--;
    cuckoocounter *cc; cc = new cuckoocounter(cc_M, K, 3, 0.01); cc->clear();

    int cc2_M;
    for (cc2_M = 1; 32 * cc_M*CC_d + 48 * K <= MEM * 1000 * 8; cc2_M++); if (cc2_M % 2 == 0) cc2_M--;
    CCCounter2 *cc2; cc2 = new CCCounter2(cc_M, K, 500);

    int cc3_M;
    for (cc3_M = 1; 64 * cc_M*CC_d <= MEM * 1000 * 8; cc3_M++); if (cc3_M % 2 == 0) cc3_M--;
    CCCounter3 *cc3; cc3 = new CCCounter3(cc_M, 500);
	
    // preparing heavykeeper
    int hk_M;
    for (hk_M=1; 32*hk_M*HK_d+432*K<=MEM*1024*8; hk_M++); if (hk_M%2==0) hk_M--;
    heavykeeper *hk; hk=new heavykeeper(hk_M,K); hk->clear();

    // preparing spacesaving
    int ss_M;
    for (ss_M=1; 432*ss_M<=MEM*1024*8; ss_M++);
    spacesaving *ss; ss=new spacesaving(ss_M,K);

    // preparing LossyCounting
    int LC_M;
    for (LC_M=1; 227*LC_M<=MEM*1024*8; LC_M++);
    LossyCounting *LC; LC=new LossyCounting(K);
    
    // preparing ASketch
    int A_M;

    for (A_M=1; 16*A_M*2+432*K<=MEM*1024*8; A_M++);
    ASketch *a; a=new ASketch(A_M,K);
 
    // preparing MVSketch
    int MV_M;
    for (MV_M=1; 32*MV_M*MV_d+432*K<=MEM*1024*8; MV_M++);
    mvsketch *mv; mv = new mvsketch(MV_M,K); mv->clear();

    // preparing NitroSketch
    int ns_M;
    for (ns_M=1; 8*ns_M*ns_d+432*K<=MEM*1024*8; ns_M++);
    nitrosketch *ns; ns = new nitrosketch(ns_M,K); ns->clear();
   
    // preparing ElasticSketch
    int ES_M;
    for (ES_M=1; 32*4*ES_M+32*ES_M+432*K<=MEM*1024*8; ES_M++);
    elasticsketch *es; es = new elasticsketch(ES_M,ES_M/ES_d,K); es->clear();


        // Inserting
	timespec time1, time2;
	long long resns;
	char default_dataset[40]="../../real_data/1.dat";
	if(dataset[0]=='\0') strcpy(dataset, default_dataset);
	cout<<"dataset: "<<dataset<<endl<<endl;
	ifstream fin(dataset, ios::in|ios::binary);
	if(!fin) {printf("Dataset not exists!\n");return -1;}
	for (int i = 1; i <= m; i++)
	{
		fin.read(tmp, KEY_LEN);
		tmp[KEY_LEN]='\0';
		s[i] = tmp;
		B[s[i]]++;
	}
	printf("*************throughput************\n");

	//CC
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		cc->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_cc = (double)1000.0  * m/ resns;
	printf("throughput of CC (insert): %.6lf Mips\n", throughput_cc);
    //CC2
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		cc2->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_cc2 = (double)1000.0  * m/ resns;
	printf("throughput of CC2 (insert): %.6lf Mips\n", throughput_cc2);

    //CC3
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		cc3->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_cc3 = (double)1000.0  * m/ resns;
	printf("throughput of CC3 (insert): %.6lf Mips\n", throughput_cc3);


	//HK
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		hk->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_hk = (double)1000.0  * m / resns;
	printf("throughput of HK (insert): %.6lf Mips\n", throughput_hk);

	//SS
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		ss->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_ss = (double)1000.0  * m / resns;
	printf("throughput of SS (insert): %.6lf Mips\n", throughput_ss);

	//LC
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		LC->Insert(s[i], i / LC_M); if (i%LC_M == 0) LC->clear(i / LC_M);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_lc = (double)1000.0  * m / resns;
	printf("throughput of LC (insert): %.6lf Mips\n", throughput_lc);

	//ASketch
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		a->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_a = (double)1000.0 * m / resns;
	printf("throughput of ASketch (insert): %.6lf Mips\n", throughput_a);

	//MVSketch
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		mv->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_mv = (double)1000.0 * m / resns;
	printf("throughput of MVSketch (insert): %.6lf Mips\n", throughput_mv);

	//NitroSketch
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		ns->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_ns = (double)1000.0 * m/resns;
	printf("throughput of NitroSketch (insert): %.6lf Mips\n", throughput_ns);

	//ElasticSketch
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int i = 1; i <= m; i++) {
		es->Insert(s[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_es = (double)1000.0 * m/resns;
	printf("throughput of ElasticSketch (insert): %.6lf Mips\n", throughput_es);

	cc->work();
    cc3->work();
	hk->work();
	ss->work();
	LC->work();
	a->work();
	mv->work();
	ns->work();
	es->work();

    cout<<"\npreparing true flow"<<endl;
    printf("\npreparing true flow\n");
	// preparing true flow
	int cnt=0;
    for (map <string,int>::iterator sit=B.begin(); sit!=B.end(); sit++)
    {
        p[++cnt].x=sit->first;
        p[cnt].y=sit->second;
    }
    sort(p+1,p+cnt+1,cmp);
    for (int i=1; i<=K+10; i++) C[p[i].x]=p[i].y;   

    // Calculating PRE, ARE, AAE
    cout<<"Calculating\n"<<endl;
	int cc_sum = 0, cc_AAE = 0; double cc_ARE = 0;
	string cc_string; int cc_num;
	for (int i = 0; i < K; i++)
	{
		cc_string = (cc->Query(i)).first; cc_num = (cc->Query(i)).second;
		cc_AAE += abs(B[cc_string] - cc_num); cc_ARE += abs(B[cc_string] - cc_num) / (B[cc_string] + 0.0);
		if (C[cc_string]) cc_sum++;
	}

    int cc2_sum = 0, cc2_AAE = 0; double cc2_ARE = 0;
	string cc2_string; int cc2_num;
	for (int i = 0; i < K; i++)
	{
		cc2_string = (cc2->Query(i)).first; cc2_num = (cc2->Query(i)).second;
		cc2_AAE += abs(B[cc2_string] - cc2_num); cc2_ARE += abs(B[cc2_string] - cc2_num) / (B[cc2_string] + 0.0);
		if (C[cc2_string]) cc2_sum++;
	}

    int cc3_sum = 0, cc3_AAE = 0; double cc3_ARE = 0;
	string cc3_string; int cc3_num;
	for (int i = 0; i < K; i++)
	{
		cc3_string = (cc3->Query(i)).first; cc3_num = (cc3->Query(i)).second;
		cc3_AAE += abs(B[cc3_string] - cc3_num); cc3_ARE += abs(B[cc3_string] - cc3_num) / (B[cc3_string] + 0.0);
		if (C[cc3_string]) cc3_sum++;
	}

    int hk_sum=0,hk_AAE=0; double hk_ARE=0;
    string hk_string; int hk_num;
    for (int i=0; i<K; i++)
    {
        hk_string=(hk->Query(i)).first; hk_num=(hk->Query(i)).second;
        hk_AAE+=abs(B[hk_string]-hk_num); hk_ARE+=abs(B[hk_string]-hk_num)/(B[hk_string]+0.0);
        if (C[hk_string]) hk_sum++;
    }

    int LC_sum=0,LC_AAE=0; double LC_ARE=0;
    string LC_string; int LC_num;
    for (int i=0; i<K; i++)
    {
        LC_string=(LC->Query(i)).first; LC_num=(LC->Query(i)).second;
        LC_AAE+=abs(B[LC_string]-LC_num); LC_ARE+=abs(B[LC_string]-LC_num)/(B[LC_string]+0.0);
        if (C[LC_string]) LC_sum++;
    }

    int ss_sum=0,ss_AAE=0; double ss_ARE=0;
    string ss_string; int ss_num;
    for (int i=0; i<K; i++)
    {
        ss_string=(ss->Query(i)).first; ss_num=(ss->Query(i)).second;
        ss_AAE+=abs(B[ss_string]-ss_num); ss_ARE+=abs(B[ss_string]-ss_num)/(B[ss_string]+0.0);
        if (C[ss_string]) ss_sum++;
    }
    
    int a_sum=0,a_AAE=0; double a_ARE=0;
    string a_string; int a_num;
    for (int i=0; i<K; i++)
    {
        a_string=(a->Query(i)).first; a_num=(a->Query(i)).second;
        a_AAE+=abs(B[a_string]-a_num); a_ARE+=abs(B[a_string]-a_num)/(B[a_string]+0.0);
        if (C[a_string]) a_sum++;
    }

    int mv_sum=0,mv_AAE=0; double mv_ARE=0;
    string mv_string; int mv_num;
    for (int i=0; i<K; i++)
    {
        mv_string=(mv->Query(i)).first; mv_num=(mv->Query(i)).second;
        mv_AAE+=abs(B[mv_string]-mv_num); mv_ARE+=abs(B[mv_string]-mv_num)/(B[mv_string]+0.0);
        if (C[mv_string]) mv_sum++;
    }

    int ns_sum=0,ns_AAE=0; double ns_ARE=0;
    string ns_string; int ns_num;
    for (int i=0; i<K; i++)
    {
	    ns_string=(ns->Query(i)).first; ns_num=(ns->Query(i)).second;
	    ns_AAE+=abs(B[ns_string]-ns_num); ns_ARE+=abs(B[ns_string]-ns_num)/(B[ns_string]+0.0);
	    if (C[ns_string]) ns_sum++;
    }
 
    int es_sum=0,es_AAE=0; double es_ARE=0;
    string es_string; int es_num;
    for (int i=0; i<K; i++)
    {
	    es_string=(es->Query(i)).first; es_num=(es->Query(i)).second;
	    es_AAE+=abs(B[es_string]-es_num); es_ARE+=abs(B[es_string]-es_num)/(B[es_string]+0.0);
	    if (C[es_string]) es_sum++;
    }   

    printf("cuckoocounter:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",cc_sum,K,(cc_sum/(K + 0.0)),cc_ARE/K,cc_AAE/(K + 0.0));   
    printf("cuckoocounter2:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",cc2_sum,K,(cc2_sum/(K + 0.0)),cc2_ARE/K,cc2_AAE/(K + 0.0));  
    printf("cuckoocounter3:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",cc3_sum,K,(cc3_sum/(K + 0.0)),cc3_ARE/K,cc3_AAE/(K + 0.0));   
    printf("heavkeeper:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",hk_sum,K,(hk_sum/(K+0.0)),hk_ARE/K,hk_AAE/(K+0.0));
    printf("LossyCounting:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",LC_sum,K,(LC_sum/(K+0.0)),LC_ARE/K,LC_AAE/(K+0.0));
    printf("spacesaving:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",ss_sum,K,(ss_sum/(K+0.0)),ss_ARE/K,ss_AAE/(K+0.0));
    printf("ASketch:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",a_sum,K,(a_sum/(K+0.0)),a_ARE/K,a_AAE/(K+0.0));
    printf("MVSketch:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",mv_sum,K,(mv_sum/(K+0.0)),mv_ARE/K,mv_AAE/(K+0.0));
    printf("NitroSketch:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",ns_sum,K,(ns_sum/(K+0.0)),ns_ARE/K,ns_AAE/(K+0.0));
    printf("ElasticSketch:\nAccepted: %d/%d  %.10f\nARE: %.10f\nAAE: %.10f\n\n",es_sum,K,(es_sum/(K+0.0)),es_ARE/K,es_AAE/(K+0.0));
	
	return 0;
}
