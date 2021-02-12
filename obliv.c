#include <stdlib.h>
#include <stdio.h>

// define the matrix dimensions A is MxP, B is PxN, and C is MxN
#define M 4096
#define N 4096
#define P 4096
#define b 32

// calculate C = AxB
void matmul(float **A, float **B, float **C,
	int row_A, int col_A, int row_B, int col_B, 
	int row_C, int col_C, int m, int n, int p) {

	if (m <= b) {
		float sum;
		int   i;
		int   j;
		int   k;

		for (i=0; i<m; i++) {
			// for each row of C
			for (j=0; j<n; j++) {
				// for each column of C
				sum = 0.0f; // temporary value
				for (k=0; k<p; k++) {
					// dot product of row from A and column from B
					sum += A[row_A+i][col_A+k]*B[row_B+k][col_B+j];
				}
				C[row_C+i][col_C+j] = sum;
			}
		}
	}
	else {
		int   m2 = m/2;
		int   n2 = n/2;
		int   p2 = p/2;
		
		matmul(A, B, C, row_A, col_A, row_B, col_B, row_C, col_C, m2, n2, p2);
		matmul(A, B, C, row_A, col_A+p2, row_B+p2, col_B, row_C, col_C, m2, n2, p2);

		matmul(A, B, C, row_A, col_A, row_B, col_B+n2, row_C, col_C+n2, m2, n2, p2);
		matmul(A, B, C, row_A, col_A+p2, row_B+p2, col_B+n2, row_C, col_C+n2, m2, n2, p2);

		matmul(A, B, C, row_A+m2, col_A, row_B, col_B, row_C+m2, col_C, m2, n2, p2);
		matmul(A, B, C, row_A+m2, col_A+p2, row_B+p2, col_B, row_C+m2, col_C, m2, n2, p2);

		matmul(A, B, C, row_A+m2, col_A, row_B, col_B+n2, row_C+m2, col_C+n2, m2, n2, p2);
		matmul(A, B, C, row_A+m2, col_A+p2, row_B+p2, col_B+n2, row_C+m2, col_C+n2, m2, n2, p2);

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

	matmul(A, B, C, 0, 0, 0, 0, 0, 0, M, N, P);

	return (0);
}
