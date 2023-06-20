#include <assert.h> // assert
#include <math.h>   // q, ceil
#include <stdio.h>  // printf
#include <stdlib.h> // malloc
#include <string.h>

#include "cc.h"
#include "murmurhash2.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) < (b)) ? (b) : (a))

#define BIT64 64
#define CCS_HASH(item, itemlen, i) MurmurHash2(item, itemlen, i)

CCSketch *NewCCSketch(size_t width, size_t depth) {
    assert(width > 0);
    assert(width < MAX_MEM);
    assert(depth > 0);

    CCSketch *cc = CCS_CALLOC(1, sizeof(CCSketch));

    cc->width = width;
    cc->depth = depth;
    //cms->counter = 0;
    for (size_t i=0; i<depth; i++){
	for (size_t j=0; j<MAX_MEM+10; j++){
	    for (size_t k=0; k<BN; k++){
		cc->HK[i][j][k].FP = cc->HK[i][j][k].C = 0;
	    }
	}
    }
    return cc;
}

void CCS_Destroy(CCSketch *cc) {
    assert(cc);

    for (size_t i=0; i<cc->depth; i++){
	for (size_t j=0; j<MAX_MEM+10; j++){
	    CCS_FREE(cc->HK[i][j]);
	}
    	CCS_FREE(cc->HK[i]);
    }

    CCS_FREE(cc);
}

size_t overflow(node (*n), size_t j){
	if (j==0 || j==1){
		if (n[2].C < 15){
			uint32_t tmp = n[j].FP;
			n[j].FP = n[2].FP;
			n[2].FP = tmp;
			n[j].C = n[2].C;
			n[2].C = 15;			
		}
		else if (n[3].C < 15){
			uint32_t tmp = n[j].FP;
			n[j].FP = n[3].FP;
			n[3].FP = tmp;
			n[j].C = n[3].C;
			n[3].C = 15;	
		}
		else return 0;
	}
	else if (j==2){
		if (n[3].C < 255){
			uint32_t tmp = n[j].FP;
			n[j].FP = n[3].FP;
			n[3].FP = tmp;
			n[j].C = n[3].C;
			n[3].C = 255;	
		}
		else return 0;
	}
	return 1;
}

size_t plus(node (*n), size_t j){
	n[j].C++;
	if ((n[j].C==15 && (j==0||j==1)) || (n[j].C==255 && j==2)){
		size_t flag = overflow(n, j);
		if (!flag) return 0;
	}
	return 1;
}

size_t CCS_IncrBy(CCSketch *cc, const char *item, size_t itemlen, size_t value) {
    assert(cc);
    assert(item);

	uint32_t maxv = 0;
	uint32_t H1 = CCS_HASH(item, strlen(item), 0); 
	uint32_t FP = (H1 >> 24);
	
	//unsigned long long H2 = H1^Hash(std::to_string(FP));//XOR CUCKOO HASHING
	uint32_t H2 = H1^FP;
	uint32_t Hsh1 = H1 % cc->width;
	uint32_t Hsh2 = H2 % cc->width;
	uint32_t count = 0;
		
	uint32_t hash[2] = { Hsh1, Hsh2 };
	uint32_t hashHH[2] = { H1, H2 };
		
	size_t flag=0;
	size_t ii, jj;
	for(size_t i = 0; i < CC_d; i++)
		for (size_t j = 0; j < BN; j++)
		{		
			if (cc->HK[i][hash[i]][j].FP == FP) {
				//cc->HK[i][hash[i]][j].C += value;
				plus(cc->HK[i][hash[i]], j);
				maxv = max(maxv, cc->HK[i][hash[i]][j].C);
				return maxv;
			}
			if(!flag && cc->HK[i][hash[i]][j].FP == 0)
			{
				ii=i; jj=j; flag=1;
			}
		}
	if (flag) {
	    cc->HK[ii][hash[ii]][jj].FP = FP;
	    cc->HK[ii][hash[ii]][jj].C = value;
	    return value;
	}
	//mean can not insert normally
	size_t rad = Hsh1 & 0x1;
	cc->HK[rad][hash[rad]][0].FP = FP;
	cc->HK[rad][hash[rad]][0].C = value;
	maxv=max(maxv, value);
	return maxv;
}

size_t CCS_Query(CCSketch *cc, const char *item, size_t itemlen) {
    assert(cc);
    assert(item);
	
	int maxv = 0;
	uint32_t H1 = CCS_HASH(item, strlen(item), 0); 
	uint32_t FP = (H1 >> 24);
	//unsigned long long H2 = H1^Hash(std::to_string(FP));//XOR CUCKOO HASHING
	uint32_t H2 = H1^FP;
	uint32_t Hsh1 = H1 % cc->width;
	uint32_t Hsh2 = H2 % cc->width;
		
	uint32_t hash[2] = { Hsh1, Hsh2 };
	uint32_t hashHH[2] = { H1, H2 };

	size_t minv = (size_t)-1;
	for(size_t i = 0; i < CC_d; i++)
		for (size_t j = 0; j < BN; j++)
		{		
			if (cc->HK[i][hash[i]][j].FP == FP) {
				return cc->HK[i][hash[i]][j].C;
			}
			minv = min(minv, cc->HK[i][hash[i]][j].C);
		}
	return minv;
}


/************ used for debugging *******************
void CMS_Print(const CMSketch *cms) {
    assert(cms);

    for (int i = 0; i < cms->depth; ++i) {
        for (int j = 0; j < cms->width; ++j) {
            printf("%d\t", cms->array[(i * cms->width) + j]);
        }
        printf("\n");
    }
    printf("\tCounter is %lu\n", cms->counter);
} */
