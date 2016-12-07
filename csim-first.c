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
	char * block;	// Pointer to set block address
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

// !!! Not necessary, only used once
long long bit_pow(int exp) {
	long long result = 1;
	result = result << exp;
	return result;
}

/*
 * printHelp - Print basic usage information
 * Params:
 *	*argv[] - pointer to an array containing the input arguments
 * Returns: void
 */
void printHelp(char* argv[]) {
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

/*
 * init_cache - Initialize the cache
 * Params:
 *	num_sets - Number of sets in the cache
 *	num_lines - Number of lines per set
 *	block_size - CURRENTLY UNUSED
 * Returns: initialized cache
 */
cache init_cache(long long num_sets, int num_lines, long long block_size) {

	// Define initial cache, set, and line
	cache newCache;
	cache_set newSet;
	set_line setLine;

	// For each set specified in the cache
	for (int setIndex = 0; setIndex < num_sets; setIndex++) {

		// Allocate appropriate memory based on number of lines in set
		newSet.lines = (set_line*) malloc(sizeof(set_line) * num_lines);

		// Add set to cache
		newCache.sets[setIndex] = newSet;

		// For each line specified in the set
		for(int lineIndex = 0; lineIndex < num_lines; lineIndex++) {

			// Initialize the line and add to the set
			setLine.last_used = 0;
			setLine.valid = 0;
			setLine.tag = 0;
			newSet.lines[lineIndex] = setLine;
		}

	}


	return newCache;
}

/*
 * clear_cache - Clear the all cache contents
 * Params:
 *	c - cache struct
 *	num_sets - number of sets in the cache
 *	num_lines - number of lines in each set
 *	block_size - CURRENTLY UNUSED
 * Returns: void
 */
void clear_cache(cache c, long long num_sets, int num_lines, long long block_size) {

	/*// For each set in the cache
	for(int i = 0; i < num_sets; i++) {
		cache_set set = c.sets[i];

		if(set.lines != NULL) {
			free(set.lines);
		}
	}*/

	// For each set in the cache
	for(int i = 0; i < num_sets; i++) {

		// Free cache set if it contains data
		if(c.sets[i].lines != NULL) {
			free(c.sets[i].lines);
		}
	}

	// Free all cache sets if they contain data?
	if(c.sets != NULL) {
		free(c.sets);
	}
}

/*
 * find_empty_line - finds next empty line in a set
 * Param:
 * 	cset - given set
 *	param - input parameters given for cache
 * Returns: index of line if empty, -1 if no empty lines were found
 */
int find_empty_line(cache_set cset, cache_param_struct param) {

	// Initialize nuber of lines based on input parameters
	int num_lines = param.E;

	// For each line in the set
	for(int i = 0; i < num_lines; i++) {

		// If line is valid (is empty), return index of line
		if(cset.lines[i].valid == 0) {
			return i;
		}
	}

	// Return -1 if no empty line can be found
	return -1;
}

/*
 * find_evict_line - 
 * Param:
 *	cset - 
 *	param - 
 *	*used_lines - 
 * Returns: 
 */
int find_evict_line(cache_set cset, cache_param_struct param, int *used_lines) {

	// Initialize number of lines based on input parameters
	int num_lines = param.E;
	// Initialize max and min used line to the last used line
	int max_used = cset.lines[0].last_used;
	int min_used = cset.lines[0].last_used;

	int min_used_index = 0;
	set_line line;

	// For each line in the given set
	for(int i = 1; i < num_lines; i++) {

		line = cset.lines[i];

		// If the line is less than the minimum, then set the minimum to the last used line
		if(min_used > line.last_used) {
			min_used_index = i;
			min_used = line.last_used;
		}

		// If the line is greater than the maximum, then set the maximum to the last used line
		if(max_used < line.last_used) {
			max_used = line.last_used;
		}
	}

	used_lines[0] = min_used;
	used_lines[1] = max_used;
	return min_used_index;
}

cache_param_struct run_sim(cache sim_cache, cache_param_struct newcache, mem_addr_t address) {
		
	int lineIndex;
	int cache_full = 1;

	int num_lines = newcache.E;
	int prev_hits = newcache.hits;

	int tag_size = (64 - (newcache.s + newcache.b));
	mem_addr_t input_tag = address >> (newcache.s + newcache.b);
	unsigned long long temp = address << (tag_size);
	unsigned long long setIndex = temp >> (tag_size + newcache.b);
	
  	cache_set query_set = sim_cache.sets[setIndex];

	for (lineIndex = 0; lineIndex < num_lines; lineIndex ++) 	{	
		set_line line = query_set.lines[lineIndex];		
		if (line.valid) {	
			if (line.tag == input_tag) {		
				line.last_used ++;
				newcache.hits ++;
				query_set.lines[lineIndex] = line;
			}
		} else if (!(line.valid) && (cache_full)) {
			//We found an empty line
			cache_full = 0;		
		}
	}	

	if (prev_hits == newcache.hits) {
		//Miss in cache;
		newcache.misses++;
	} else {
		//Data is in cache
		return newcache;
	}

	//We missed, so evict if necessary and write data into cache.
		
	int *used_lines = (int*) malloc(sizeof(int) * 2);
	int min_used_index = find_evict_line(query_set, newcache, used_lines);	

	if (cache_full) 
	{
		newcache.evicts++;

		//Found least-recently-used line, overwrite it.
		query_set.lines[min_used_index].tag = input_tag;
		query_set.lines[min_used_index].last_used = used_lines[1] + 1;
	
	}else{
		int empty_index = find_empty_line(query_set, newcache);

		//Found first empty line, write to it.
		query_set.lines[empty_index].tag = input_tag;
		query_set.lines[empty_index].valid = 1;
		query_set.lines[empty_index].last_used = used_lines[1] + 1;
	}						

	free(used_lines);
	return newcache;
}


/* */
int main(int argc, char **argv)
{
	
	cache example_cache;
	cache_param_struct newcache; //builds a new cache struct
	bzero(&newcache, sizeof(newcache)); //sets the first set of bytes equal to the size of the newcache to 0

	long long num_sets;
	long long block_size;	

	FILE *read_trace;
	char trace_cmd;
	mem_addr_t address;
	int size;
	
	char *trace_file;
	char c;

    while((c=getopt(argc,argv,"s:E:b:t:vh")) != -1) //switch statement for the sEbtvh options on the command line
    {
        switch(c)
		{
        case 's': //Number of set index bits
            newcache.s = atoi(optarg);
            break;
        case 'E': //Associativity
            newcache.E = atoi(optarg);
            break;
        case 'b': //Number of block bits
            newcache.b = atoi(optarg);
            break;
        case 't': //Name of the valgrind trace to replay
            trace_file = optarg;
            break;
        case 'v': //Optional verbose flag that displays trace info
            verbosity = 1;
            break;
        case 'h': //Optional help flag that prints usage info
            printHelp(argv);
            exit(0);
        default: //default exit statement
            printHelp(argv);
            exit(1);
        }
    }

    if (newcache.s == 0 || newcache.E == 0 || newcache.b == 0 || trace_file == NULL) //verifies the commant line arguments are entered correctly 
	{
        printf("%s: You did not enter a valid command line argument\n", argv[0]);
        exit(1);
    }

	
	num_sets = pow(2.0, newcache.s); //exponential function to calculate S
	block_size = bit_pow(newcache.b); //shifts the newcache over to calculate B

	//sets the newcaches variables to 0
	newcache.hits = 0;
	newcache.misses = 0;
	newcache.evicts = 0;
	
	example_cache = init_cache(num_sets, newcache.E, block_size); //builds the cache
 	
	// fill in rest of the simulator routine
	read_trace  = fopen(trace_file, "r");
	
	
	if (read_trace != NULL) {
		while (fscanf(read_trace, " %c %llx,%d", &trace_cmd, &address, &size) == 3) {
			switch(trace_cmd) {
				case 'I':
					break;
				case 'L':
					newcache = run_sim(example_cache, newcache, address);
					break;
				case 'S':
					newcache = run_sim(example_cache, newcache, address);
					break;
				case 'M':
					newcache = run_sim(example_cache, newcache, address);
					newcache = run_sim(example_cache, newcache, address);	
					break;
				default:
					break;
			}
		}
	}
	
    printSummary(newcache.hits, newcache.misses, newcache.evicts); //prints the summary of hits misses and evicts
	clear_cache(example_cache, num_sets, newcache.E, block_size);
	fclose(read_trace);

    return 0;
}