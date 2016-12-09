/*
 * cjunderhill-sccoache
 */

#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#include "cachelab.h"

#define INPUT_CAP 250

// Structure containing Set data
struct set { 
    int *valid;				// Pointer to valid bit
    clock_t *last_accessed;	// Track access info for implementation of LRU
    long *tag;    			// Pointer to tag
};
struct set *g_set;

int hits, misses, evicts;	// Counters to track cache hits, misses, and evictions
int s = 0, b = 0, E = 0;	// Holds parameter input (s = set index, b = block offset, E = # lines/set)

char *trace_file = NULL;		// Hold pointer to the input cache trace file
long access_time = 0;	// Hold access info for LRU implementation

/* 
 * get_set - Get set number from the address
 * Params:
 *	*addr - Pointer to the memory address of the set.
 * Returns: void
 */
int get_set(void *addr) {

	int sbit = (int) ((1 << s) - 1);

    return ((long) addr >> b) & sbit;
}

/* 
 * get_tag - Get tag from the address
 * Params:
 *	*addr - Pointer to the memory address of the tag.
 * Returns: void
 */
long get_tag(void *addr) {

	int sb_bits = (s + b);

    return (long) addr >> sb_bits;
}

/*
 * operate_L - handle a LOAD operation passed in from the cache trace
 * Params:
 *	*addr - Pointer to the memory address being accessed.
 *	size - Number of bytes accessed by the operation.
 * Returns: void
 */
void operate_L(void *addr, int size) {

	// Initialize pointer to current set in cache
    struct set *current_set = &g_set[get_set(addr)];

    int i = 0, is_full = 1;
    int empty_item = 0;         // Track the empty entry
    int last_entry = 0;         // Track the evict entry
    int last_time = current_set->last_accessed[0];  

    // For each line in the set 
    for (; i < E; i++) {   
        // Find and update the access time if entry is valid and has matching tag
        if (current_set->valid[i] == 1 && get_tag(addr) == current_set->tag[i]) {
            current_set->last_accessed[i] = access_time++;
            break;

        // Else if entry is not valid, then it's considered empty and the cache is not full
        } else if (current_set->valid[i] == 0) {
            is_full = 0;
            empty_item = i;

        // Else the entry is valid but the tag is not equal
        } else { 

            // Track LRU item, which will be evicted
            if (current_set->last_accessed[i] < last_time) {
                last_entry = i;
                last_time = current_set->last_accessed[i];
            }
        }
    }

    // If we have a miss
    if (i == E) {
        misses++;

        // If cache is full, evict
        if (is_full) {
            current_set->last_accessed[last_entry] = access_time++;
            current_set->tag[last_entry] = get_tag(addr);
            evicts++;

        // Otherwise it's simply a miss
        } else {
        	// Set line bit to valid, assign an address to the cache, and set the access time
            current_set->last_accessed[empty_item] = access_time++;
            current_set->valid[empty_item] = 1;
            current_set->tag[empty_item] = get_tag(addr);
        }
    // Otherwise it's a hit!
    } else {
        hits++;
    }    

}

/* 
 * operate_S - Handle a STORE operation passed in from the cache trace;
 *				Runs a LOAD operation if miss.
 * Params:
 *	*addr - Pointer to the memory address being accessed.
 *	size - Number of bytes accessed by the operation.
 * Returns: void
 */
void operate_S(void *addr, int size) {
    
    // Initialize pointer to current set in cache
    struct set *current_set = &g_set[get_set(addr)];

    int i = 0;

    // For each line in the set
    for (; i < E; i++) {
    	// Find and update the access time if entry is valid and has matching tag
        if (current_set->valid[i] == 1 && get_tag(addr) == current_set->tag[i]) {
            current_set->last_accessed[i] = access_time++;
            break;
        }
    } 

    // If we have a miss, load the data
    if (i == E) {
        operate_L(addr, size);
    // Otherwise it's a hit!
    } else {
        hits++;
    }
}

/* 
 * operate_M - Handle a MODIFY operation passed in from the cache trace;
 * 				Simply a LOAD operation followed by a STORE operation.
 * Params:
 *	*addr - Pointer to the memory address being accessed.
 *	size - Number of bytes accessed by the operation.
 * Returns: void
 */
void operate_M(void *addr, int size) {

    operate_L(addr, size);
    operate_S(addr, size);
}

/*
 * get_operator - Processes input program parameters.
 *					Utilizes getopt library for core functionality.
 * Params:
 *	argc - Argument count
 *	**argv - Pointer to argument array
 * Returns: void
 */
//function to find the program parameters
void get_operator(int argc, char **argv) {
    int toggle;	// Holds input parameter character for comparison

    // Process while there are still remaining unhandled parameters (where getopt then returns -1)
    while ((toggle = getopt(argc, argv, "s:E:b:t:")) != -1) {

    	// Process input argument
    	if(toggle == 's') {
            s = atoi(optarg);
    	} else if(toggle == 'E') {
			E = atoi(optarg);
    	} else if(toggle == 'b') {
			b = atoi(optarg);
    	} else if(toggle == 't') {
			trace_file = optarg;
    	} else { // Error case
            printf("Error: Illegal operation!\n");
            exit(0);	// Terminate
    	}

    }
}

/*
 * initialize - Initialize cache data structure in memory
 * Returns: void
 */
void initialize() {
    int S = (1 << s);	// Calc number of sets (2^s)

    // Handle nonpositive set counts with error
    if (S <= 0) {
        fprintf(stderr, "Error: Attempted to initialize cache with nonpositive number of sets!\n");
        exit(0);	// Terminate
    }

    // Allocate memory for all sets in cache
    g_set = (struct set*) malloc(sizeof(struct set) * S);
    
    // Allocate memory for data in each set
    for (int i = 0; i < S; i++) {
        g_set[i].last_accessed = (clock_t *) malloc(sizeof(clock_t) * E);
        g_set[i].valid = (int *) malloc(sizeof(int) * E);
        g_set[i].tag = (long *) malloc(sizeof(long) * E);

        // Initialize all blocks on each line to empty
        for(int j = 0 ; j < E; j++) {
            g_set[i].last_accessed[j] = 0;
            g_set[i].valid[j] = 0;
            g_set[i].tag[j] = 0;
        }
    }
}

/*
 * deinitialize - Free cache data structure in memory
 * Returns: void
 */
void deinitialize() {
    int S = (1 << s);	// Calc number of sets (2^s)

    g_set = (struct set*) malloc(sizeof(struct set) * S);

    // Sequentially free memory for each set in the cache
    for (int i = 0; i < S; i++) {
        free(g_set[i].last_accessed);
        free(g_set[i].valid);
        free(g_set[i].tag);
    }

    // Free memory for entire cache
    free(g_set);
}

/*
 * main - Entry point for the program
 * Params:
 *	argc - Argument count
 *	**argv - Pointer to argument array
 * Returns: 0 if success
 */
int main(int argc, char **argv) {

	// Process input parameters
    get_operator(argc, argv);

    // Initialize cache data structure
    initialize();

    char operation[INPUT_CAP];	// Cache operation
    void *addr;					// Operation memory address
    int size;					// Size (in bytes) accessed by operation
    char buf[INPUT_CAP];		// Hold line currently read from the file
    FILE *fp = fopen(trace_file, "r");	// Hold pointer to the specified trace file    

    // Throw error if trace file is invalid
    if (fp == NULL) {
        fprintf(stderr, "Error 404: trace file not found!\n");
        exit(0);	// Terminate
    }

    // For each line in the cache file
    while (fgets(buf, INPUT_CAP, fp) != NULL) {

    	// Parse the line, store operation, address, and size
        sscanf(buf, "%s %p,%d", operation, &addr, &size);

        // Perform relevant operation based on specified operation
        if (*operation == 'S') {
            operate_S(addr, size);
        }
        else if (*operation == 'M') {
            operate_M(addr, size);
        }
        else if (*operation == 'L') {
            operate_L(addr, size);
        }
    }

    // Free cache data structure
    deinitialize();

    // Print summary of cache simulation instructions
    printSummary(hits, misses, evicts);

    return 0;	// Indicate successful run
}