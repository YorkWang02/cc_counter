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
#include <map>

#include "CMSketch.h"
#include "CUSketch.h"
#include "ASketch.h"
#include "PCUSketch.h"
#include "cuckoo_counter.h"
#include "cuckoo_counter2.h"
#include "cuckoo_counter3.h"
//#include "ElasticSketch.h"
#include "NitroSketch.h"
#include "ElasticSketch.h"
#include "MVSketch.h"
using namespace std;


char * filename_stream = "../data0/1.dat";


char insert[30000000][14];
char query[30000000][14];


unordered_map<string, int> unmp;

#define testcycles 3


int main(int argc, char** argv)
{
    double memory = 0.1;	//MB
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

    int memory_ = memory * 1000;//KB
    int word_size = 64;


    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;	//how many counter;
    int w_p = memory * 1024 * 1024 * 8.0 / (word_size * 2);
    int m1 = memory * 1024 * 1024 * 1.0/4 / 4 / 8;
    int m2 = memory * 1024 * 1024 * 3.0/4 / 2 / 1;
    int m2_mv = memory * 1024 * 1024 / 4 / 4;

    printf("\n******************************************************************************\n");
    printf("Evaluation starts!\n\n");

    
    CMSketch *cmsketch;
    CUSketch *cusketch;
    ASketch *asketch;
    PCUSketch *pcusketch;
    CCCounter *cccounter;
    CCCounter2 *cccounter2;
    CCCounter3 *cccounter3;
    Nitrosketch *nitrosketch;
    Elasticsketch *elasticsketch;
    MVsketch *mvsketch;

    char _temp[200], temp2[200];
    int t = 0;

    int package_num = 0;


    FILE *file_stream = fopen(filename_stream, "r");
    //while(fgets(insert[package_num], 105, file_stream)!=NULL) //for real-life dateset
    while( fread(insert[package_num], 1, KEY_LEN, file_stream)==KEY_LEN ) //for the rest
    {
        unmp[string(insert[package_num])]++;
        package_num++;

        if(package_num == MAX_INSERT_PACKAGE)
            break;
    }
    fclose(file_stream);

    uint32_t threshold = 0.0002*package_num;
    printf("test:the value of threshold:%d\n",threshold);




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
    printf("throughput of CM (insert): %.6lf Mips\n", throughput_cm);
    //printf("Pcm = %.6lf\n", cmsketch->zero());


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
    printf("throughput of A (insert): %.6lf Mips\n", throughput_a);


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
    printf("throughput of CU (insert): %.6lf Mips\n", throughput_cu);


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
    printf("throughput of PCU (insert): %.6lf Mips\n", throughput_pcusketch);

 		
	clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter = new CCCounter(memory * 1024 *1024/8);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter = (double)1000.0 * testcycles * package_num / resns;
        printf("throughput of CC (insert): %.6lf Mips\n", throughput_cccounter);
	//printf("Pcc = %.6lf\n", cccounter->zero());

    clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter2 = new CCCounter2(memory * 1024 *1024/8,threshold);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter2->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter2 = (double)1000.0 * testcycles * package_num / resns;
        printf("throughput of CC2 (insert): %.6lf Mips\n", throughput_cccounter2);
    
    clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                cccounter3 = new CCCounter3(memory * 1024 *1024/16);
		for (int i = 0; i < package_num; i++)
                {
                        cccounter3->Insert(insert[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        double throughput_cccounter3 = (double)1000.0 * testcycles * package_num / resns;
        printf("throughput of CC3 (insert): %.6lf Mips\n", throughput_cccounter3);



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
	printf("throughput of Elastic (insert): %.6lf Mips\n", throughput_elastic);


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        nitrosketch = new Nitrosketch(w / LOW_HASH_NUM, LOW_HASH_NUM, 1.0/128);
        for(int i = 0; i < package_num; i++)
        {
            nitrosketch->Insert(insert[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    double throughput_nitro = (double)1000.0 * testcycles * package_num / resns;
    printf("throughput of Nitro (insert): %.6lf Mips\n", throughput_nitro);

    
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
    printf("throughput of MVsketch (insert): %.6lf Mips\n", throughput_mvsketch);

/********************************************************************************************/

    printf("*************************************\n");

    // printf("\n******************************************************************************\n");



/********************************query*********************************/

    double res_tmp=0;
	//double query_temp = 0;
    int flow_num = unmp.size();

    double sum = 0;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = cmsketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cm = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CM (query): %.6lf Mips\n", throughput_cm);
    //printf("%d\n", res_tmp);
    sum += res_tmp;



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = asketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_a = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of A (query): %.6lf Mips\n", throughput_a);
    //printf("%d\n", res_tmp);
    sum += res_tmp;



    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = cusketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_cu = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of CU (query): %.6lf Mips\n", throughput_cu);
    //printf("%d\n", res_tmp);
    sum += res_tmp;
	
	    

    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = pcusketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_pcusketch = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of PCU (query): %.6lf Mips\n", throughput_pcusketch);
    //printf("%d\n", res_tmp);
    sum += res_tmp;   


        clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                for (int i = 0; i < flow_num; i++)
                {
                        res_tmp = cccounter->Query(query[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        throughput_cccounter = (double)1000.0 * testcycles * flow_num / resns;
        printf("throughput of CC (query): %.6lf Mips\n", throughput_cccounter);
        //printf("%d\n", res_tmp);
        sum += res_tmp;

    clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                for (int i = 0; i < flow_num; i++)
                {
                        res_tmp = cccounter2->Query(query[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        throughput_cccounter2 = (double)1000.0 * testcycles * flow_num / resns;
        printf("throughput of CC2 (query): %.6lf Mips\n", throughput_cccounter2);
        //printf("%d\n", res_tmp);
        sum += res_tmp;

        clock_gettime(CLOCK_MONOTONIC, &time1);
        for (int t = 0; t < testcycles; t++)
        {
                for (int i = 0; i < flow_num; i++)
                {
                        res_tmp = cccounter3->Query(query[i]);
                }
        }
        clock_gettime(CLOCK_MONOTONIC, &time2);
        resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
        throughput_cccounter3 = (double)1000.0 * testcycles * flow_num / resns;
        printf("throughput of CC3 (query): %.6lf Mips\n", throughput_cccounter3);
        //printf("%d\n", res_tmp);
        sum += res_tmp;

//			************ELASTIC**************
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < testcycles; t++)
	{
		for (int i = 0; i < flow_num; i++)
		{
			//uint8_t key[4];
			//memcpy(key, query[i], 4);
			res_tmp = elasticsketch->Query(query[i]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	throughput_elastic = (double)1000.0 * testcycles * flow_num / resns;
	printf("throughput of Elastic (query): %.6lf Mips\n", throughput_elastic);
	//printf("%d\n", res_tmp);
	sum += res_tmp;


    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = nitrosketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_nitro = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of Nitro (query): %.6lf Mips\n", throughput_nitro);
    //printf("%d\n", res_tmp);
    sum += res_tmp;

    
    clock_gettime(CLOCK_MONOTONIC, &time1);
    for(int t = 0; t < testcycles; t++)
    {
        for(int i = 0; i < flow_num; i++)
        {
            res_tmp = mvsketch->Query(query[i]);
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &time2);
    resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
    throughput_mvsketch = (double)1000.0 * testcycles * flow_num / resns;
    printf("throughput of MVsketch (query): %.6lf Mips\n", throughput_mvsketch);
    // printf("%d\n", res_tmp);
    sum += res_tmp;


/********************************************************************************************/
    // printf("\n******************************************************************************\n");
    printf("*************************************\n");

    //avoid the over-optimize of the compiler! 
    if(sum == (1 << 30))
        return 0;



    char temp[200];

    double re_cm = 0.0, re_cu = 0.0,  re_a = 0.0,  re_pcusketch = 0.0, re_pcsketch = 0.0, re_cccounter = 0.0,re_cccounter2 = 0.0,re_cccounter3 = 0.0, re_elastic=0.0, re_nitro=0.0, re_mvsketch=0.0;
    double re_cm_sum = 0.0, re_cu_sum = 0.0,  re_a_sum = 0.0,  re_pcusketch_sum = 0.0, re_cccounter_sum = 0.0,re_cccounter2_sum = 0.0,re_cccounter3_sum = 0.0, re_elastic_sum=0.0, re_nitro_sum=0.0, re_mvsketch_sum=0.0;
    
    double ae_cm = 0.0, ae_cu = 0.0,  ae_a = 0.0,  ae_pcusketch = 0.0, ae_cccounter = 0.0,ae_cccounter2 = 0.0,ae_cccounter3 = 0.0, ae_elastic=0.0, ae_nitro=0.0, ae_mvsketch=0.0;
    double ae_cm_sum = 0.0, ae_cu_sum = 0.0,  ae_a_sum = 0.0,  ae_pcusketch_sum = 0.0, ae_cccounter_sum = 0.0,ae_cccounter2_sum = 0.0,ae_cccounter3_sum = 0.0, ae_elastic_sum=0.0, ae_nitro_sum=0.0, ae_mvsketch_sum=0.0;

    double val_cm = 0.0, val_cu = 0.0,  val_a = 0.0,  val_pcusketch = 0.0, val_cccounter = 0.0,val_cccounter2 = 0.0,val_cccounter3 = 0.0, val_elastic=0.0, val_nitro=0.0, val_mvsketch=0.0;
    double erro_cm = 0.0, erro_cu = 0.0, erro_a = 0.0,  erro_pcusketch = 0.0, erro_cccounter = 0.0,erro_cccounter2 = 0.0,erro_cccounter3 = 0.0, erro_elastic=0.0, erro_nitro=0.0, erro_mvsketch=0.0;
    double mem_cc = 0.0, mem_cc_sum = 0.0,mem_cc2 = 0.0, mem_cc2_sum = 0.0,mem_cc3 = 0.0, mem_cc3_sum = 0.0;

    //double mark_cm=0, mark_el=0, mark_cc=0, mark_pcu=0, mark_nitro=0, mark_mv=0;

    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++)
    {
        strcpy(temp, (it->first).c_str());
        val = it->second;
        

        val_cm = cmsketch->Query(temp);
        val_cu = cusketch->Query(temp);    
        val_a = asketch->Query(temp);      
        val_pcusketch = pcusketch->Query(temp);
	val_cccounter = cccounter->Query(temp);
    val_cccounter2 = cccounter2->Query(temp);
    val_cccounter3 = cccounter3->Query(temp);
	val_elastic = elasticsketch->Query(temp);
	val_nitro = nitrosketch->Query(temp);
	val_mvsketch = mvsketch->Query(temp);

	mem_cc = cccounter->Mem(temp);
    mem_cc2 = cccounter2->Mem(temp);
    mem_cc3 = cccounter3->Mem(temp);
	mem_cc_sum += mem_cc;
    mem_cc2_sum += mem_cc2;
    mem_cc3_sum += mem_cc3;

        re_cm = fabs(val_cm - val) / (val * 1.0);
        re_cu = fabs(val_cu - val) / (val * 1.0);
        re_a = fabs(val_a - val) / (val * 1.0);
        re_pcusketch = fabs(val_pcusketch - val) / (val * 1.0);
	re_cccounter = fabs(val_cccounter - val) / (val * 1.0);	
    re_cccounter2 = fabs(val_cccounter2 - val) / (val * 1.0);	
    re_cccounter3 = fabs(val_cccounter3 - val) / (val * 1.0);
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
	ae_elastic = fabs(val_elastic - val);
	ae_nitro = fabs(val_nitro - val);
	ae_mvsketch = fabs(val_mvsketch-val);

        re_cm_sum += re_cm;
        re_cu_sum += re_cu;        
        re_a_sum += re_a;      
        re_pcusketch_sum += re_pcusketch;            
	re_cccounter_sum += re_cccounter;
    re_cccounter2_sum += re_cccounter2;
    re_cccounter3_sum += re_cccounter3;
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
	ae_elastic_sum += ae_elastic;
	ae_nitro_sum += ae_nitro;
	ae_mvsketch_sum += ae_mvsketch;
    }


    	double a = package_num * 1.0;
    	double b = unmp.size() * 1.0;

	printf("mem_cc_averag= %lf\n", mem_cc_sum / b);
	printf("use ratio of bucket= %lf\n", cccounter->Ratio());
    printf("mem_cc2_averag= %lf\n", mem_cc2_sum / b);
	printf("use ratio of bucket= %lf\n", cccounter2->Ratio());
    printf("mem_cc3_averag= %lf\n", mem_cc3_sum / b);
	printf("use ratio of bucket= %lf\n", cccounter3->Ratio());
	printf("*************************************\n");
    	printf("aae_cm = %lf\n", ae_cm_sum / b);
	printf("aae_a = %lf\n", ae_a_sum / b);
    	printf("aae_cu = %lf\n", ae_cu_sum / b);
    	printf("aae_pcu = %lf\n", ae_pcusketch_sum / b); 
	printf("aae_CC = %lf\n", ae_cccounter_sum / b);
    printf("aae_CC2 = %lf\n", ae_cccounter2_sum / b);
    printf("aae_CC3 = %lf\n", ae_cccounter3_sum / b);
	printf("aae_elastic = %lf\n", ae_elastic_sum / b);
	printf("aae_nitro = %lf\n", ae_nitro_sum / b);
	printf("aae_mvsketch = %lf\n", ae_mvsketch_sum / b);
      	
    	printf("*************************************\n");

    	printf("are_cm = %lf\n", re_cm_sum / b);
	printf("are_a = %lf\n", re_a_sum / b);
    	printf("are_cu = %lf\n", re_cu_sum / b);
    	printf("are_pcu = %lf\n", re_pcusketch_sum / b);
    	printf("are_CC = %lf\n", re_cccounter_sum / b);
        printf("are_CC2 = %lf\n", re_cccounter2_sum / b);
        printf("are_CC3 = %lf\n", re_cccounter3_sum / b);
	printf("are_elastic = %lf\n", re_elastic_sum / b); 
	printf("are_nitro = %lf\n", re_nitro_sum / b); 
	printf("are_mvsketch = %lf\n", re_mvsketch_sum / b); 	

    	printf("**************************************\n");
	printf("Evaluation Ends!\n\n");

    return 0;

}