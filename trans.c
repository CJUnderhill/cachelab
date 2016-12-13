/*
 * Chad Underhill and Sam Coache
 * cjunderhill-sccoache
 */

/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit_known(int M, int N, int A[N][M], int B[M][N], int b);
void transpose_submit_other(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N]){

	// Determine variation of transpose function to run based on dimensions of input matrix
	switch(N) {
    	case 32:	// 32x32 matrix (set block size to 8)
    		transpose_submit_known(M, N, A, B, 8);
        	break;
    	case 64:	// 64x64 matrix (set block size to 4)
    		transpose_submit_known(M, N, A, B, 4);
      		break;
      	default:	// all matrices not 32x32 or 64x64
        	transpose_submit_other(M, N, A, B);
        	break;
    }
}


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 


char transpose_submit_known_desc[] = "Transpose a 32x32 or 64x64 matrix";
void transpose_submit_known(int M, int N, int A[N][M], int B[M][N], int b){

	int i, j; 		// Indecies for rows and columns in matrix
	int row, col;	// Track current row and column in matrix
	int diag = 0;	// Hold position of diagonal element found in matrix (detailed in below code)
	int d_val = 0;	// Hold value of diagonal element found in matrix (detailed in below code)

	// Iterates through each column and row
	for (col = 0; col < N; col += b) { 
		for (row = 0; row < N; row += b) {

			// For each row and column in the designated block, until end of matrix
			for (i = row; i < row + b; i++) {
				for (j = col; j < col + b; j++) {

					// If row and column do not match, transposition will occur
					if (i != j) {
						B[j][i] = A[i][j];
					// Else, row and column are same and element in matrix is defined as a diagonal
					} else {

						// Assign diagonal element to a d_valorary variable
						// This saves an individual cache miss on each run through the matrix while the columns and rows still match up
						d_val = A[i][j]; 
						diag = i;
					}
				}
				// If row and column are same, element is defined as a diagonal and our d_valorarily saved element is assigned
				if (row == col) {
					B[diag][diag] = d_val;
				}
			}	
		}
	}
}

char transpose_submit_other_desc[] = "Transpose any matrix that isn't 32x32 or 64x64";
void transpose_submit_other(int M, int N, int A[N][M], int B[M][N]){

	int i, j; 		// Indecies for rows and columns in matrix
	int row, col;	// Track current row and column in matrix
	int diag = 0;	// Hold position of diagonal element found in matrix (detailed in below code)
	int d_val = 0;	// Hold value of diagonal element found in matrix (detailed in below code)

	// Iterates through each column and row
	for (col = 0; col < M; col += 16) {
		for (row = 0; row < N; row += 16) {

			// For each row and column after current one, until end of matrix
			for (i = row; (i < row + 16) && (i < N); i++) {
				for (j = col; (j < col + 16) && (j < M); j++) {

					// If row and column do not match, transposition will occur
					if (i != j) {
						B[j][i] = A[i][j];
					// Else, row and column are same and element in matrix is defined as a diagonal
					} else {

						// Assign diagonal element to a d_valorary variable
						// This saves an individual cache miss on each run through the matrix while the columns and rows still match up
						d_val = A[i][j];
						diag = i;
					}
				}
				// If row and column are same, element is defined as a diagonal and our d_valorarily saved element is assigned
				if (row == col) {
					B[diag][diag] = d_val;
				}
			}
	 	}
	}
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}