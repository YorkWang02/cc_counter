#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <vector>

#include "CMSketch.h"
#include "CUSketch.h"
#include "ASketch.h"
#include "PCUSketch.h"
#include "cuckoo_counter.h"
#include "cuckoo_counter2.h"
#include "cuckoo_counter3.h"
#include "cuckoo_counter4.h"
#include "ElasticSketch.h"
#include "NitroSketch.h"
#include "MVSketch.h"

using namespace std;


char * filename_stream = "../../data/";


char insert[20000000 + 1000000 / 5][14];
char query[20000000 + 1000000 / 5][14];


unordered_map<string, int> unmp;

#define testcycles 2
#define hh 0.0002//0.00002
#define hc 0.0005

int main(int argc, char** argv)
{
    double memory = 0.1;
    if(argc >= 2)
    {
        filename_stream = argv[1];
    }
    if (argc >= 3)
    {
    	memory = stod(argv[2]);
    }
    

    unmp.clear();
    int val;



    //const double memory = 0.4;// MB
    int memory_ = memory * 1000;//KB
    int word_size = 64;


    int w = memory_ * 1024 * 8.0 / COUNTER_SIZE;	//how many counter;
    int w_p = memory * 1024 * 1024 * 8.0 / (word_size * 2);
    int m1 = memory * 1024 * 1024 * 1.0/4 / 8 / 8;
    int m2 = memory * 1024 * 1024 * 3.0/4 / 2 / 1;
    int m2_mv = memory * 1024 * 1024 / 8 / 4;

    printf("\n******************************************************************************\n");
    printf("Evaluation starts!\n\n");

    CMSketch *cmsketch;
    CUSketch *cusketch;
    ASketch *asketch;
    PCUSketch *pcusketch;
    CCCounter *cccounter;
    CCCounter2 *cccounter2;
    CCCounter3 *cccounter3;
    CCCounter4 *cccounter4;
    Nitrosketch *nitrosketch;
    Elasticsketch *elasticsketch;
    MVsketch *mvsketch;

    char _temp[200], temp2[200];
    int t = 0;

    int package_num = 0;


    FILE *file_stream = fopen(filename_stream, "r");

    //while(fgets(insert[package_num], 105, file_stream) != NULL)
    while (fread(insert[package_num], 1, KEY_LEN, file_stream)==KEY_LEN)
    {
        unmp[string(insert[package_num])]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_stream);
 
    printf("memory = %dKB\n", memory_);
    printf("dataset name: %s\n", filename_stream);
    printf("total stream size = %d\n", package_num);
    printf("distinct item number = %d\n", unmp.size());
    
    int max_freq = 0;
    unordered_map<string, int>::iterator it = unmp.begin();

    for(int i = 0; i < unmp.size(); i++, it++)
    {
        strcpy(query[i], it->first.c_str());

        int temp2 = it->second;
        max_freq = max_freq > temp2 ? max_freq : temp2;
    }
    printf("max_freq = %d\n", max_freq);
    
    printf("*************************************\n");



/********************************insert*********************************/

    timespec time1, time2;
    long long resns;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        cmsketch = new CMSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; i++)
        {
            cmsketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cm = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of CM (insert): %.6lf Mips\n", throughput_cm);
   


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        asketch = new ASketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; i++)
        {
            asketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_a = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of A (insert): %.6lf Mips\n", throughput_a);



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        cusketch = new CUSketch(w / LOW_HASH_NUM, LOW_HASH_NUM);
        for(int i = 0; i < package_num; i++)
        {
            cusketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_cu = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of CU (insert): %.6lf Mips\n", throughput_cu);
	


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        pcusketch = new PCUSketch(w_p, LOW_HASH_NUM, word_size);
        for(int i = 0; i < package_num; i++)
        {
            pcusketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_pcusketch = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of PCU (insert): %.6lf Mips\n", throughput_pcusketch);

		
	clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter = new CCCounter(memory * 1000 *1000/8);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter = (double)1000.0 * testcycles * package_num / resns;
        //printf("throughput of CC (insert): %.6lf Mips\n", throughput_cccounter);

    clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter2 = new CCCounter2(memory * 1000 *1000/8,package_num*hh);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter2->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter2 = (double)1000.0 * testcycles * package_num / resns;
        //printf("throughput of CC (insert): %.6lf Mips\n", throughput_cccounter);

    clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter3 = new CCCounter3(memory * 1000 *1000/16);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter3->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter3 = (double)1000.0 * testcycles * package_num / resns;
        //printf("throughput of CC (insert): %.6lf Mips\n", throughput_cccounter);

    clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter4 = new CCCounter4(memory * 1000 *1000/8,w / LOW_HASH_NUM, LOW_HASH_NUM);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter4->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter4 = (double)1000.0 * testcycles * package_num / resns;
        //printf("throughput of CC (insert): %.6lf Mips\n", throughput_cccounter);


	
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < testcycles; t++)
	{
		elasticsketch = new Elasticsketch(m1, m2);
		for (int i = 0; i < package_num; i++)
		{
			elasticsketch->Insert(insert[i]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_elastic = (double)1000.0 * testcycles * package_num / resns;
	//printf("throughput of Elastic (insert): %.6lf Mips\n", throughput_elastic);


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        nitrosketch = new Nitrosketch(w / LOW_HASH_NUM, LOW_HASH_NUM, 0.01);
        for(int i = 0; i < package_num; i++)
        {
            nitrosketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_nitro = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of Nitro (insert): %.6lf Mips\n", throughput_nitro);

    
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        mvsketch = new MVsketch(m2_mv);
        for(int i = 0; i < package_num; i++)
        {
            mvsketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_mvsketch = (double)1000.0 * testcycles * package_num / resns;
    //printf("throughput of MVsketch (insert): %.6lf Mips\n", throughput_mvsketch);	

 
/********************************************************************************************/


    //avoid the over-optimize of the compiler! 
    double sum = 0;

    if(sum == (1 << 30))
        return 0;

    char temp[105];

    double re_cm = 0.0, re_cu = 0.0,  re_a = 0.0,  re_pcusketch = 0.0, re_pcsketch = 0.0, re_cccounter = 0.0,re_cccounter2 = 0.0,re_cccounter3 = 0.0,re_cccounter4 = 0.0, re_elastic=0.0, re_nitro=0.0, re_mvsketch=0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0,  re_a_sum = 0.0,  re_pcusketch_sum = 0.0, re_cccounter_sum = 0.0,re_cccounter2_sum = 0.0,re_cccounter3_sum = 0.0,re_cccounter4_sum = 0.0, re_elastic_sum=0.0, re_nitro_sum=0.0, re_mvsketch_sum=0.0;
    
    double ae_cm = 0.0, ae_cu = 0.0,  ae_a = 0.0,  ae_pcusketch = 0.0, ae_cccounter = 0.0,ae_cccounter2 = 0.0,ae_cccounter3 = 0.0,ae_cccounter4 = 0.0, ae_elastic=0.0, ae_nitro=0.0, ae_mvsketch=0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0,  ae_a_sum = 0.0,  ae_pcusketch_sum = 0.0, ae_cccounter_sum = 0.0,ae_cccounter2_sum = 0.0,ae_cccounter3_sum = 0.0,ae_cccounter4_sum = 0.0, ae_elastic_sum=0.0, ae_nitro_sum=0.0, ae_mvsketch_sum=0.0;

    double val_cm = 0.0, val_cu = 0.0,  val_a = 0.0,  val_pcusketch = 0.0, val_cccounter = 0.0,val_cccounter2 = 0.0,val_cccounter3 = 0.0,val_cccounter4 = 0.0, val_elastic=0.0, val_nitro=0.0, val_mvsketch=0.0;
    double mem_cc = 0.0, mem_cc_sum = 0.0,mem_cc2 = 0.0, mem_cc2_sum = 0.0,mem_cc3 = 0.0, mem_cc3_sum = 0.0,mem_cc4 = 0.0, mem_cc4_sum = 0.0;

    double rc_cm = 0.0, rc_cu = 0.0,  rc_a = 0.0,  rc_pcusketch = 0.0, rc_cccounter = 0.0,rc_cccounter2 = 0.0,rc_cccounter3 = 0.0,rc_cccounter4 = 0.0, rc_elastic=0.0, rc_nitro=0.0, rc_mvsketch=0.0;
    double pr_cm = 0.0, pr_cu = 0.0,  pr_a = 0.0,  pr_pcusketch = 0.0, pr_cccounter = 0.0,pr_cccounter2 = 0.0,pr_cccounter3 = 0.0,pr_cccounter4 = 0.0, pr_elastic=0.0, pr_nitro=0.0, pr_mvsketch=0.0;
    double f1_cm = 0.0, f1_cu = 0.0,  f1_a = 0.0,  f1_pcusketch = 0.0, f1_cccounter = 0.0,f1_cccounter2 = 0.0,f1_cccounter3 = 0.0,f1_cccounter4 = 0.0, f1_elastic=0.0, f1_nitro=0.0, fl_mvsketch=0.0;
    double tp_cm = 0.0, tp_cu = 0.0,  tp_a = 0.0,  tp_pcusketch = 0.0, tp_cccounter = 0.0,tp_cccounter2 = 0.0,tp_cccounter3 = 0.0,tp_cccounter4 = 0.0, tp_elastic=0.0, tp_nitro=0.0, tp_mvsketch=0.0;
    double fp_cm = 0.0, fp_cu = 0.0,  fp_a = 0.0,  fp_pcusketch = 0.0, fp_cccounter = 0.0,fp_cccounter2 = 0.0,fp_cccounter3 = 0.0,fp_cccounter4 = 0.0, fp_elastic=0.0, fp_nitro=0.0, fp_mvsketch=0.0;
    double tn_cm = 0.0, tn_cu = 0.0,  tn_a = 0.0,  tn_pcusketch = 0.0, tn_cccounter = 0.0,tn_cccounter2 = 0.0,tn_cccounter3 = 0.0,tn_cccounter4 = 0.0, tn_elastic=0.0, tn_nitro=0.0, tn_mvsketch=0.0;
    double fn_cm = 0.0, fn_cu = 0.0,  fn_a = 0.0,  fn_pcusketch = 0.0, fn_cccounter = 0.0,fn_cccounter2 = 0.0,fn_cccounter3 = 0.0,fn_cccounter4 = 0.0, fn_elastic=0.0, fn_nitro=0.0, fn_mvsketch=0.0;

    int threshold = package_num * hh;
    int hh_num = 0;

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        strcpy(temp, (it->first).c_str());
        val = it->second;
        
	bool f1_true = 0;
	bool f2_cm = 0, f2_cu = 0,  f2_a = 0,  f2_pcusketch = 0, f2_cccounter = 0,f2_cccounter2 = 0,f2_cccounter3 = 0,f2_cccounter4 = 0, f2_elastic=0, f2_nitro=0, f2_mvsketch=0;
 
	if (val >= threshold) {
		f1_true = 1;
		hh_num++;
	}

        val_cm = cmsketch->Query(temp);
        val_cu = cusketch->Query(temp);    
        val_a = asketch->Query(temp);      
        val_pcusketch = pcusketch->Query(temp);
	val_cccounter = cccounter->Query(temp);
    val_cccounter2 = cccounter2->Query(temp);
    val_cccounter3 = cccounter3->Query(temp);
    val_cccounter4 = cccounter4->Query(temp);
    // printf("test:result of cc2:%d\n",val_cccounter2);
    // if(val_cccounter!=val_cccounter2){
    //     printf("test:cc vs cc2:%f,%f\n",val_cccounter,val_cccounter2);
    // }
	val_elastic = elasticsketch->Query(temp);
	val_nitro = nitrosketch->Query(temp);
	val_mvsketch = mvsketch->Query(temp);

	if (val_cm >= threshold) f2_cm = 1;
	if (val_cu >= threshold) f2_cu = 1;
	if (val_a >= threshold) f2_a = 1;
	if (val_pcusketch >= threshold) f2_pcusketch = 1;
	if (val_cccounter >= threshold) f2_cccounter = 1;
    if (val_cccounter2 >= threshold) f2_cccounter2 = 1;
    if (val_cccounter3 >= threshold) f2_cccounter3 = 1;
    if (val_cccounter4 >= threshold) f2_cccounter4 = 1;
	if (val_elastic >= threshold) f2_elastic = 1;
	if (val_nitro >= threshold) f2_nitro = 1;
	if (val_mvsketch >= threshold) f2_mvsketch = 1;

	if (f1_true) {
        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_cu = fabs(val_cu - val) / (val * 1.0);
        re_a = fabs(val_a - val) / (val * 1.0);
        re_pcusketch = fabs(val_pcusketch - val) / (val * 1.0);
	re_cccounter = fabs(val_cccounter - val) / (val * 1.0);	
    re_cccounter2 = fabs(val_cccounter2 - val) / (val * 1.0);	
    re_cccounter3 = fabs(val_cccounter3 - val) / (val * 1.0);
    re_cccounter4 = fabs(val_cccounter4 - val) / (val * 1.0);	
	re_elastic = fabs(val_elastic - val) / (val * 1.0);
	re_nitro = fabs(val_nitro - val) / (val * 1.0);
	re_mvsketch = fabs(val_mvsketch - val) / (val * 1.0);

        ae_cm = fabs(val_cm - val);
        ae_cu = fabs(val_cu - val);       
        ae_a = fabs(val_a - val);      
        ae_pcusketch = fabs(val_pcusketch - val);       
	ae_cccounter = fabs(val_cccounter - val);
    ae_cccounter2 = fabs(val_cccounter2 - val);
    ae_cccounter3 = fabs(val_cccounter3 - val);
    ae_cccounter4 = fabs(val_cccounter4 - val);
    // if(ae_cccounter2<ae_cccounter){
    //     printf("test:cc vs cc2:%f,%f,%f\n",val_cccounter,val_cccounter2,val);
    // }
	ae_elastic = fabs(val_elastic - val);
	ae_nitro = fabs(val_nitro - val);
	ae_mvsketch = fabs(val_mvsketch - val);


        re_cm_sum += re_cm;
        re_cu_sum += re_cu;        
        re_a_sum += re_a;      
        re_pcusketch_sum += re_pcusketch;            
	re_cccounter_sum += re_cccounter;
    re_cccounter2_sum += re_cccounter2;
    re_cccounter3_sum += re_cccounter3;
    re_cccounter4_sum += re_cccounter4;
	re_elastic_sum += re_elastic;
	re_nitro_sum += re_nitro;
	re_mvsketch_sum += re_mvsketch;

        ae_cm_sum += ae_cm;
        ae_cu_sum += ae_cu;      
        ae_a_sum += ae_a;       
        ae_pcusketch_sum += ae_pcusketch;      
	ae_cccounter_sum += ae_cccounter;
    ae_cccounter2_sum += ae_cccounter2;
    ae_cccounter3_sum += ae_cccounter3;
    ae_cccounter4_sum += ae_cccounter4;
	ae_elastic_sum += ae_elastic;
	ae_nitro_sum += ae_nitro;
	ae_mvsketch_sum += ae_mvsketch;
	}

	if (f1_true && f2_cm) tp_cm++;
	else if (f1_true && !f2_cm) fn_cm++;
	else if (!f1_true && f2_cm) fp_cm++;
	else tn_cm++;

	if (f1_true && f2_cu) tp_cu++;
	else if (f1_true && !f2_cu) fn_cu++;
	else if (!f1_true && f2_cu) fp_cu++;
	else tn_cu++;

	if (f1_true && f2_a) tp_a++;
	else if (f1_true && !f2_a) fn_a++;
	else if (!f1_true && f2_a) fp_a++;
	else tn_a++;

	if (f1_true && f2_pcusketch) tp_pcusketch++;
	else if (f1_true && !f2_pcusketch) fn_pcusketch++;
	else if (!f1_true && f2_pcusketch) fp_pcusketch++;
	else tn_pcusketch++;

	if (f1_true && f2_cccounter) tp_cccounter++;
	else if (f1_true && !f2_cccounter) fn_cccounter++;
	else if (!f1_true && f2_cccounter) fp_cccounter++;
	else tn_cccounter++;

    if (f1_true && f2_cccounter2) tp_cccounter2++;
	else if (f1_true && !f2_cccounter2) fn_cccounter2++;
	else if (!f1_true && f2_cccounter2) fp_cccounter2++;
	else tn_cccounter2++;

    if (f1_true && f2_cccounter3) tp_cccounter3++;
	else if (f1_true && !f2_cccounter3) fn_cccounter3++;
	else if (!f1_true && f2_cccounter3) fp_cccounter3++;
	else tn_cccounter3++;

    if (f1_true && f2_cccounter4) tp_cccounter4++;
	else if (f1_true && !f2_cccounter4) fn_cccounter4++;
	else if (!f1_true && f2_cccounter4) fp_cccounter4++;
	else tn_cccounter4++;

	if (f1_true && f2_elastic) tp_elastic++;
	else if (f1_true && !f2_elastic) fn_elastic++;
	else if (!f1_true && f2_elastic) fp_elastic++;
	else tn_elastic++;

	if (f1_true && f2_nitro) tp_nitro++;
	else if (f1_true && !f2_nitro) fn_nitro++;
	else if (!f1_true && f2_nitro) fp_nitro++;
	else tn_nitro++;
 
	if (f1_true && f2_mvsketch) tp_mvsketch++;
	else if (f1_true && !f2_mvsketch) fn_mvsketch++;
	else if (!f1_true && f2_mvsketch) fp_mvsketch++;
	else tn_nitro++;
   }

    	double b = hh_num * 1.0;
	printf("Heavy Hitter threshold = %d\n",threshold);
	printf("Heavy Hitter numbers = %d\n", hh_num);
//cout<<b<<endl;
    	printf("\n*************** Heavy hitter detection: ****************\n\n");
 	printf("*************** AAE ****************\n");

    	printf("aae_cm = %lf\n", ae_cm_sum / b);
	printf("aae_a = %lf\n", ae_a_sum / b);
    	printf("aae_cu = %lf\n", ae_cu_sum / b);
    	printf("aae_pcu = %lf\n", ae_pcusketch_sum / b); 
	printf("aae_CC = %lf\n", ae_cccounter_sum / b);
    printf("aae_CC2 = %lf\n", ae_cccounter2_sum / b);
    printf("aae_CC3 = %lf\n", ae_cccounter3_sum / b);
    printf("aae_CC4 = %lf\n", ae_cccounter4_sum / b);
	printf("aae_elastic = %lf\n", ae_elastic_sum / b);
	printf("aae_nitro = %lf\n", ae_nitro_sum / b);
	printf("aae_mv = %lf\n", ae_mvsketch_sum / b);

    	printf("******************* ARE ******************\n");

    	printf("are_cm = %lf\n", re_cm_sum / b);
	printf("are_a = %lf\n", re_a_sum / b);
    	printf("are_cu = %lf\n", re_cu_sum / b);
    	printf("are_pcu = %lf\n", re_pcusketch_sum / b);
    	printf("are_CC = %lf\n", re_cccounter_sum / b);
        printf("are_CC2 = %lf\n", re_cccounter2_sum / b);
        printf("are_CC3 = %lf\n", re_cccounter3_sum / b);
        printf("are_CC4 = %lf\n", re_cccounter4_sum / b);
	printf("are_elastic = %lf\n", re_elastic_sum / b); 
	printf("are_nitro = %lf\n", re_nitro_sum / b); 
	printf("are_mv = %lf\n", re_mvsketch_sum / b);

	printf("****************** Recall *******************\n");

    	printf("recall_cm = %lf\n", tp_cm / (tp_cm + fn_cm));
	printf("recall_a = %lf\n", tp_a / (tp_a + fn_a));
    	printf("recall_cu = %lf\n", tp_cu / (tp_cu + fn_cu));
    	printf("recall_pcu = %lf\n", tp_pcusketch / (tp_pcusketch + fn_pcusketch));
    	printf("recall_CC = %lf\n", tp_cccounter / (tp_cccounter + fn_cccounter));
        printf("recall_CC2 = %lf\n", tp_cccounter2 / (tp_cccounter2 + fn_cccounter2));
        printf("recall_CC3 = %lf\n", tp_cccounter3 / (tp_cccounter3 + fn_cccounter3));
        printf("recall_CC4 = %lf\n", tp_cccounter4 / (tp_cccounter4 + fn_cccounter4));
	printf("recall_elastic = %lf\n", tp_elastic / (tp_elastic + fn_elastic)); 
	printf("recall_nitro = %lf\n", tp_nitro / (tp_nitro + fn_nitro)); 
	printf("recall_mv = %lf\n", tp_mvsketch / (tp_mvsketch + fn_mvsketch));

	printf("******************* Precision ******************\n");

    	printf("precision_cm = %lf\n", tp_cm / (tp_cm + fp_cm));
	printf("precision_a = %lf\n", tp_a / (tp_a + fp_a));
    	printf("precision_cu = %lf\n", tp_cu / (tp_cu + fp_cu));
    	printf("precision_pcu = %lf\n", tp_pcusketch / (tp_pcusketch + fp_pcusketch));
    	printf("precision_CC = %lf\n", tp_cccounter / (tp_cccounter + fp_cccounter));
        printf("precision_CC2 = %lf\n", tp_cccounter2 / (tp_cccounter2 + fp_cccounter2));
        printf("precision_CC3 = %lf\n", tp_cccounter3 / (tp_cccounter3 + fp_cccounter3));
        printf("precision_CC4 = %lf\n", tp_cccounter4 / (tp_cccounter4 + fp_cccounter4));
	printf("precision_elastic = %lf\n", tp_elastic / (tp_elastic + fp_elastic)); 
	printf("precision_nitro = %lf\n", tp_nitro / (tp_nitro + fp_nitro));     	
	printf("precision_mv = %lf\n", tp_mvsketch / (tp_mvsketch + fp_mvsketch)); 

	printf("****************** F1 score *******************\n");

	printf("f1score_cm = %lf\n", 2 * tp_cm / (2 * tp_cm + fp_cm + fn_cm));
	printf("f1score_a = %lf\n", 2 * tp_a / (2 * tp_a + fp_a + fn_a));
    	printf("f1score_cu = %lf\n", 2 * tp_cu / (2 * tp_cu + fp_cu + fn_cu));
    	printf("f1score_pcu = %lf\n", 2 * tp_pcusketch / (2 * tp_pcusketch + fp_pcusketch + fn_pcusketch));
    	printf("f1score_CC = %lf\n", 2 * tp_cccounter / (2 * tp_cccounter + fp_cccounter + fn_cccounter));
        printf("f1score_CC2 = %lf\n", 2 * tp_cccounter2 / (2 * tp_cccounter2 + fp_cccounter2 + fn_cccounter2));
        printf("f1score_CC3 = %lf\n", 2 * tp_cccounter3 / (2 * tp_cccounter3 + fp_cccounter3 + fn_cccounter3));
        printf("f1score_CC4 = %lf\n", 2 * tp_cccounter4 / (2 * tp_cccounter4 + fp_cccounter4 + fn_cccounter4));
	printf("f1score_elastic = %lf\n", 2 * tp_elastic / (2 * tp_elastic + fp_elastic + fn_elastic)); 
	printf("f1score_nitro = %lf\n", 2 * tp_nitro / (2 * tp_nitro + fp_nitro + fn_nitro)); 
	printf("f1score_mv = %lf\n", 2 * tp_mvsketch / (2 * tp_mvsketch + fp_mvsketch + fn_mvsketch));

    	printf("******************************************************************************\n");
    	printf("Evaluation Ends!\n\n");

    return 0;
}
