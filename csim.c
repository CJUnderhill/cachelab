/*
 * cjunderhill-sccoache
 */

#include "cachelab.h"
#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#define MAXLINE 250

//initializes the base functions
void get_operator(int argc, char **argv);
void initialize();
void deinitialize();

//initilizes operations functions
void operate_L(void *addr, int size);
void operate_M(void *addr, int size);
void operate_S(void *addr, int size);

//initializes the functions for the caches set and tag
int  get_set(void *addr);
long get_tag(void *addr);

//The struct for one set
struct set { 
    int *valid;
    clock_t *last_accessed;
    long *tag;    
};

int hits, misses, evicts; //variable counter for hits misses and evicts
struct set *g_set; //pointer to the cache
int s = 0, E = 0, b = 0; //
char *file = NULL; //pointer to the file
long accesstime = 0; //the most recent access time

//the main function
int main(int argc, char **argv){
    get_operator(argc, argv); //variables for the operations
    initialize(); //calls the initialize function
   
    FILE *fp = fopen(file, "r"); //get the file

    if (fp == NULL){ //checks to see if the file is valid
        fprintf(stderr, "Error 404: file not found\n");
        exit(0);
    } 

    char op[MAXLINE];
    void *addr;
    int size;
    char buf[MAXLINE];

    while (fgets(buf, MAXLINE, fp) != NULL){ //while loops for the variable operations
        sscanf(buf, "%s %p,%d", op, &addr, &size);
        if (*op == 'S'){ //for the S operator
            operate_S(addr, size);
        }
        else if (*op == 'M'){ //for the M operator
            operate_M(addr, size);
        }
        else if (*op == 'L'){ //for the L operator
            operate_L(addr, size);
        }
    }

    deinitialize();
    printSummary(hits, misses, evicts); //call to the output helper function
    return 0;
}

//function to find the program parameters
void get_operator(int argc, char **argv)
{
    int toggle;

    while ((toggle = getopt(argc, argv, "s:E:b:t:")) != -1)
    {
        switch (toggle)
        {
            case 's': //case for s
                s = atoi(optarg);
                break;
            case 'E': //case for e
                E = atoi(optarg);
                break;
            case 'b': //case for b
                b = atoi(optarg);
                break;
            case 't': //case for t
                file = optarg;
                break;
            default: //error message for an illegal or invalid operation
                printf("illegal operation\n");
                exit(0);
        }
    }
}

//function to initialize a data structure
void initialize()
{
    int S = (1 << s); //

    if ( S <= 0){ //error message for nonpositive S values
        fprintf(stderr, "S is negative or 0\n");
        exit(0);
    }

    g_set = (struct set*)malloc(sizeof(struct set) * S); //mallocs memory for the cache
    
    for (int i = 0; i < S; ++i){ //mallocs the memory for the cache
        g_set[i].valid = (int *)malloc(sizeof(int) * E);
        g_set[i].last_accessed = (clock_t *)malloc(sizeof(clock_t) * E);
        g_set[i].tag = (long *)malloc(sizeof(long) * E);
        for(int j = 0 ; j < E; j++){ //initializes the values in the cache
            g_set[i].valid[j] = 0;
            g_set[i].last_accessed[j] = 0;
            g_set[i].tag[j] = 0;
        }
    }
}

// explicitly return the space back to heap
void deinitialize()
{
    int S = (1 << s); //
    g_set = (struct set*)malloc(sizeof(struct set) * S);

    for (int i = 0; i < S; ++i){ //frees the information from the system memory
        free(g_set[i].valid);
        free(g_set[i].last_accessed);
        free(g_set[i].tag);
    }
    free(g_set); //frees the struct from the system memory
}


//--------------------------------------------------------------------------------------

/*
 * operate_L - handle a LOAD operation passed in from the cache trace
 * Params:
 *	*addr - Pointer to the memory address being accessed.
 *	size - Number of bytes accessed by the operation.
 * Returns: void
 */
void operate_L(void *addr, int size) {

	//
    struct set *current_set = &g_set[get_set(addr)];

    int i = 0, full = 1;
    int empty_item = 0;         // Track the empty item
    int last_item = 0;          // Track the evict item
    int last_time = current_set->last_accessed[0];  

    // For each line in the set 
    for (; i < E; i++) {   
        // Find and update the access time if entry is valid
        if (current_set->valid[i] == 1 && get_tag(addr) == current_set->tag[i]) {
            current_set->last_accessed[i] = ++accesstime;
            break;

        // Else if entry is not valid, then it's considered empty and the cache is not full
        } else if (current_set->valid[i] == 0) {
            full = 0;
            empty_item = i;

        // Else the entry is valid but the tag is not equal
        } else { 

            // Track LRU item, which will be evicted
            if (current_set->last_accessed[i] < last_time) {
                last_item = i;
                last_time = current_set->last_accessed[i];
            }
        }
    }

    // If we have a miss
    if (i == E) {
        misses++;

        // If cache is full, evict
        if (full) {
        	//
            current_set->tag[last_item] = get_tag(addr);
            current_set->last_accessed[last_item] = ++accesstime;
            evicts++;

        // Otherwise it's simply a miss
        } else {
        	// Set line bit to valid, assign an address to the cache, and set the access time
            current_set->valid[empty_item] = 1;
            current_set->tag[empty_item] = get_tag(addr);
            current_set->last_accessed[empty_item] = ++accesstime;
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
    
    //
    struct set *current_set = &g_set[get_set(addr)];

    int i = 0;
    for (; i < E; ++i) {
    	// find
        if (current_set->valid[i] == 1 && get_tag(addr) == current_set->tag[i]) {
            current_set->last_accessed[i] = ++accesstime;
            break;
        }
    } 

    // Store miss
    if (i == E) {
        operate_L(addr, size);   // if miss, then load
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
 * get_set - Get the set number from the address
 * Params:
 *	*addr - Pointer to the memory address of the set.
 * Returns: void
 */
int get_set(void *addr) {
    return (int) (( (long) addr >> b) & ((1 << s) - 1) );
}

/* 
 * get_tag - Get the tag number from the address
 * Params:
 *	*addr - Pointer to the memory address of the tag.
 * Returns: void
 */
long get_tag(void *addr) {
    return (long) ( (long) addr >> (s + b));
}
