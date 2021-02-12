#include <stdlib.h>

// define the matrix dimensions A is MxP, B is PxN, and C is MxN
#define M 4096
#define N 4096
#define P 4096
#define b 32

// calculate C = AxB
void matmul(float **A, float **B, float **C) {
	float sum;
	int   i;
	int   j;
	int   k;
	int   kk;
	int   jj;

	for (kk = 0; k < P; kk += b) {
		for (jj = 0; jj < N; jj += b) {
			for (i = 0; i < M; i++) {
				for (j = jj; j < jj + b; j++) {
					sum = 0.0f;
					for (k = kk; k < kk + b; k++) {
						sum += A[i][k]*B[k][j];
					}
					C[i][j] = sum;
				}
			}
		}
	}
}

// function to allocate a matrix on the heap
// creates an mXn matrix and returns the pointer.
//
// the matrices are in row-major order.
void create_matrix(float*** A, int m, int n) {
	float **T = 0;
	int i;

	T = (float**)malloc( m*sizeof(float*));
	for ( i=0; i<m; i++ ) {
		T[i] = (float*)malloc(n*sizeof(float));
	}
	*A = T;
}

int main() {
	float** A;
	float** B;
	float** C;

	create_matrix(&A, M, P);
	create_matrix(&B, P, N);
	create_matrix(&C, M, N);

	// assume some initialization of A and B
	// think of this as a library where A and B are
	// inputs in row-major format, and C is an output
	// in row-major.

	matmul(A, B, C);

	return (0);
}
