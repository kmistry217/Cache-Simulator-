#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

struct Set{
	struct Line** line; 
};

struct Line{
	unsigned long int tag; 
	int valid; 
};

unsigned int mask (unsigned int x, unsigned int y){
	unsigned int mask = 0; 
	unsigned int i;
	for (i=x; i<=y; i++){
		mask |= 1<<i;
	}
	return mask; 
}

int main (int argc, char** argv){
	
	int i,j,k;
	int reads=0, writes=0, hit=0, miss=0; 
	int pReads=0, pWrites=0, pHit=0, pMiss=0; 
	unsigned long int addr, indexBits, tagBits, t1, pAddr, pIndexBits, pTagBits, t2;
	int	numSet, assoc, index, blockOffset; 
	char mode; 
	
	if (argc != 6){
		printf("Error: incorrect input");
		return 0; 
	}
	
	int cacheSize = atoi(argv[1]); 
	char* associativity = argv[2]; 
	int blockSize = atoi(argv[4]); 
	
	int tempCacheSize = cacheSize;
	int tempBlockSize = blockSize;
	
	while (tempCacheSize!=1){
		if(tempCacheSize%2 != 0){
			printf("Error: incorrect input in cache size");
			return 0;
		}
		else{
			tempCacheSize /= 2;
		}
	}
	while (tempBlockSize!=1){
		if(tempBlockSize%2 != 0){
			printf("Error: incorrect input in block size");
			return 0;
		}
		else{
			tempBlockSize /= 2;
		}
	}
	 
	char* file = argv[5];
	FILE* fp = NULL;
	fp = fopen(file, "r");
	
	if (strcmp(associativity, "direct") == 0){
		assoc = 1;
		numSet = cacheSize/blockSize;
	}
	else if (strcmp(associativity, "assoc") == 0){
		assoc = cacheSize/blockSize;
		numSet = 1;
		indexBits = 0; 
		pIndexBits = 0;
	}
	else {
		int len = strlen(associativity);
		int num = len-6;
		char* n = &associativity[len-num];
		assoc = atoi(n);
		numSet = cacheSize/(blockSize*assoc); 
	}
	
	index = log2(numSet);
	blockOffset = log2(blockSize);
	
	struct Set** cache = (struct Set**) malloc (numSet * sizeof(struct Set*));
	for (i = 0; i<numSet; i++){
		cache[i] = (struct Set*) malloc (sizeof(struct Set));
		cache[i]->line = malloc(assoc*sizeof(struct Line));
		for(j = 0; j < assoc; j++){
			cache[i]->line[j] = malloc(sizeof(struct Line));
		}
	}
	struct Set** cache2 = (struct Set**) malloc (numSet * sizeof(struct Set*));
	for (i = 0; i<numSet; i++){
		cache2[i] = (struct Set*) malloc (sizeof(struct Set));
		cache2[i]->line = malloc(assoc*sizeof(struct Line));
		for(j = 0; j < assoc; j++){
			cache2[i]->line[j] = malloc(sizeof(struct Line));
			cache2[i]->line[j]->valid = 0;
			cache2[i]->line[j]->tag = 0;
		}
	}
	
	while (fscanf(fp, "%*x: %c %lx\n", &mode, &addr) == 2){
		tagBits = addr>>(blockOffset+index);
		if (strcmp(associativity, "assoc") != 0){
			t1 = addr>>blockOffset;
			indexBits = mask(0,index-1) & t1;  
		}
		
		
		pAddr = addr + blockSize;
		pTagBits = pAddr>>(blockOffset+index);
		if (strcmp(associativity, "assoc") != 0){
			t2 = pAddr>>blockOffset;
			pIndexBits = mask(0,index-1) & t2;  
		}
		
		
			for (i=0; i<assoc; i++){
				if (cache[indexBits]->line[i]->valid == 1){
					if (cache[indexBits]->line[i]->tag == tagBits){
						hit++;
						if (mode == 'W'){
							writes++;
						}
			
			break; 
					}
					else{
						if (i == assoc-1){
							miss++;
							reads++;
							if (mode == 'W'){
								writes++;
							}
							for (j = assoc-1; j>0; j--){
								cache[indexBits]->line[j]->tag = cache[indexBits]->line[j-1]->tag;
								cache[indexBits]->line[j]->valid = cache[indexBits]->line[j-1]->valid;				
							}
							cache[indexBits]->line[0]->tag = tagBits;
							cache[indexBits]->line[0]->valid = 1;
						}
						else{
							continue;
						}
					}
				}
				else{
					if (i == assoc-1){
						miss++;
						reads++;
						if (mode == 'W'){
							writes++;
						}
						for (j = assoc-1; j>0; j--){
							cache[indexBits]->line[j]->tag = cache[indexBits]->line[j-1]->tag;
							cache[indexBits]->line[j]->valid = cache[indexBits]->line[j-1]->valid;				
						}
						cache[indexBits]->line[0]->tag = tagBits;
						cache[indexBits]->line[0]->valid = 1;
					}
					else{
						continue;
					}
				}
			}
				
		
			
			for (i=0; i<assoc; i++){
				if(cache2[indexBits]->line[i]->valid == 1){
					if (cache2[indexBits]->line[i]->tag == tagBits){
						pHit++;
						if (mode == 'W'){
							pWrites++;
						}
						break; 
					}
				}
				if (i == assoc-1){
						pMiss++;
						pReads++;
						if (mode == 'W'){
							pWrites++;
						}
						for (j = assoc-1; j>0; j--){
							cache2[indexBits]->line[j]->tag = cache2[indexBits]->line[j-1]->tag;
							cache2[indexBits]->line[j]->valid = cache2[indexBits]->line[j-1]->valid;				
						}
						cache2[indexBits]->line[0]->tag = tagBits;
						cache2[indexBits]->line[0]->valid = 1;
				
						for(j=0; j<assoc; j++){
							if (cache2[pIndexBits]->line[j]->tag == pTagBits){
								break;
							}
							else{
								if(j==assoc-1){
									for (k = assoc-1; k>0; k--){
										cache2[pIndexBits]->line[k]->tag = cache2[pIndexBits]->line[k-1]->tag;
										cache2[pIndexBits]->line[k]->valid = cache2[pIndexBits]->line[k-1]->valid;				
									}
									cache2[pIndexBits]->line[0]->tag = pTagBits;
									cache2[pIndexBits]->line[0]->valid = 1;
									pReads++;
								}
							}
						}
				}
			}
				
		}
				
	
	printf("no-prefetch\n");
	printf("Memory reads: %d\n", reads);
	printf("Memory writes: %d\n", writes);
	printf("Cache hits: %d\n", hit);
	printf("Cache misses: %d\n", miss);
	printf("with-prefetch\n");
	printf("Memory reads: %d\n", pReads);
	printf("Memory writes: %d\n", pWrites);
	printf("Cache hits: %d\n", pHit);
	printf("Cache misses: %d\n", pMiss);
	
	fclose(fp);
	return 0; 
}








