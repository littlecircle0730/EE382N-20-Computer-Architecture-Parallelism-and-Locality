#include <stdlib.h>
#include <pthread.h>
#include <math.h>

// define the matrix dimensions A is MxP, B is PxN, and C is MxN
#define M 512
#define N 512
#define P 512
#define MAXTHREADS 256

// global variables
int nthreads;
int row_size;
int col_size;
long block_size;
float** A;
float** B;
float** C;

void* subroutine(void *vargp) {
  int myId = *(int*)vargp;
  int row_block_idx = myId / col_size;
  int col_block_idx = myId % col_size;

  int start_row_idx = row_block_idx*block_size;
  int start_col_idx = col_block_idx*block_size;
  float sum;
  int   i;
  int   j;
  int   k;
  for(i = start_row_idx; i < (start_row_idx + block_size); i++) {
    for(j = start_col_idx; j < (start_col_idx + block_size); j++) {
      sum = 0.0f;
      for (k=0; k<P; k++) {
        // dot product of row from A and column from B
        sum += A[i][k]*B[k][j];
      }
      C[i][j] = sum;
    }
  }
}

void matmul(float **A, float **B, float **C) {

  pthread_t tid[MAXTHREADS];
  int myId[MAXTHREADS];
  row_size = sqrt(nthreads);
  col_size = row_size;
  block_size = M/row_size;

  for(int i = 0; i < nthreads; i++) {
    myId[i] = i;
    pthread_create(&tid[i], NULL, subroutine, &myId[i]); 
  }

  for (int i = 0; i < nthreads; i++) {
    pthread_join(tid[i], NULL); 
  }
}

void create_matrix(float*** A, int m, int n) {
  float **T = 0;
  int i;

  T = (float**)malloc( m*sizeof(float*));
  for ( i=0; i<m; i++ ) {
     T[i] = (float*)malloc(n*sizeof(float));
  }
  *A = T;
}

int main(int argc, char **argv) {

  create_matrix(&A, M, P);
  create_matrix(&B, P, N);
  create_matrix(&C, M, N);

  nthreads = atoi(argv[1]); 
  matmul(A, B, C);

  return (0);
}