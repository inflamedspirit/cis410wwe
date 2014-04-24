/*
 * Matrix Multiply.
 *
 * This is a simple matrix multiply program which will compute the product
 *
 *                C  = A * B
 *
 * A ,B and C are both square matrix. They are statically allocated and
 * initialized with constant number, so we can focuse on the parallelism.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

//Flag for enabeling reorganization optimization
#define REORGANIZE

//Flag for enableing parallelization optimization
//requires REORGANIZE to be activated
#define PARALLEL

#define ORDER 1000   // the order of the matrix
#define AVAL  3.0    // initial value of A
#define BVAL  5.0    // initial value of B
#define TOL   0.001  // tolerance used to check the result

#define N ORDER
#define P ORDER
#define M ORDER

//BLOCK_SIZE is chosen for maximum cache performance. 16 Does best.
#define BLOCK_SIZE 16

//SIZE are the number of blocks. Rounded up to store entire array
#define SIZE_N (N/BLOCK_SIZE + ((N % BLOCK_SIZE)?1:0))
#define SIZE_P (P/BLOCK_SIZE + ((P % BLOCK_SIZE)?1:0))
#define SIZE_M (M/BLOCK_SIZE + ((M % BLOCK_SIZE)?1:0))

//ELEM are the number of elements... unused?
#define ELEM_N SIZE_N*BLOCK_SIZE
#define ELEM_P SIZE_P*BLOCK_SIZE
#define ELEM_M SIZE_M*BLOCK_SIZE

double A[N][P];
double B[P][M];
double C[N][M];

//Create matrices with same size but better cache concurrency
double SUB_A[SIZE_N][SIZE_P][BLOCK_SIZE][BLOCK_SIZE];
double SUB_B[SIZE_P][SIZE_M][BLOCK_SIZE][BLOCK_SIZE];
double SUB_C[SIZE_N][SIZE_M][BLOCK_SIZE][BLOCK_SIZE];

// Initialize the matrices (uniform values to make an easier check)
void matrix_init(void) {
  int i, j, k, l;

	// A[N][P] -- Matrix A
	for (i=0; i<N; i++) {
		for (j=0; j<P; j++) {
		    A[i][j] = AVAL;
		}
	}

	// B[P][M] -- Matrix B
	for (i=0; i<P; i++) {
		for (j=0; j<M; j++) {
		    B[i][j] = BVAL;
		}
	}

	// C[N][M] -- result matrix for AB
	for (i=0; i<N; i++) {
		for (j=0; j<M; j++) {
			C[i][j] = 0.0;
		}
	}

	// SUB_A[N][P] -- Matrix A
	for (i=0; i<SIZE_N; i++) {
	  for (j=0; j<SIZE_P; j++) {
	    for(k=0; k<BLOCK_SIZE; k++) {
	      for(l=0; l<BLOCK_SIZE; l++) {
	      int ii=i*BLOCK_SIZE+k;
	      int jj=j*BLOCK_SIZE+l;
	      if(ii<N && jj<P){
		SUB_A[i][j][k][l] = A[ii][jj];
	      } else {
		SUB_A[i][j][k][l] = 0.0;
	      }
	    }
	  }
	}
	}

	// SUB_B[N][P] -- Matrix B
	for (i=0; i<SIZE_P; i++) {
	  for (j=0; j<SIZE_M; j++) {
	    for(k=0; k<BLOCK_SIZE; k++) {
	      for(l=0; l<BLOCK_SIZE; l++) {
	      int ii=i*BLOCK_SIZE+k;
	      int jj=j*BLOCK_SIZE+l;
	      if(ii<P && jj<M){
		SUB_B[i][j][k][l] = B[ii][jj];
	      } else {
		SUB_B[i][j][k][l] = 0.0;
	      }
	    }
	  }
	}
	}

	// SUB_C[N][P] -- Matrix C
	for (i=0; i<SIZE_N; i++) {
	  for (j=0; j<SIZE_M; j++) {
	    for(k=0; k<BLOCK_SIZE; k++) {
	    for(l=0; l<BLOCK_SIZE; l++) {
	      int ii=i*BLOCK_SIZE+k;
	      int jj=j*BLOCK_SIZE+l;
	      SUB_C[i][j][k][l] = 0.0;
	      }
	  }
	}
	}
}

void copy_results(void){

  int i, j, k, l;
  // SUB_C[N][P] -- Matrix C -- Copy Result in C_SUB to C
  for (i=0; i<SIZE_N; i++) {
    for (j=0; j<SIZE_M; j++) {
      for(k=0; k<BLOCK_SIZE; k++) {
	for(l=0; l<BLOCK_SIZE; l++) {
	int ii=i*BLOCK_SIZE+k;
	int jj=j*BLOCK_SIZE+l;
	if(ii<P && jj<M){
	  C[ii][jj] = SUB_C[i][j][k][l];
	}
      }
    }
  }
  }

  return;
}


// The actual mulitplication function, totally naive
#ifdef REORGANIZE
double matrix_multiply(void) {
  int oi, oj;
  int i, j, k, l, q, p;
  double start, end;

	// timer for the start of the computation
	// Reorganize the data but do not start multiplying elements before 
	// the timer value is captured.
  start = omp_get_wtime(); 

  /* //Single loop - makes no obvious difference

  #pragma omp parallel for  	
  for(i=0; i<SIZE_N*SIZE_M; i++){

    oi = i / SIZE_M;
    oj = i % SIZE_M;
    //  #pragma omp parallel for  
    //  for(oj=0; oj<SIZE_M; oj++){
  //this is where we launch!

      //iterate over matrices:
      for(k=0; k<SIZE_P; k++){
      //submatrix multiply:
      for(q=0; q<BLOCK_SIZE; q++){
      for(l=0; l<BLOCK_SIZE; l++){
      for(p=0; p<BLOCK_SIZE; p++){
	SUB_C[oi][oj][q][l] += SUB_A[oi][k][q][p] * SUB_B[k][oj][p][l];
      }
      }
      }
      }

  //end launch
  }
*/

  #ifdef PARALLEL
  #pragma omp parallel for     
  #endif
  for(oi=0; oi<SIZE_N; oi++){
  #ifdef PARALLEL
  #pragma omp parallel for  
  #endif
  for(oj=0; oj<SIZE_M; oj++){
  //this is where we launch!

      //iterate over matrices:
      for(k=0; k<SIZE_P; k++){
      //submatrix multiply:
      for(q=0; q<BLOCK_SIZE; q++){
      for(l=0; l<BLOCK_SIZE; l++){
      for(p=0; p<BLOCK_SIZE; p++){
	SUB_C[oi][oj][q][l] += SUB_A[oi][k][q][p] * SUB_B[k][oj][p][l];
      }
      }
      }
      }


  //end launch
  }
  }

  // timer for the end of the computation
  end = omp_get_wtime();

  //copy the results from the streamlined matrix into the old matrix form (for... compatability?)
  copy_results();

  // return the amount of high resolution time spent
  return end - start;

}
#endif


#ifndef REORGANIZE
double matrix_multiply(void) {
	int i, j, k;
	double start, end;

	// timer for the start of the computation
	// Reorganize the data but do not start multiplying elements before 
	// the timer value is captured.
	start = omp_get_wtime(); 


	for (i=0; i<N; i++){
		for (j=0; j<M; j++){
			for(k=0; k<P; k++){
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}

	// timer for the end of the computation
	end = omp_get_wtime();
	// return the amount of high resolution time spent
	return end - start;
}
#endif



// Function to check the result, relies on all values in each initial
// matrix being the same
int check_result(void) {
	int i, j;

	double e  = 0.0;
	double ee = 0.0;
	double v  = AVAL * BVAL * ORDER;

	for (i=0; i<N; i++) {
		for (j=0; j<M; j++) {
			e = C[i][j] - v;
			ee += e * e;
		}
	}

	if (ee > TOL) {
		return 0;
	} else {
		return 1;
	}
}

// main function
int main(int argc, char **argv) {
	int correct;
	double run_time;
	double mflops;

	// initialize the matrices
	matrix_init();
	// multiply and capture the runtime
	run_time = matrix_multiply();
	// verify that the result is sensible
	correct  = check_result();

	// Compute the number of mega flops
	mflops = (2.0 * N * P * M) / (1000000.0 * run_time);
	printf("Order %d multiplication in %f seconds \n", ORDER, run_time);
	printf("Order %d multiplication at %f mflops\n", ORDER, mflops);

	// Display check results
	if (correct) {
		printf("\n Hey, it worked");
	} else {
		printf("\n Errors in multiplication");
	}
	printf("\n all done \n");

	return 0;
}
