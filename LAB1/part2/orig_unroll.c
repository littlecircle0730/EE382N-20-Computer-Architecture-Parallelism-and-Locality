#include <stdlib.h>

// define the matrix dimensions A is MxP, B is PxN, and C is MxN
#define M 512
#define N 512
#define P 512

// calculate C = AxB
void matmul(float **A, float **B, float **C) {
	float sum_0, sum_1, sum_2, sum_3;
	int   i;
	int   j;
	int   k;

	for (i=0; i<M; i+=2) {
		// for each row of C
		for (j=0; j<N; j+=2) {
			// for each column of C
			sum_0 = 0.0f; // temporary value
			sum_1 = 0.0f; // temporary value
			sum_2 = 0.0f; // temporary value
			sum_3 = 0.0f; // temporary value
			for (k=0; k<P; k+=2) {
				// dot product of row from A and column from B
				sum_0 += A[i][k]*B[k][j];
				sum_0 += A[i][k+1]*B[k+1][j];
				sum_1 += A[i][k]*B[k][j+1];
				sum_1 += A[i][k+1]*B[k+1][j+1];
				sum_2 += A[i+1][k]*B[k][j];
				sum_2 += A[i+1][k+1]*B[k+1][j];
				sum_3 += A[i+1][k]*B[k][j+1];
				sum_3 += A[i+1][k+1]*B[k+1][j+1];
			}
			C[i][j] = sum_0;
			C[i][j+1] = sum_1;
			C[i+1][j] = sum_2;
			C[i+1][j+1] = sum_3;
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
