/* cjunderhill-sccoache
 * 
 */

#include "cachelab.h"

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <math.h>

// Always use a 64-bit variable to hold memory addresses
typedef unsigned long long int mem_addr_t;

// Struct to hold parameters for cache
typedef struct {
	int s; // Number of set index bits (2^s)?
	int b; // Number of block bits (2^b)?
	int E; // Associativity (number of lines per set)
	int S; // Number of sets (S = 2^s)
	int B; // Block size (B = 2^b)

	int hits;	// Number of cache hits
	int misses;	// Number of cache misses
	int evicts;	// Number of cache evictions

} cache_param_struct;

//
typedef struct {
	int last_used;	// LRU counter
	int valid;		// Valid bit
	mem_addr_t tag;	// Tag
	char * block;
} set_line;

//
typedef struct {
	set_line *lines;
} cache_set;

//
typedef struct {
	cache_set *sets;
} cache;

int verbosity;	// 0 if not verbose output

long long bit_pow(int exp) {
	long long result = 1;
	result = result << exp;
	return result;
}

/*
 * printHelp - Print basic usage information
 */
void printHelp(char* argv[])
{
    printf("Usage format: %s [-hv] -s <int> -E <int> -b <int> -t <file>\n", argv[0]);
    printf("Options:\n");
    printf("  -h         Print this usage message.\n");
    printf("  -v         Toggle verbosity (optional).\n");
    printf("  -s <int>   Number of set index bits.\n");
    printf("  -E <int>   Number of lines per set.\n");
    printf("  -b <int>   Number of block offset bits.\n");
    printf("  -t <file>  Trace file.\n");
    printf("\nExample:\n");
    printf("  %s -s 12 -E 3 -b 12 -t traces/dave.trace\n", argv[0]);
    exit(0);
}

//
cache build_cache(long long num_sets, int num_lines, long long block_size) {

	cache newCache;
	cache_set newCacheSet;
	set_line newSetLine;

	int setIndex, lineIndex;

	for (setIndex = 0; setIndex < num_sets; setIndex++) {
		newCacheSet.lines = (set_line*) malloc(sizeof(set_line) * num_lines);
		newCache.sets[setIndex] = set;

		for(lineIndex = 0; lineIndex < num_lines; lineIndex++) {
			line.last_used = 0;
			line.valid = 0;
			line.tag = 0;
			newCacheSet.lines[lineIndex] = line;
		}

	}


	return newCache;
}

//
void clear_cache(cache sim_cache, long long num_sets, int num_lines, long long block_size) {
	int setIndex;

	for(setIndex = 0; setIndex < num_sets; setIndex++) {
		cache_set set = sim_cache.sets[setIndex];

		if(set.lines != NULL) {
			free(set.lines);
		}
	}

	if(sim_cache.sets != NULL) {
		free(sim_cache.sets);
	}
}

//
int find_empty_line(cache_set query_set, cache_param_struct param) {

	int num_lines = param.E;
	set_line line;

	for(int i = 0; i < num_lines; i++) {
		line = query_set.lines[index];
		if(line.valid == 0) {
			return i;
		}
	}

	//
	return -1;
}

//
int find_evict_line(cache_set query_set, cache_param_struct param, int *used_lines) {

	int num_lines = param.E;
	int max_used = query_set.lines[0].last_used;
	int min_used = query_set.lines[0].last_used;
	int min_used_index = 0;

	set_line line;
	int lineIndex;

	for(lineIndex = 1; lineIndex < numLines; lineIndex++) {
		line = query_set.lines[lineIndex];

		if(min_used > line.last_used) {
			min_used_index = lineIndex;
			min_used = line.last_used;
		}

		if(max_used < line.last_used) {
			max_used = line.last_used;
		}
	}

	used_lines[0] = min_used;
	used_lines[1] = max_used;
	return min_used_index;
}

/* */
int main()
{
    printSummary(0, 0, 0);
    return 0;
}
